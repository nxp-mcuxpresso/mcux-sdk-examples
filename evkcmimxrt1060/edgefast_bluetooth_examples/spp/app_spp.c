/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include "fsl_debug_console.h"
#include "bluetooth/sdp.h"
#include "bluetooth/spp.h"
#include "BT_common.h"
#include "app_connect.h"
#include "app_spp.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BT_DEVICE_ADDR_POINTER(ref)\
        (ref)[0],(ref)[1],(ref)[2],(ref)[3],(ref)[4],(ref)[5]

typedef enum spp_appl_state {
    SPP_APPL_STATE_INITIALIZED,
    SPP_APPL_STATE_STARTED,
    SPP_APPL_STATE_IN_CONNECT,
    SPP_APPL_STATE_CONNECTED,
    SPP_APPL_STATE_WAIT_4_CONNECT,
    SPP_APPL_STATE_IN_CONFIG,
}spp_appl_state_t;

struct spp_appl_t
{
    spp_appl_state_t state;
    bt_spp_role_t    role;
    uint8_t          channel;
    struct bt_conn * conn;
    struct bt_spp  * spp_handle;
};

#define SPP_APPL_INIT(index)                                 \
{                                                            \
    spp_appl[index].state      = SPP_APPL_STATE_INITIALIZED; \
    spp_appl[index].role       = BT_SPP_ROLE_SERVER;         \
    spp_appl[index].channel    = 0U;                         \
    spp_appl[index].conn       = NULL;                       \
    spp_appl[index].spp_handle = NULL;                       \
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* get free spp appl */
static uint8_t get_free_spp_appl(void);

/* get spp appl handle with spp handle */
static uint8_t get_spp_appl_with_spp(struct bt_spp *spp);

/* spp application connected callback */
static void spp_connected(struct bt_spp *spp, int error);

/* spp application disconnected callback */
static void spp_disconnected(struct bt_spp *spp, int error);

/* spp application data received callback */
static void spp_data_received(struct bt_spp *spp, uint8_t *data, uint16_t len);

/* spp application data sent callback */
static void spp_data_sent(struct bt_spp *spp, uint8_t *data, uint16_t len);

/* spp control message callback */
static void spp_control(struct bt_spp_control *control, int error);

/* print all active spp appl info */
static void print_spp_appl_info(uint8_t index);

/* print spp port setting */
static void appl_spp_print_port_setting(struct bt_spp_port * port);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t initialized = 0U;

/* spp appl handle list */
static struct spp_appl_t spp_appl[CONFIG_BT_SPP_MAX_CONN];

/* current spp appl handle */
static uint8_t current_spp_handle;

/* Send Str Buffer */
static uint8_t appl_spp_buffer[31];

static bt_spp_callback spp_application_callback =
{
    .connected       = spp_connected,
    .disconnected    = spp_disconnected,
    .data_received   = spp_data_received,
    .data_sent       = spp_data_sent,
    .control         = spp_control,
};

/* spp server channel 3 */
static struct bt_sdp_attribute spp_server_3_attrs[] = {
    BT_SDP_NEW_SERVICE,
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_SERIAL_PORT_SVCLASS) //11 01
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROTO_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 12),//35 0c
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3),// 35 03
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_L2CAP) // 01 00
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 5),// 35 05
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_UUID_RFCOMM_VAL) // 00 03
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
                BT_SDP_ARRAY_16(3) // 03 channel number
            },
            )
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROFILE_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 8), //35 08
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6), //35 06
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_SERIAL_PORT_SVCLASS) //11 01
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0102U) //01 02
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("COM5"),
};

/* spp server channel 5 */
static struct bt_sdp_attribute spp_server_5_attrs[] = {
    BT_SDP_NEW_SERVICE,
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_SERIAL_PORT_SVCLASS) //11 01
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROTO_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 12),//35 0c
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3),// 35 03
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_L2CAP) // 01 00
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 5),// 35 05
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_UUID_RFCOMM_VAL) // 00 03
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
                BT_SDP_ARRAY_16(5) // 05 channel number
            },
            )
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROFILE_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 8), //35 08
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6), //35 06
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_SERIAL_PORT_SVCLASS) //11 01
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0102U) //01 02
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("COM5"),
};

static struct bt_sdp_record spp_server_3_rec = BT_SDP_RECORD(spp_server_3_attrs);
static struct bt_sdp_record spp_server_5_rec = BT_SDP_RECORD(spp_server_5_attrs);
/*******************************************************************************
 * Code
 ******************************************************************************/
static uint8_t get_free_spp_appl(void)
{
    uint8_t index;

    for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index++)
    {
        if(SPP_APPL_STATE_INITIALIZED == spp_appl[index].state)
        {
            spp_appl[index].state = SPP_APPL_STATE_STARTED;
            break;
        }
    }

    return index;
}

static uint8_t get_spp_appl_with_spp(struct bt_spp *spp)
{
    uint8_t          index;
    uint8_t          channel;
    bt_spp_role_t    role;
    struct bt_conn * conn;
    int              err;

    err = bt_spp_get_channel(spp, &channel);
    assert(0 == err);

    err = bt_spp_get_role(spp, &role);
    assert(0 == err);

    err = bt_spp_get_conn(spp, &conn);
    assert(0 == err);
    (void)err;

    for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index++)
    {
        if(SPP_APPL_STATE_INITIALIZED == spp_appl[index].state)
        {
            continue;
        }

        if((SPP_APPL_STATE_WAIT_4_CONNECT == spp_appl[index].state) &&
           (role == spp_appl[index].role) &&
           (channel == spp_appl[index].channel))
        {
            break;
        }
        else if((role == spp_appl[index].role) &&
                (channel == spp_appl[index].channel) &&
                (0 == memcmp(bt_conn_get_dst_br(spp_appl[index].conn)->val, bt_conn_get_dst_br(conn)->val, BT_BD_ADDR_SIZE)))
        {
            break;
        }
        else
        {
            /*invalid spp*/
        }
    }

    return index;
}

static void spp_connected(struct bt_spp *spp, int error)
{
    uint8_t index;
    struct bt_conn * conn;

    index = get_spp_appl_with_spp(spp);

    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        return;
    }

    if(0 == error)
    {
        spp_appl[index].state      = SPP_APPL_STATE_CONNECTED;
        spp_appl[index].spp_handle = spp;

        if(BT_SPP_ROLE_SERVER == spp_appl[index].role)
        {
            bt_spp_get_conn(spp, &conn);
            spp_appl[index].conn = conn;
        }

        print_spp_appl_info(index);
        current_spp_handle = index;
        PRINTF("SPP appl handle %d is connected successfully and becomes current spp appl handle!\n", current_spp_handle);
    }
    else
    {
        if(BT_SPP_ROLE_SERVER == spp_appl[index].role)
        {
            spp_appl[index].conn       = NULL;
            spp_appl[index].spp_handle = NULL;
        }
        else
        {
            SPP_APPL_INIT(index);
        }

        print_spp_appl_info(index);
        PRINTF("SPP appl handle %d is connected failed.\n", index);

        if(index == current_spp_handle)
        {
            PRINTF("Current spp handle isn't valid, please select a valid spp handle.\n", current_spp_handle);
            current_spp_handle = 0xFFU;
        }
    }
}

static void spp_disconnected(struct bt_spp *spp, int error)
{
    uint8_t index;

    index = get_spp_appl_with_spp(spp);

    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        return;
    }

    print_spp_appl_info(index);

    if(0 == error)
    {
        if(BT_SPP_ROLE_SERVER == spp_appl[index].role)
        {
            spp_appl[index].state      = SPP_APPL_STATE_WAIT_4_CONNECT;
            spp_appl[index].conn       = NULL;
            spp_appl[index].spp_handle = NULL;
        }
        else
        {
            SPP_APPL_INIT(index);
        }

        PRINTF("SPP appl handle %d is disconnected successfully.\n", index);

        if(index == current_spp_handle)
        {
            for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index++)
            {
                if(SPP_APPL_STATE_CONNECTED == spp_appl[index].state)
                {
                    break;
                }
            }

            if(CONFIG_BT_SPP_MAX_CONN != index)
            {
                current_spp_handle = index;
                print_spp_appl_info(current_spp_handle);
                PRINTF("Current spp handle is %d.\n", current_spp_handle);
            }
            else
            {
                PRINTF("Current spp handle isn't valid, please select a valid spp handle.\n");
                current_spp_handle = 0xFFU;
            }
        }
    }
    else
    {
        PRINTF("SPP handle %d is disconnected failed, reason = 0x%04X.\n", index, error);
    }
}

static void spp_data_received(struct bt_spp *spp, uint8_t *data, uint16_t len)
{
    uint32_t index;

    index = get_spp_appl_with_spp(spp);

    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        return;
    }

    current_spp_handle = index;
    PRINTF("SPP appl handle %d received %d data callback, dumped here:", current_spp_handle, len);

    PRINTF("\n----------------CHAR DUMP-----------------------\n");
    for (index = 0; index < len; index++)
    {
        PRINTF("%c ", data[index]);
    }
    PRINTF("\n----------------------------------------------------\n");
    PRINTF("\n----------------HEX DUMP------------------------\n");
    for (index = 0; index < len; index++)
    {
        PRINTF("%02X ", data[index]);
    }
    PRINTF("\n----------------------------------------------------\n");
}

static void spp_data_sent(struct bt_spp *spp, uint8_t *data, uint16_t len)
{
    uint32_t index;

    index = get_spp_appl_with_spp(spp);

    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        return;
    }

    PRINTF("\nSPP appl handle %d sent %d data callback, dumped here:", index, len);
    PRINTF("\n----------------CHAR DUMP-----------------------\n");
    for (index = 0; index < len; index++)
    {
        PRINTF("%c ", data[index]);
    }
    PRINTF("\n-----------------------------------------------------\n");
}

static void spp_control(struct bt_spp_control *control, int error)
{
    uint8_t channel;
    bt_spp_role_t role;

    if(NULL == control->spp)
    {
        PRINTF("SPP appl callback on channel %d ", control->channel);
        if(BT_SPP_ROLE_SERVER == control->role)
        {
            PRINTF("server: \n");
        }
        else
        {
            PRINTF("client: \n");
        }
    }
    else
    {
        bt_spp_get_channel(control->spp, &channel);
        bt_spp_get_role(control->spp, &role);

        PRINTF("SPP appl callback on channel %d ", channel);
        if(BT_SPP_ROLE_SERVER == role)
        {
            PRINTF("server: \n");
        }
        else
        {
            PRINTF("client: \n");
        }
    }

    switch(control->type)
    {
    case SPP_REQUEST_PORT_SETTING:
    case SPP_NEGOTIATE_PORT_SETTING:
      if(SPP_NEGOTIATE_PORT_SETTING == control->type)
      {
          PRINTF("Event Type: SPP_NEGOTIATE_PORT_SETTING\n");
      }
      else
      {
          PRINTF("Event Type: SPP_REQUEST_PORT_SETTING\n");
      }
      PRINTF("Status: 0x%04X\n", error);

      if(0 == error)
      {
          appl_spp_print_port_setting(&control->control_data.port);
      }
      break;

    case SPP_SEND_PN:
      PRINTF("Event Type: SPP_SEND_PN\n");
      PRINTF("Status: 0x%04X\n", error);

      if(0 == error)
      {
          PRINTF("DLCI                    : %d\n", control->control_data.pn.dlci);
          PRINTF("MTU                     : %d\n", control->control_data.pn.mtu);
#if 0
          PRINTF("Priority                : %d\n", control->control_data.pn.priority);
          PRINTF("Flow Control            : %d\n", control->control_data.pn.flow_ctrl);
          PRINTF("Initial Count of Credits: %d\n", control->control_data.pn.credits);
#endif
      }
      break;

    case SPP_SEND_LINE_STATUS:
    case SPP_REC_LINE_STATUS:
      if(SPP_SEND_LINE_STATUS == control->type)
      {
          PRINTF("Event Type: SPP_SEND_LINE_STATUS\n");
      }
      else
      {
          PRINTF("Event Type: SPP_REC_LINE_STATUS\n");
      }
      PRINTF("Status: 0x%04X\n", error);

      if(0 == error)
      {
          PRINTF("DLCI  : %d\n", control->control_data.rls.dlci);
          switch(control->control_data.rls.rls)
          {
          case BT_SPP_RLS_NO_ERROR:
            PRINTF("Line Statue: RLS No Error.\n");
            break;

          case BT_SPP_RLS_OVERRUN_ERROR:
            PRINTF("Line Statue: RLS Overrun Error.\n");
            break;

          case BT_SPP_RLS_PARITY_ERROR:
            PRINTF("Line Statue: RLS Parity Error.\n");
            break;

          case BT_SPP_RLS_FRAMEING_ERROR:
            PRINTF("Line Statue: RLS Framing Error.\n");
            break;

          default:
            /* invalid rls */
            break;
          }
      }
      break;

    case SPP_SEND_MSC:
    case SPP_REC_MSC:
      if(SPP_SEND_MSC == control->type)
      {
          PRINTF("Event Type: SPP_SEND_MSC\n");
      }
      else
      {
          PRINTF("Event Type: SPP_REC_MSC\n");
      }
      PRINTF("Status: 0x%04X\n", error);

      if(0 == error)
      {
          PRINTF("DLCI                : %d\n", control->control_data.msc.dlci);
          PRINTF("Flow Control        : %d\n", control->control_data.msc.fc);
          PRINTF("Ready to Communicate: %d\n", control->control_data.msc.rtc);
          PRINTF("Ready to Receive    : %d\n", control->control_data.msc.rtr);
          PRINTF("Incoming Call       : %d\n", control->control_data.msc.ic);
          PRINTF("Data Valid          : %d\n", control->control_data.msc.dv);
      }
      break;

    default:
      /* invalid event */
      break;
    }
}

static void print_spp_appl_info(uint8_t index)
{
    if(index >= CONFIG_BT_SPP_MAX_CONN)
    {
        printf("%d is not a valid spp application index.\n", index);
    }
    else
    {
        if(SPP_APPL_STATE_INITIALIZED != spp_appl[index].state)
        {
            PRINTF("spp handle %d: ", index);

            if(BT_SPP_ROLE_SERVER == spp_appl[index].role)
            {
                PRINTF("server, ");
            }
            else
            {
                PRINTF("client, ");
            }

            PRINTF("channel = %d, ", spp_appl[index].channel);

            if(SPP_APPL_STATE_CONNECTED == spp_appl[index].state)
            {
                PRINTF("connected with device %02X:%02X:%02X:%02X:%02X:%02X.\n", BT_DEVICE_ADDR_POINTER(bt_conn_get_dst_br(spp_appl[index].conn)->val));
            }
            else
            {
                PRINTF("not connected.\n");
            }
        }
    }
}

void spp_appl_init(void)
{
    uint8_t index;

    if(0U == initialized)
    {
        /* Init spp appl handle */
        for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index ++)
        {
            SPP_APPL_INIT(index);
        }

        current_spp_handle = 0xFFU;

        initialized = 1U;
    }
}

void spp_appl_handle_info(void)
{
    uint8_t index;

    for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index++)
    {
        print_spp_appl_info(index);
    }

    if(0xFFU == current_spp_handle)
    {
        PRINTF("No active spp appl handle.\n");
    }
    else
    {
        PRINTF("Current spp appl handle is %d.\n", current_spp_handle);
    }
}

void spp_appl_handle_select(uint8_t handle)
{
    if(handle >= CONFIG_BT_SPP_MAX_CONN)
    {
        PRINTF("spp appl handle %d is out of range, please input spp handle to check valid spp appl handle.\r\n", handle);
    }
    else
    {
        if(SPP_APPL_STATE_INITIALIZED == spp_appl[handle].state)
        {
            PRINTF("spp appl handle %d is invalid, please input spp handle to check valid spp appl handle.\r\n", handle);
        }
        else
        {
            current_spp_handle = handle;
            print_spp_appl_info(current_spp_handle);
            PRINTF("Current spp appl handle is %d.\n", current_spp_handle);
        }
    }
}

void spp_appl_server_register(uint8_t channel)
{
    int     err;
    uint8_t index;

    if ((5U != channel)  && (3U != channel))
    {
        /* Just support 2 rfcomm server now. */
        PRINTF("Channel %d isn't supported, just support channel 5 and 3.\n", channel);
        return;
    }

    /* register sdp record */
    if (5U == channel)
    {
        err = bt_sdp_register_service(&spp_server_5_rec);
        assert(0 == err);
    }
    else if(3U == channel)
    {
        err = bt_sdp_register_service(&spp_server_3_rec);
        assert(0 == err);
    }
    else
    {
        /** Current spp appl just registers channel 3 and 5.
            But host stack supports channel 2-21.
            Please refer channel 3 to register other channel.
          */
    }

    err = bt_spp_server_register(channel, &spp_application_callback);
    if (0 == err)
    {
        index = get_free_spp_appl();
        if(CONFIG_BT_SPP_MAX_CONN == index)
        {
            PRINTF("Max spp appl is reached!\n");
            return;
        }

        spp_appl[index].state   = SPP_APPL_STATE_WAIT_4_CONNECT;
        spp_appl[index].role    = BT_SPP_ROLE_SERVER;
        spp_appl[index].channel = channel;

        current_spp_handle      = index;

        PRINTF("SPP channel %d register successfully, waitting for connected callback!\n", channel);
    }
    else
    {
        PRINTF("SPP channel %d register failed, reason = %d\n",channel, err);
    }
}

void spp_appl_connect(struct bt_conn *conn, uint8_t channel)
{
    uint8_t index;
    int     err;

    index = get_free_spp_appl();
    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        PRINTF("Max spp_appl is reached!\n");
        return;
    }

    err = bt_spp_client_connect(conn, channel, &spp_application_callback, &spp_appl[index].spp_handle);
    if (0 == err)
    {
        spp_appl[index].state   = SPP_APPL_STATE_IN_CONNECT;
        spp_appl[index].role    = BT_SPP_ROLE_CLIENT;
        spp_appl[index].channel = channel;
        spp_appl[index].conn    = conn;

        current_spp_handle      = index;
        PRINTF("SPP appl handle %d connect SPP channel %d successfully, waitting for connected callback!\n", current_spp_handle, channel);
    }
    else
    {
        SPP_APPL_INIT(index);
        PRINTF("SPP appl handle %d connect failed. err = %d\n", current_spp_handle, err);
    }
}

void spp_appl_disconnect(void)
{
    int err;

    if(0xFF == current_spp_handle)
    {
        PRINTF("Current SPP handle %d is invalid.\n", current_spp_handle);
        return;
    }

    err = bt_spp_disconnect(spp_appl[current_spp_handle].spp_handle);

    if (0 != err)
    {
        PRINTF("SPP appl handle %d disconnect failed, reason = %d\n", current_spp_handle, err);
    }
    else
    {
        PRINTF("SPP appl handle %d disconnect successfully, waiting for disconnected callback.\n", current_spp_handle);
    }
}

void spp_appl_send(uint8_t index)
{
    int err;

    if(0xFF == current_spp_handle)
    {
        PRINTF("Current SPP handle %d is invalid.\n", current_spp_handle);
        return;
    }

    if (0xFFU == index)
    {
        PRINTF("the parameter is wrong\r\n");
        return;
    }

     /*
     * 1.  AT+CIND=?\\r\n\
     * 2.  AT+CIND?\\r\n\
     * 3.  ATEP\\r\n\
     * 4.  AT+CKPD=E\\r\n\
     */
    switch( index )
    {
    case 1:
        sprintf((char *)appl_spp_buffer,"AT+CIND=?\\r");
        break;

    case 2:
        sprintf((char *)appl_spp_buffer,"AT+CIND?\\r");
        break;

    case 3:
        sprintf((char *)appl_spp_buffer,"ATEP\\r");
        break;

    case 4:
        sprintf((char *)appl_spp_buffer,"AT+CKPD=E\\r");
        break;

    default:
        PRINTF("Invalid choice. Defaulting to AT+CIND=?\\r\n");
        sprintf((char *)appl_spp_buffer,"AT+CIND=?\\r");
        break;
    } /* switch */

    err = bt_spp_data_send
             (
                 spp_appl[current_spp_handle].spp_handle,
                 appl_spp_buffer,
                 (uint16_t)sizeof(appl_spp_buffer)
             );

    if (0 != err)
    {
        PRINTF("SPP appl handle %d send string failed, reason = %d\n",current_spp_handle, err);
    }
    else
    {
        PRINTF("SPP appl handle %d send string successfully, waiting for data sent callback.\n", current_spp_handle);
    }
}

static void appl_spp_print_port_setting(struct bt_spp_port * port)
{
    PRINTF("DLCI       : %d\n", port->dlci);
    PRINTF("Baud Rate  : ");
    switch (port->baud_rate)
    {
    case SPP_PORT_BAUD_RATE_2400:
        PRINTF("2400\n");
        break;
    case SPP_PORT_BAUD_RATE_4800:
        PRINTF("4800\n");
        break;
    case SPP_PORT_BAUD_RATE_7200:
        PRINTF("7200\n");
        break;
    case SPP_PORT_BAUD_RATE_9600:
        PRINTF("9600 (Default)\n");
        break;
    case SPP_PORT_BAUD_RATE_19200:
        PRINTF("19200\n");
        break;
    case SPP_PORT_BAUD_RATE_38400:
        PRINTF("38400\n");
        break;
    case SPP_PORT_BAUD_RATE_57600:
        PRINTF("57600\n");
        break;
    case SPP_PORT_BAUD_RATE_115200:
        PRINTF("115200\n");
        break;
    case SPP_PORT_BAUD_RATE_230400:
        PRINTF("230400\n");
        break;
    default:
        PRINTF("????\n");
        break;
    }

    PRINTF("Data Bits  :");
    switch (port->data_bits)
    {
    case SPP_PORT_DATA_BITS_5:
        PRINTF("5\n");
        break;
    case SPP_PORT_DATA_BITS_6:
        PRINTF("6\n");
        break;
    case SPP_PORT_DATA_BITS_7:
        PRINTF("7\n");
        break;
    case SPP_PORT_DATA_BITS_8:
        PRINTF("8 (Default)\n");
        break;
    default:
        PRINTF("????\n");
        break;
    }

    PRINTF("Stop Bit   :");
    switch (port->stop_bit)
    {
    case SPP_PORT_STOP_BIT_1:
        PRINTF("1 (Default)\n");
        break;
    case SPP_PORT_STOP_BIT_1_5:
        PRINTF("1/5\n");
        break;
    default:
        PRINTF("????\n");
        break;
    }

    PRINTF("Parity     :");
    switch (port->parity)
    {
    case SPP_PORT_PARITY_NONE:
        PRINTF("None (Default)\n");
        break;
    case SPP_PORT_PARITY_SET:
        PRINTF("Set\n");
        break;
    default:
        PRINTF("????\n");
        break;
    }

    PRINTF("Parity Type: ");
    switch (port->parity_type)
    {
    case SPP_PORT_PARITY_TYPE_ODD:
        if (SPP_PORT_PARITY_NONE == port->parity)
        {
            PRINTF("None (Default)\n");
        }
        else
        {
            PRINTF("Odd\n");
        }
        break;
    case SPP_PORT_PARITY_TYPE_EVEN:
        PRINTF("Even\n");
        break;
    case SPP_PORT_PARITY_TYPE_MARK:
        PRINTF("Mark\n");
        break;
    case SPP_PORT_PARITY_TYPE_SPACE:
        PRINTF("Space\n");
        break;
    default:
        PRINTF("????\n");
        break;
    }
}

void spp_appl_get_server_port(uint8_t channel)
{
    int err;

    err = bt_spp_request_port_setting(default_conn, channel, BT_SPP_ROLE_SERVER, &spp_application_callback);
    if(0 == err)
    {
        PRINTF("SPP appl get server channel %d port setting successfully, waiting for callback.\n", channel);
    }
    else if(-57 == err)
    {
        PRINTF("SPP appl get server channel %d port setting failed because rfcomm dlci isn't connected.\n", channel);
    }
    else
    {
        PRINTF("SPP appl get server channel %d port setting failed, reason = %d\n", channel, err);
    }
}

void spp_appl_get_client_port(uint8_t channel)
{
    int err;

    err = bt_spp_request_port_setting(default_conn, channel, BT_SPP_ROLE_CLIENT, &spp_application_callback);
    if(0 == err)
    {
        PRINTF("SPP appl get client channel %d port setting successfully, waiting for callback.\n", channel);
    }
    else if(-57 == err)
    {
        PRINTF("SPP appl get client channel %d port setting failed because rfcomm dlci isn't connected.\n", channel);
    }
    else
    {
        PRINTF("SPP appl get client channel %d port setting failed, reason = %d\n", channel, err);
    }
}

void spp_appl_set_server_port(uint8_t channel)
{
    int err;
    uint8_t index;

    struct bt_spp_port spp_appl_port_setting =
    {
        .baud_rate   = SPP_PORT_BAUD_RATE_57600,
        .data_bits   = SPP_PORT_DATA_BITS_5,
        .stop_bit    = SPP_PORT_STOP_BIT_1,
        .parity      = SPP_PORT_PARITY_SET,
        .parity_type = SPP_PORT_PARITY_TYPE_EVEN,
    };

    for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index++)
    {
        if((SPP_APPL_STATE_CONNECTED == spp_appl[index].state) &&
           (BT_SPP_ROLE_SERVER == spp_appl[index].role) &&
           (channel == spp_appl[index].channel) &&
           (default_conn == spp_appl[index].conn))
        {
            break;
        }
    }

    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        PRINTF("SPP channel %d server isn't connected, just port setting of client can be set.\n", channel);
        return;
    }

    err = bt_spp_negotiate_port_setting(default_conn, channel, BT_SPP_ROLE_SERVER, &spp_application_callback, &spp_appl_port_setting);

    if(0 == err)
    {
        PRINTF("SPP appl set server channel %d port setting successfully, waiting for callback.\n", channel);
    }
    else
    {
        PRINTF("SPP appl set server channel %d port setting failed, reason = %d\n", channel, err);
    }
}

void spp_appl_set_client_port(uint8_t channel)
{
    int err;
    struct bt_spp_port spp_appl_port_setting =
    {
        .baud_rate   = SPP_PORT_BAUD_RATE_115200,
        .data_bits   = SPP_PORT_DATA_BITS_7,
        .stop_bit    = SPP_PORT_STOP_BIT_1_5,
        .parity      = SPP_PORT_PARITY_SET,
        .parity_type = SPP_PORT_PARITY_TYPE_ODD,
    };

    err = bt_spp_negotiate_port_setting(default_conn, channel, BT_SPP_ROLE_CLIENT, &spp_application_callback, &spp_appl_port_setting);

    if(0 == err)
    {
        PRINTF("SPP appl set client channel %d port setting successfully, waiting for callback.\n", channel);
    }
    else
    {
        PRINTF("SPP appl set client channel %d port setting failed, reason = %d\n", channel, err);
    }
}

void spp_appl_set_server_pn(uint8_t channel)
{
    int              err;
    uint8_t          index;
    struct bt_spp_pn spp_pn;

    memset(&spp_pn, 0U, sizeof(struct bt_spp_pn));
    spp_pn.mtu = 25U;

    for(index = 0U; index < CONFIG_BT_SPP_MAX_CONN; index++)
    {
        if((SPP_APPL_STATE_CONNECTED == spp_appl[index].state) &&
           (BT_SPP_ROLE_SERVER == spp_appl[index].role) &&
           (channel == spp_appl[index].channel) &&
           (default_conn == spp_appl[index].conn))
        {
            break;
        }
    }

    if(CONFIG_BT_SPP_MAX_CONN == index)
    {
        PRINTF("SPP channel %d server isn't connected, just pn of client can be set.\n", channel);
        return;
    }

    err = bt_spp_send_pn(default_conn, channel, BT_SPP_ROLE_SERVER, &spp_application_callback, &spp_pn);
    if(0 == err)
    {
        PRINTF("SPP appl send server channel %d parameter negotiation successfully, waiting for callback.\n", channel);
    }
    else
    {
        PRINTF("SPP appl set server channel %d parameter negotiation failed, reason = %d\n", channel, err);
    }
}

void spp_appl_set_client_pn(uint8_t channel)
{
    int err;
    struct bt_spp_pn spp_pn;

    memset(&spp_pn, 0U, sizeof(struct bt_spp_pn));
    spp_pn.mtu = 25U;

    err = bt_spp_send_pn(default_conn, channel, BT_SPP_ROLE_CLIENT, &spp_application_callback, &spp_pn);
    if(0 == err)
    {
        PRINTF("SPP appl send server channel %d parameter negotiation successfully, waiting for callback.\n", channel);
    }
    else
    {
        PRINTF("SPP appl set server channel %d parameter negotiation failed, reason = %d\n", channel, err);
    }
}

void spp_appl_get_local_server_pn(uint8_t channel)
{
    int              err;
    struct bt_spp_pn spp_pn;

    memset(&spp_pn, 0U, sizeof(struct bt_spp_pn));
    err = bt_spp_get_local_pn(default_conn, channel, BT_SPP_ROLE_SERVER, &spp_pn);
    if (0 == err)
    {
        PRINTF("SPP appl get local pn on channel server successfully, pn value is :%d ",current_spp_handle, channel);
        PRINTF("DLCI      : %d\n", spp_pn.dlci);
        PRINTF("MTU       : %d\n", spp_pn.mtu);
        PRINTF("Priority  : %d\n", spp_pn.priority);
#if 0
        PRINTF("Flow Control            : %d\n", spp_pn.flow_ctrl);
        PRINTF("Initial Count of Credits: %d\n", spp_pn.credits);
#endif
    }
    else if(-57 == err)
    {
        PRINTF("SPP appl get local pn on server channel %d failed because rfcomm dlci isn't connected.\n", channel);
    }
    else
    {
        PRINTF("SPP appl get local pn on server channel %d failed, reason = %d",channel, err);
    }
}

void spp_appl_get_local_client_pn(uint8_t channel)
{
    int              err;
    struct bt_spp_pn spp_pn;

    memset(&spp_pn, 0U, sizeof(struct bt_spp_pn));
    err = bt_spp_get_local_pn(default_conn, channel, BT_SPP_ROLE_CLIENT, &spp_pn);
    if (0 == err)
    {
        PRINTF("SPP appl get local pn on channel client successfully, pn value is :%d ",current_spp_handle, channel);
        PRINTF("DLCI     : %d\n", spp_pn.dlci);
        PRINTF("MTU      : %d\n", spp_pn.mtu);
        PRINTF("Priority : %d\n", spp_pn.priority);
#if 0
        PRINTF("Flow Control            : %d\n", spp_pn.flow_ctrl);
        PRINTF("Initial Count of Credits: %d\n", spp_pn.credits);
#endif
    }
    else if(-57 == err)
    {
        PRINTF("SPP appl get local pn on client channel %d failed because rfcomm dlci isn't connected.\n", channel);
    }
    else
    {
        PRINTF("SPP appl get local pn on client channel %d failed, reason = %d", channel, err);
    }
}

void spp_appl_send_rls(void)
{
    int               err;
    struct bt_spp_rls spp_rls;

    if(0xFF == current_spp_handle)
    {
        PRINTF("Current SPP handle %d is invalid.\n", current_spp_handle);
        return;
    }
    else
    {
        spp_rls.rls = BT_SPP_RLS_FRAMEING_ERROR;

        err = bt_spp_send_rls(spp_appl[current_spp_handle].spp_handle, &spp_rls);
        if (0 != err)
        {
            PRINTF("SPP appl handle %d send rls failed, reason = %d\n",current_spp_handle, err);
        }
        else
        {
            PRINTF("SPP appl handle %d send rls successfully, waiting for callback.\n", current_spp_handle);
        }
    }
}

void spp_appl_send_msc(void)
{
    int               err;
    struct bt_spp_msc spp_msc;

    if(0xFF == current_spp_handle)
    {
        PRINTF("Current SPP handle %d is invalid.\n", current_spp_handle);
        return;
    }
    else
    {
        memset(&spp_msc, 0U, sizeof(struct bt_spp_msc));
        spp_msc.fc  = 0U;
        spp_msc.rtc = 1U;
        spp_msc.rtr = 1U;
        spp_msc.ic  = 0U;
        spp_msc.dv  = 1U;

        err = bt_spp_send_msc(spp_appl[current_spp_handle].spp_handle, &spp_msc);
        if (0 != err)
        {
            PRINTF("SPP appl handle %d send msc failed, reason = %d\n",current_spp_handle, err);
        }
        else
        {
            PRINTF("SPP appl handle %d send msc successfully, waiting for callback.\n", current_spp_handle);
        }
    }
}
