/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "nx_driver_phy.h"
#include "nx_driver_mcx.h"
#include "fsl_debug_console.h"

#include "nx_api.h"

#ifndef NX_DRIVER_DEFERRED_PROCESSING
#error "Please define NX_DRIVER_DEFERRED_PROCESSING in nx_user.h."
#endif

#ifndef NX_ENABLE_INTERFACE_CAPABILITY
#error "Please define NX_ENABLE_INTERFACE_CAPABILITY in nx_user.h."
#endif

#define ENET_REF_CLOCK      (50000000UL)
#define ENET_TXBD_NUM       (16)
#define ENET_RXBD_NUM       (8)

#define ENET_ALIGN(x, align)                                \
        ((unsigned int)((x) + ((align) - 1))                \
         & (unsigned int)(~(unsigned int)((align) - 1)))

#define ENET_RXBUFF_SIZE_ALIGNED                                                \
            ENET_ALIGN(ENET_FRAME_MAX_FRAMELEN + NX_DRIVER_ETHERNET_HEADER_PAD, \
                       ENET_BUFF_ALIGNMENT)

#define NX_DRIVER_ETHERNET_HEADER_REMOVE(p)                         \
{                                                                   \
    p->nx_packet_prepend_ptr += NX_DRIVER_ETHERNET_HEADER_SIZE;     \
    p->nx_packet_length -= NX_DRIVER_ETHERNET_HEADER_SIZE;          \
}

#define NX_DRIVER_CAPABILITY            0U

enum _nx_driver_state {
    NX_DRIVER_STATE_NOT_INITIALIZED     = 0,
    NX_DRIVER_STATE_INITIALIZED         = 1,
    NX_DRIVER_STATE_LINK_ENABLED        = 2,
};

struct _nx_buf_infor {
    void *addr;
    NX_PACKET *pkt;
};

struct eth_if {
    ENET_Type *base;
    enet_handle_t handle;
    enet_tx_reclaim_info_t tx_dirty[ENET_TXBD_NUM];
    uint32_t rx_buff_start_addr[ENET_RXBD_NUM];

    phy_handle_t phy_handle;

    TX_SEMAPHORE tx_sync;
    void (*tx_complete_notify)(struct eth_if *enet);

    NX_IP *ip_ptr;
    NX_INTERFACE *interface_ptr;
    enum _nx_driver_state driver_state;

    struct _nx_buf_infor buf_addr_pkt[ENET_RXBD_NUM * 2];
};

static struct eth_if enet_if[NX_MAX_PHYSICAL_INTERFACES];

static ENET_Type *nx_driver_enet_ptrs[] = ENET_BASE_PTRS;


/* The default MAC address. */
uint8_t _nx_driver_mac_address[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x56};

AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t g_txBuffDescrip[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);

AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t g_rxBuffDescrip[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);

static status_t _nx_driver_hardware_initialize(struct eth_if *enet);
static void* _nx_driver_rxbuf_alloc(ENET_Type *base, void *user_data, uint8_t channel);
static void _nx_driver_rxbuf_free(ENET_Type *base, void *buffer, void *user_data, uint8_t channel);

static struct eth_if *_nx_driver_get_enet_by_phy(uint8_t phyAddr)
{
    int i;

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        if (enet_if[i].phy_handle.phyAddr == phyAddr)
            return &enet_if[i];
    }

    return NULL;
}

static status_t mdio_write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    struct eth_if *enet;

    enet = _nx_driver_get_enet_by_phy(phyAddr);
    if (enet == NULL)
        return kStatus_InvalidArgument;

    return ENET_MDIOWrite(enet->base, phyAddr, regAddr, data);
}

static status_t mdio_read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    struct eth_if *enet;

    enet = _nx_driver_get_enet_by_phy(phyAddr);
    if (enet == NULL)
        return kStatus_InvalidArgument;

    return ENET_MDIORead(enet->base, phyAddr, regAddr, pData);
}

static void _nx_driver_mdio_init(struct eth_if *enet)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(enet->base)]);

    ENET_SetSMI(enet->base, CLOCK_GetCoreSysClkFreq());
}

static VOID _nx_driver_rx_defered_handler(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{
    struct eth_header *pkt = (struct eth_header *)packet_ptr->nx_packet_prepend_ptr;
    uint16_t packet_type = ntohs(pkt->proto);

    /* Point to the payload. */
    packet_ptr->nx_packet_prepend_ptr += NX_DRIVER_ETHERNET_HEADER_SIZE;
    packet_ptr->nx_packet_length      -= NX_DRIVER_ETHERNET_HEADER_SIZE;

    switch(packet_type) {
    case NX_DRIVER_ETHERNET_IP:
    case NX_DRIVER_ETHERNET_IPV6:
        _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
        break;
    case NX_DRIVER_ETHERNET_ARP:
        _nx_arp_packet_deferred_receive(ip_ptr, packet_ptr);
        break;
    case NX_DRIVER_ETHERNET_RARP:
        _nx_rarp_packet_deferred_receive(ip_ptr, packet_ptr);
        break;
    default:
        /* Invalid ethernet header... release the packet.  */
        nx_packet_release(packet_ptr);
        break;
    }
}

static NX_PACKET* _nx_convert_addr_to_packet(struct eth_if *enet, void *addr)
{
    NX_PACKET *pkt_ptr = NULL;
    int i;

    for (i = 0; i < ENET_RXBD_NUM * 2; i++) {
        if (addr == enet->buf_addr_pkt[i].addr) {
            pkt_ptr = enet->buf_addr_pkt[i].pkt;

            /* clear */
            enet->buf_addr_pkt[i].addr = 0;
            enet->buf_addr_pkt[i].pkt = NULL;
            break;
        }
    }
    return pkt_ptr;
}

static VOID _nx_driver_rx_processing(struct eth_if *enet)
{
    enet_buffer_struct_t buf_info;
    enet_rx_frame_struct_t rx_frame;
    NX_PACKET *pkt_ptr;
    status_t status;

    while (1) {
        rx_frame.rxBuffArray = &buf_info;

        status = ENET_GetRxFrame(enet->base, &enet->handle, &rx_frame, 0);
        if (status != kStatus_Success)
            break;

        pkt_ptr = _nx_convert_addr_to_packet(enet, rx_frame.rxBuffArray->buffer);
        if (pkt_ptr == NULL) {
            break;
        }
        pkt_ptr->nx_packet_ip_interface = enet->interface_ptr;
        pkt_ptr->nx_packet_next = NULL;
        pkt_ptr->nx_packet_length = rx_frame.totLen;
        pkt_ptr->nx_packet_append_ptr = pkt_ptr->nx_packet_prepend_ptr + pkt_ptr->nx_packet_length;

        _nx_ip_driver_deferred_receive(enet->ip_ptr, pkt_ptr);
    }
}

static void _nx_driver_callback(ENET_Type *base,
                                enet_handle_t *handle,
                                enet_event_t event,
                                uint8_t channel,
                                enet_tx_reclaim_info_t *tx_dirty,
                                void *user_data)
{
    struct eth_if *enet = (struct eth_if *)user_data;
    NX_PACKET *pkt_ptr;

    switch (event) {
    case kENET_TxIntEvent:
        /* For TX events, just release the packet buffer. */
        pkt_ptr = (NX_PACKET *)tx_dirty->context;
        tx_dirty->context = NULL;
        if (pkt_ptr != NULL) {
            NX_DRIVER_ETHERNET_HEADER_REMOVE(pkt_ptr);

            nx_packet_transmit_release(pkt_ptr);

            if (enet->tx_complete_notify)
                (*enet->tx_complete_notify)(enet);
        }
        break;

    case kENET_RxIntEvent:
        _nx_driver_rx_processing(enet);
        break;

    default:
        break;
    }
}

static status_t _nx_driver_hardware_initialize(struct eth_if *enet)
{
    status_t status;
    enet_config_t config;
    enet_buffer_config_t buff_config = {
        .txRingLen = ENET_TXBD_NUM,
        .txDescStartAddrAlign   = &g_txBuffDescrip[0],
        .txDescTailAddrAlign    = &g_txBuffDescrip[0],
        .txDirtyStartAddr       = &enet->tx_dirty[0],

        .rxRingLen = ENET_RXBD_NUM,
        .rxDescStartAddrAlign   = &g_rxBuffDescrip[0],
        .rxDescTailAddrAlign    = &g_rxBuffDescrip[ENET_RXBD_NUM],
        .rxBufferStartAddr      = &enet->rx_buff_start_addr[0],
        .rxBuffSizeAlign        = ENET_RXBUFF_SIZE_ALIGNED,  // TODO
    };

    ENET_GetDefaultConfig(&config);

    /* Multiple queues are not supported. */
    config.multiqueueCfg = NULL;

    config.interrupt     = kENET_DmaTx | kENET_DmaRx;
    config.rxBuffAlloc   = _nx_driver_rxbuf_alloc;
    config.rxBuffFree    = _nx_driver_rxbuf_free;

    /* Do not support special features. */
    config.specialControl = 0;

    _nx_driver_mdio_init(enet);

    nx_drvier_phy_init(&enet->phy_handle, &config, mdio_read, mdio_write);

    ENET_Init(enet->base, &config, &_nx_driver_mac_address[0], ENET_REF_CLOCK);

    status = ENET_DescriptorInit(enet->base, &config, &buff_config);
    if (status != kStatus_Success) {
        return status;
    }

    ENET_CreateHandler(enet->base, &enet->handle, &config, &buff_config,
                       _nx_driver_callback, enet);

    status = ENET_RxBufferAllocAll(enet->base, &enet->handle);
    if (status != kStatus_Success) {
        return status;
    }

    ENET_StartRxTx(enet->base, 1, 1);

    return kStatus_Success;
}

static void _nx_driver_initialize(NX_IP_DRIVER *driver_req_ptr,
                                  struct eth_if *enet)
{
    NX_INTERFACE *interface_ptr;
    UINT interface_index;
    ULONG msw, lsw;
    status_t status;

    if(enet->driver_state >= NX_DRIVER_STATE_INITIALIZED) {
        driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
        return;
    }
    interface_ptr = driver_req_ptr->nx_ip_driver_interface;
    enet->interface_ptr = interface_ptr;

    interface_index = interface_ptr->nx_interface_index;
    enet->base = nx_driver_enet_ptrs[interface_index];

    enet->ip_ptr = driver_req_ptr->nx_ip_driver_ptr;

    enet->tx_complete_notify = NULL;

    status = _nx_driver_hardware_initialize(enet);
    if (status != kStatus_Success) {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR_INITIALIZE_FAILED;
        return;
    }
    nx_ip_interface_mtu_set(enet->ip_ptr, interface_index, NX_DRIVER_IP_MTU);

    msw = (ULONG)((_nx_driver_mac_address[0] << 8)
                  | (_nx_driver_mac_address[1]));

    lsw = (ULONG)((_nx_driver_mac_address[2] << 24)
                  | (_nx_driver_mac_address[3] << 16)
                  | (_nx_driver_mac_address[4] << 8)
                  | (_nx_driver_mac_address[5]));
    nx_ip_interface_physical_address_set(enet->ip_ptr, interface_index,
                                         msw, lsw, NX_FALSE);

    nx_ip_interface_address_mapping_configure(enet->ip_ptr,
                                              interface_index,
                                              NX_TRUE);

    _nx_ip_driver_deferred_enable(enet->ip_ptr, _nx_driver_rx_defered_handler);

    enet->driver_state = NX_DRIVER_STATE_INITIALIZED;

    PRINTF("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
           _nx_driver_mac_address[0],
           _nx_driver_mac_address[1],
           _nx_driver_mac_address[2],
           _nx_driver_mac_address[3],
           _nx_driver_mac_address[4],
           _nx_driver_mac_address[5]);

    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;

    return;
}

static void _nx_driver_enable(NX_IP_DRIVER *driver_req_ptr, struct eth_if *enet)
{
    enet->driver_state = NX_DRIVER_STATE_LINK_ENABLED;
    driver_req_ptr->nx_ip_driver_interface->nx_interface_link_up = NX_TRUE;
    driver_req_ptr->nx_ip_driver_status =  NX_SUCCESS;
}

static void _nx_driver_disable(NX_IP_DRIVER *driver_req_ptr, struct eth_if *enet)
{
    enet->driver_state = NX_DRIVER_STATE_INITIALIZED;
    driver_req_ptr->nx_ip_driver_interface->nx_interface_link_up = NX_FALSE;
    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
}

static void _nx_driver_tx_notify(struct eth_if *enet)
{
    tx_semaphore_put(&enet->tx_sync);
}

static status_t _nx_driver_hardware_packet_send(struct eth_if *enet, NX_PACKET *packet_ptr)
{
    int count, timeout;
    status_t status;
    NX_PACKET *pkt_ptr;
    enet_buffer_struct_t tx_buff_info;
    enet_tx_frame_struct_t tx_frame = {
        .txBuffArray        = NULL,
        .txBuffNum          = 0U,
        .txConfig.intEnable = 1U,
        .txConfig.tsEnable  = 0U,
        .txConfig.slotNum   = 0U,
        .context            = NULL,
    };

    assert(packet_ptr != NULL);

    pkt_ptr = packet_ptr;
    assert(pkt_ptr->nx_packet_next == NULL);

    tx_buff_info.buffer = (void *)pkt_ptr->nx_packet_prepend_ptr;
    tx_buff_info.length = (uint16_t)pkt_ptr->nx_packet_length;

    tx_frame.txBuffArray = &tx_buff_info;
    tx_frame.txBuffNum   = 1;
    tx_frame.context = (void *)pkt_ptr;

    count = 3;
    do {

        status = ENET_SendFrame(enet->base, &enet->handle, &tx_frame, 0U);
        if (status == kStatus_Success) {

            return kStatus_Success;
        } else if (status == kStatus_ENET_TxFrameBusy) {

            /* The TX buffer is full, wait for it to be freed. */
            tx_semaphore_create(&enet->tx_sync, "enet_tx_sync", 0);
            enet->tx_complete_notify = &_nx_driver_tx_notify;

            timeout = 20;
            tx_semaphore_get(&enet->tx_sync, timeout);

            enet->tx_complete_notify = NULL;
            tx_semaphore_delete(&enet->tx_sync);
        } else {
            break;
        }

    } while (--count);

    return kStatus_Fail;
}

static void _nx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr, struct eth_if *enet)
{
    NX_PACKET *packet_ptr;
    uint32_t *ethernet_frame_ptr;
    uint32_t msw, lsw;
    status_t status;

    packet_ptr = driver_req_ptr->nx_ip_driver_packet;

    if (packet_ptr->nx_packet_length > NX_DRIVER_IP_MTU) {
        /* This packet exceeds the size of the driver's MTU. Simply throw it away! */

        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

        nx_packet_transmit_release(packet_ptr);
        return;
    }
    /* Point to the Ethernet header. */
    packet_ptr->nx_packet_prepend_ptr -= NX_DRIVER_ETHERNET_HEADER_SIZE;
    packet_ptr->nx_packet_length += NX_DRIVER_ETHERNET_HEADER_SIZE;

    /* Move back another 2 bytes to get 32-bit word alignment. */
    ethernet_frame_ptr = (uint32_t *)(packet_ptr->nx_packet_prepend_ptr
                                      - NX_DRIVER_ETHERNET_HEADER_PAD);

    /* make sure it's 32-bit word alignment */
    assert(((uint32_t)ethernet_frame_ptr & 0x03UL) == 0);

    /* Set up the destination MAC address in the Ethernet header. */
    *ethernet_frame_ptr       = driver_req_ptr->nx_ip_driver_physical_address_msw;
    *(ethernet_frame_ptr + 1) = driver_req_ptr->nx_ip_driver_physical_address_lsw;

    /* Set up the source MAC address. */
    msw = driver_req_ptr->nx_ip_driver_interface->nx_interface_physical_address_msw;
    lsw = driver_req_ptr->nx_ip_driver_interface->nx_interface_physical_address_lsw;

    *(ethernet_frame_ptr + 2) = msw << 16 | lsw >> 16;
    *(ethernet_frame_ptr + 3) = lsw << 16;

    /* Set up the frame type field in the Ethernet harder. */
    switch(driver_req_ptr->nx_ip_driver_command) {
    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
        *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_ARP;
        break;
    case NX_LINK_RARP_SEND:
        *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_RARP;
        break;
    case NX_LINK_PACKET_SEND:
    case NX_LINK_PACKET_BROADCAST:
        if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) {
            *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_IP;
        } else if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6) {
            *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_IPV6;
        } else {
            /* Indicate an unsuccessful packet send.  */
            driver_req_ptr->nx_ip_driver_status =  NX_DRIVER_ERROR;

            /* Remove the Ethernet header and free the packet. */
            NX_DRIVER_ETHERNET_HEADER_REMOVE(packet_ptr);
            nx_packet_transmit_release(packet_ptr);
            return;
        }
        break;
    default: /* impossible */
        break;
    }

    /* Change the endianness of the packet to big-endian (network byte order). */
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr));
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 1));
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 2));
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 3));

    /* Transmit the packet through the Ethernet controller low level access routine. */
    status = _nx_driver_hardware_packet_send(enet, packet_ptr);
    if (status != kStatus_Success) {
        /* Driver's hardware send packet routine failed to send the packet.  */

        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

        /* Remove the Ethernet header and free the packet. */
        NX_DRIVER_ETHERNET_HEADER_REMOVE(packet_ptr);
        nx_packet_transmit_release(packet_ptr);
        return;
    }

    driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
    return;
}

static void* _nx_driver_rxbuf_alloc(ENET_Type *base, void *user_data, uint8_t channel)
{
    struct eth_if *enet = (struct eth_if *)user_data;
    NX_PACKET *pkt_ptr;
    void *addr = NULL;
    UINT ret;
    int i;

    ret = nx_packet_allocate(enet->ip_ptr->nx_ip_default_packet_pool, &pkt_ptr,
                             NX_RECEIVE_PACKET, NX_NO_WAIT);
    if (ret != NX_SUCCESS)
        return NULL;

    /* Add 2 bytes padding. */
    pkt_ptr->nx_packet_prepend_ptr += NX_DRIVER_ETHERNET_HEADER_PAD;
    addr = pkt_ptr->nx_packet_prepend_ptr;

    for (i = 0; i < ENET_RXBD_NUM * 2; i++) {
        /* Find an empty entry and save address pointer. */
        if (enet->buf_addr_pkt[i].addr == 0) {
            enet->buf_addr_pkt[i].addr = addr;
            enet->buf_addr_pkt[i].pkt = pkt_ptr;
            break;
        }
    }
    return addr;
}

static void _nx_driver_rxbuf_free(ENET_Type *base, void *buffer, void *user_data, uint8_t channel)
{
    struct eth_if *enet = (struct eth_if *)user_data;
    NX_PACKET *pkt_ptr;

    pkt_ptr = _nx_convert_addr_to_packet(enet, buffer);
    if (pkt_ptr == NULL)
        return;

    nx_packet_release(pkt_ptr);
}

static VOID  _nx_driver_capability_get(NX_IP_DRIVER *driver_req_ptr, struct eth_if *enet)
{
    /* Return the capability of the Ethernet controller.  */
    *(driver_req_ptr->nx_ip_driver_return_ptr) = NX_DRIVER_CAPABILITY;

    /* Return the success status.  */
    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
}

static VOID  _nx_driver_capability_set(NX_IP_DRIVER *driver_req_ptr, struct eth_if *enet)
{
    /* Can not change the capability. */
    driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
}

VOID nx_link_driver(NX_IP_DRIVER *driver_req_ptr)
{
    struct eth_if *enet;
    UINT interface_index;

    assert(driver_req_ptr != NULL);

    interface_index = driver_req_ptr->nx_ip_driver_interface->nx_interface_index;
    enet = &enet_if[interface_index];

    switch (driver_req_ptr->nx_ip_driver_command) {
    case NX_LINK_INTERFACE_ATTACH:
        memset(enet, 0, sizeof(struct eth_if));
        enet->interface_ptr = driver_req_ptr->nx_ip_driver_interface;
        driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
        driver_req_ptr->nx_ip_driver_interface->nx_interface_capability_flag = NX_DRIVER_CAPABILITY;
        return;
    case NX_LINK_INITIALIZE:
        _nx_driver_initialize(driver_req_ptr, enet);
        return;
    default:
        break;
    }

    if (enet->driver_state == NX_DRIVER_STATE_NOT_INITIALIZED) {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR_NOT_INITIALIZED;
        return;
    }

    switch (driver_req_ptr->nx_ip_driver_command) {
    case NX_LINK_ENABLE:
        _nx_driver_enable(driver_req_ptr, enet);
        break;

    case NX_LINK_DISABLE:
        _nx_driver_disable(driver_req_ptr, enet);
        break;

    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_RARP_SEND:
    case NX_LINK_PACKET_SEND:
    case NX_LINK_PACKET_BROADCAST:
        _nx_driver_packet_send(driver_req_ptr, enet);
        break;

    case NX_INTERFACE_CAPABILITY_GET:
        _nx_driver_capability_get(driver_req_ptr, enet);
        break;
    case NX_INTERFACE_CAPABILITY_SET:
        _nx_driver_capability_set(driver_req_ptr, enet);
        break;

    default:
        /* Invalid driver request.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
}
