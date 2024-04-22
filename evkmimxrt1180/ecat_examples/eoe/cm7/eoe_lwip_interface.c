/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This file is the interface file between EoE and lwip. 
 * SSC implements the EoE function by calling the EoE driver.
 */

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/

#include "eoe_lwip_interface.h"

#if STATIC_ETHERNET_BUFFER
uint8_t sendBuf[FRAME_MAX_FRAMELEN];
#endif

void EoE_RecvFrame(uint16_t *pFrame, uint16_t frameSize)
{
    EoE_PutFrameToRxQueue(pFrame, frameSize);
}

err_t EoE_SendFrame(void)
{
#if MAILBOX_QUEUE
    if ((!bEoESendFramePending && (nAlStatus != STATE_INIT)) && (pEoeSendStored == NULL))
#else
    if ((!bEoESendFramePending && (nAlStatus != STATE_INIT)) || (bPendingMbxResponse == FALSE))
#endif /* MAILBOX_QUEUE */
    {
        /** 
        * no Ethernet is sent yet, no datagram is currently stored and the slave is at least in PRE-OP,
        * so we could sent the requested frame
        */  
        uint16_t *buf;
        uint16_t len = 0;
        if ((EoE_GetFrameFromTxQueue(&buf, &len) == ERR_OK)) {
#if STATIC_ETHERNET_BUFFER
            MEMCPY(sendBuf, buf, len);
            /* store the buffer of the Ethernet frame to be sent */
            pEthernetSendFrame = (MEM_ADDR MBXMEM *)sendBuf;
#else
            /* store the buffer of the Ethernet frame to be sent */
            pEthernetSendFrame = (MEM_ADDR MBXMEM *)buf;
#endif /* STATIC_ETHERNET_BUFFER */
            /* Ethernet frame is to be sent */
            bEoESendFramePending = TRUE;
            /* store the size of the Ethernet frame to be sent */
            u16EthernetSendSize = len;
            /* we start with the first fragment */
            u16EthernetSendOffset = 0;
            u8SendFragmentNo = 0;
            SendFragment();               
        } else {
            /* frame could not be sent, try it later */
            return ERR_MEM;
        }
    }
    return ERR_OK;
}

void EoE_SetIp(uint16_t *aMacAdd, uint16_t *aIpAdd, uint16_t *aSubNetMask, uint16_t *aDefaultGateway, uint16_t *aDnsIp) 
{
    struct netif *EoE_netif = EoE_GetNetif();
    uint8_t macaddr[6];
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
    
    MEMCPY(&macaddr, aMacAdd, 6);
    MEMCPY(&ip.addr, aIpAdd, 4);
    MEMCPY(&netmask.addr, aSubNetMask, 4);
    MEMCPY(&gateway.addr, aDefaultGateway, 4);

    /* Set netif down */
    if (netif_is_up(EoE_netif)) {
        netifapi_netif_set_link_down(EoE_netif);
        netifapi_netif_set_down(EoE_netif);
    }

    /* Set netif up */
    memcpy(EoE_netif->hwaddr, macaddr, sizeof(EoE_netif->hwaddr));
    netifapi_netif_set_addr(EoE_netif, &ip, &netmask, &gateway);
    netifapi_netif_set_default(EoE_netif);
    netifapi_netif_set_link_up(EoE_netif);
    netifapi_netif_set_up(EoE_netif);
}
