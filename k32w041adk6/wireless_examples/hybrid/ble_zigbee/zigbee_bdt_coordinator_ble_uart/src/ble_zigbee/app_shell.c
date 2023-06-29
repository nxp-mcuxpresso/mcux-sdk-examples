/*
* Copyright 2016-2020 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <jendefs.h>
#include "EmbeddedTypes.h"
#include "shell.h"
#include "bdb_api.h"
#include "bdb_DeviceCommissioning.h"
#include "app_common.h"
#include "ZQueue.h"
#include "app_main.h"
#include "app_coordinator.h"
#include "app_dual_mode_switch.h"

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

typedef enum
{

    E_TLV_TAG_UNENCRYPTED_NETWORK_INFO_PAYLOAD  = 0xa0,
    E_TLV_TAG_ENCRYPTED_NETWORK_INFO_PAYLOAD    = 0xa1,
    E_TLV_TAG_DEVICE_INFO_PAYLOAD               = 0xa2,

    E_TLV_TAG_NODE_ADDR                         = 0xc0,
    E_TLV_TAG_INST_CODE                         = 0xc1,
    E_TLV_TAG_CRC                               = 0xc2,


    E_TLV_TAG_NETWORK_KEY                       = 0xf0,
    E_TLV_TAG_TRUST_CENTER_ADDR                 = 0xf1,
    E_TLV_TAG_EXT_PAN_ID                        = 0xf2,
    E_TLV_TAG_PAN_ID                            = 0xf3,
    E_TLV_TAG_ACTIVE_SQ_NB                      = 0xf4,
    E_TLV_TAG_CHANNEL                           = 0xf5,
    E_TLV_TAG_MIC                               = 0xf6,
} TLV_Tag;

typedef struct 
{
    uint8_t tag;
    uint8_t len;
    uint8_t *pData;
} structTLV_t;


#define TLV_TAG_SIZE sizeof(((structTLV_t *)0)->tag)
#define TLV_LEN_SIZE sizeof(((structTLV_t *)0)->len)
#define TLV_HEADER_SIZE (TLV_TAG_SIZE + TLV_LEN_SIZE)

#define DEV_ADDR_SIZE sizeof(((struct dev_info *)0)->addr)
#define DEV_INSTCODE_SIZE sizeof(((struct dev_info *)0)->instCode)
#define DEV_CRC_SIZE sizeof(((struct dev_info *)0)->crc)

#define DEV_INFO_SIZE sizeof(struct dev_info)
#define TLV_DEV_INFO_SIZE (DEV_INFO_SIZE + 3 * TLV_HEADER_SIZE)

#define OOB_KEY_SIZE sizeof(((struct oob_info *)0)->key)
#define OOB_ADDR_SIZE sizeof(((struct oob_info *)0)->tcAddress)
#define OOB_PI_SIZE sizeof(((struct oob_info *)0)->panId)
#define OOB_SPI_SIZE sizeof(((struct oob_info *)0)->shortPanId)
#define OOB_SQ_SIZE sizeof(((struct oob_info *)0)->activeKeySeq)
#define OOB_CH_SIZE sizeof(((struct oob_info *)0)->channel)
#define OOB_MIC_SIZE sizeof(((struct oob_info_enc *)0)->mic)

#define OOB_INFO_SIZE sizeof(struct oob_info)
#define TLV_OOB_INFO_SIZE (OOB_INFO_SIZE + 6 * TLV_HEADER_SIZE)

#define OOB_INFO_ENC_SIZE sizeof(struct oob_info_enc)
#define TLV_OOB_INFO_ENC_SIZE (OOB_INFO_ENC_SIZE + 7 * TLV_HEADER_SIZE)

/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/* Shell callbacks */
static int8_t SHELL_ZigbeeGetNetworkStatus(uint8_t argc, char *argv[]);
static int8_t SHELL_ZigbeeGetNetworkInfo(uint8_t argc, char *argv[]);
static int8_t SHELL_ZigbeeCreateNetwork(uint8_t argc, char *argv[]);
static int8_t SHELL_ZigbeeFindInitiator(uint8_t argc, char *argv[]);
static int8_t SHELL_ZigbeeToggle(uint8_t argc, char *argv[]);
static int8_t SHELL_ZigbeeFactoryReset(uint8_t argc, char *argv[]);

/* Other functions */
static int8_t SHELL_CopyCharTableToByteBuffer(const char *pCharTable, uint8_t charTableLen, uint8_t *pResult);

static bool_t SHELL_read_device_info(const char* arg, struct dev_info* info);
static void SHELL_write_oob_info(const struct oob_info_enc* oob, bool_t enc);

static void SHELL_LogTlvData(const structTLV_t *pTLV, bool_t printData);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

const cmd_tbl_t gZigBee_commands[] = 
{
    {
        "zigbee_get_network_status", 1, 0, SHELL_ZigbeeGetNetworkStatus
#if SHELL_USE_HELP
        ,"Returns the network status"
        ,"Returns the network status NETWORK_CREATED or NETWORK_NOT_CREATED"
#endif /* SHELL_USE_HELP */
#if SHELL_USE_AUTO_COMPLETE
        ,NULL
#endif /* SHELL_USE_AUTO_COMPLETE */
    },
    {
        "zigbee_get_network_info", 2, 0, SHELL_ZigbeeGetNetworkInfo
#if SHELL_USE_HELP
        ,"Returns the network info"
        ,"Returns the network info, Network Info would be serialized in the TLV format if created or NETWORK_NOT_CREATED (if the network is not formed)"
#endif /* SHELL_USE_HELP */
#if SHELL_USE_AUTO_COMPLETE
        ,NULL
#endif /* SHELL_USE_AUTO_COMPLETE */
    },
    {
        "zigbee_create_network", 1, 0, SHELL_ZigbeeCreateNetwork
#if SHELL_USE_HELP
        ,"Allows to create a zigbee network"
        ,"Allows to create a zigbee network, NETWORK_ALREADY_CREATED will be retured if the network is already formed"
#endif /* SHELL_USE_HELP */
#if SHELL_USE_AUTO_COMPLETE
        ,NULL
#endif /* SHELL_USE_AUTO_COMPLETE */
    },
    {
        "zigbee_find", 1, 0, SHELL_ZigbeeFindInitiator
#if SHELL_USE_HELP
        ,"Start Find & Bind as an initiator"
        ,"Start Find & Bind as an initiator, make sure to trigger Find & Bind on a TARGET device"
#endif /* SHELL_USE_HELP */
#if SHELL_USE_AUTO_COMPLETE
        ,NULL
#endif /* SHELL_USE_AUTO_COMPLETE */
    },
    {
        "zigbee_toggle", 1, 0, SHELL_ZigbeeToggle
#if SHELL_USE_HELP
        ,"Sends on\\off toggle command to bound devices"
        ,"Sends on\\off toggle command to bound devices"
#endif /* SHELL_USE_HELP */
#if SHELL_USE_AUTO_COMPLETE
        ,NULL
#endif /* SHELL_USE_AUTO_COMPLETE */
    },
    {
        "zigbee_factoryreset", 1, 0, SHELL_ZigbeeFactoryReset
#if SHELL_USE_HELP
        ,"Resets the device to its factory state"
        ,"Resets the device to its factory state"
#endif /* SHELL_USE_HELP */
#if SHELL_USE_AUTO_COMPLETE
        ,NULL
#endif /* SHELL_USE_AUTO_COMPLETE */
    },
};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*!*************************************************************************************************
\brief  Initialize Shell.
***************************************************************************************************/
void app_shell_init(void)
{
    static const char * prompt = "$ ";
    shell_init((char*)prompt);
    shell_register_function_array((cmd_tbl_t*)gZigBee_commands, NumberOfElements(gZigBee_commands));
}


/************************************************************************************
*************************************************************************************
* Private functions - Shell command callbacks
*************************************************************************************
************************************************************************************/

static int8_t SHELL_ZigbeeGetNetworkStatus(uint8_t argc, char *argv[])
{
    if (FALSE == sBDB.sAttrib.bbdbNodeIsOnANetwork)
    {
        shell_write("NETWORK_NOT_CREATED\n");
    }
    else
    {
        shell_write("NETWORK_CREATED\n");
    }
    shell_write("DONE");
    return CMD_RET_SUCCESS;
}

static int8_t SHELL_ZigbeeGetNetworkInfo(uint8_t argc, char *argv[])
{
    bool_t valid = TRUE;
    bool_t enc = FALSE;
    struct dev_info dev_info;
    struct oob_info_enc oob_info;

    switch (argc)
    {
        case 2:
            enc = TRUE;
            valid = SHELL_read_device_info(argv[1], &dev_info);
        case 1:
            break;
        default:
            valid = FALSE;
            shell_write("WRONG_ARG\n");
    }

    if (valid)
    {
        valid = APP_GetOOBInfo(enc ? &dev_info : NULL, &oob_info);
    }
    if (valid)
    {
        SHELL_write_oob_info(&oob_info, enc);
        shell_write("\n");
    }
    else
    {
        shell_write("NO OOB INFO\n");
    }
    shell_write("DONE");
    return CMD_RET_SUCCESS;
}

static int8_t SHELL_ZigbeeCreateNetwork(uint8_t argc, char *argv[])
{
    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_E_EVENT_NONE;
    if (FALSE == sBDB.sAttrib.bbdbNodeIsOnANetwork)
    {
        sButtonEvent.eType = APP_E_EVENT_SERIAL_FORM_NETWORK;
        if(!ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent))
        {
            shell_write("NETWORK_CREATION_FAILURE\n");
        }
    }
    else
    {
        shell_write("NETWORK_ALREADY_CREATED\n");
    }
    shell_write("DONE");
    return CMD_RET_SUCCESS;
}

static int8_t SHELL_ZigbeeFindInitiator(uint8_t argc, char *argv[])
{
    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_E_EVENT_NONE;
    if (sBDB.sAttrib.bbdbNodeIsOnANetwork)
    {
        sButtonEvent.eType = APP_E_EVENT_SERIAL_FIND_BIND_START;
        if(!ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent))
        {
            shell_write("FIND_&_BIND_FAILURE\n");
        }
    }
    else
    {
        shell_write("NETWORK_NOT_CREATED\n");
    }
    shell_write("DONE");
    return CMD_RET_SUCCESS;
}

static int8_t SHELL_ZigbeeToggle(uint8_t argc, char *argv[])
{
    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_E_EVENT_NONE;
    if (sBDB.sAttrib.bbdbNodeIsOnANetwork)
    {
        sButtonEvent.eType = APP_E_EVENT_SERIAL_TOGGLE;
        if(!ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent))
        {
            shell_write("TOGGLE_FAILURE\n");
        }
    }
    else
    {
        shell_write("NETWORK_NOT_CREATED\n");
    }
    shell_write("DONE");
    return CMD_RET_SUCCESS;
}

static int8_t SHELL_ZigbeeFactoryReset(uint8_t argc, char *argv[])
{
    shell_write("DONE");
    dm_switch_processEvent((void *) e15_4FactoryResetEvent);
    return CMD_RET_SUCCESS;
}

/************************************************************************************
*************************************************************************************
* Private functions - Other util functions
*************************************************************************************
************************************************************************************/

static bool_t SHELL_read_device_info(const char* arg, struct dev_info* info)
{
    uint32_t idx = 0;
    structTLV_t tmp_tlv;

    if (strlen(arg) != (TLV_HEADER_SIZE + TLV_DEV_INFO_SIZE) * 2)
        return FALSE;

    if (SHELL_CopyCharTableToByteBuffer(arg, 4, (uint8_t *)&tmp_tlv))
        return FALSE;

    if (tmp_tlv.tag != E_TLV_TAG_DEVICE_INFO_PAYLOAD || tmp_tlv.len != TLV_DEV_INFO_SIZE)
        return FALSE;

    /* addr */
    idx += 4;
    if (SHELL_CopyCharTableToByteBuffer(arg + idx, 4, (uint8_t *)&tmp_tlv))
        return FALSE;

    if (tmp_tlv.tag != E_TLV_TAG_NODE_ADDR || tmp_tlv.len != DEV_ADDR_SIZE)
        return FALSE;

    idx += 4;
    if (SHELL_CopyCharTableToByteBuffer(arg + idx, tmp_tlv.len * 2, (uint8_t *)&info->addr))
            return FALSE;

    /* install code */
    idx += tmp_tlv.len * 2;
    if (SHELL_CopyCharTableToByteBuffer(arg + idx, 4, (uint8_t *)&tmp_tlv))
        return FALSE;

    if (tmp_tlv.tag != E_TLV_TAG_INST_CODE || tmp_tlv.len != DEV_INSTCODE_SIZE)
        return FALSE;

    idx += 4;
    if (SHELL_CopyCharTableToByteBuffer(arg + idx, tmp_tlv.len * 2, (uint8_t *)&info->instCode))
            return FALSE;

    /* CRC */
    idx += tmp_tlv.len * 2;
    if (SHELL_CopyCharTableToByteBuffer(arg + idx, 4, (uint8_t *)&tmp_tlv))
        return FALSE;

    if (tmp_tlv.tag != E_TLV_TAG_CRC || tmp_tlv.len != DEV_CRC_SIZE)
        return FALSE;

    idx += 4;
    if (SHELL_CopyCharTableToByteBuffer(arg + idx, tmp_tlv.len * 2, (uint8_t *)&info->crc))
            return FALSE;

    return TRUE;
}

static void SHELL_write_oob_info(const struct oob_info_enc* oob, bool_t enc)
{
    structTLV_t tmp_tlv;

    if (enc)
    {
        tmp_tlv.tag = E_TLV_TAG_ENCRYPTED_NETWORK_INFO_PAYLOAD;
        tmp_tlv.len = TLV_OOB_INFO_ENC_SIZE;
    }
    else
    {
        tmp_tlv.tag = E_TLV_TAG_UNENCRYPTED_NETWORK_INFO_PAYLOAD;
        tmp_tlv.len = TLV_OOB_INFO_SIZE;
    }
    SHELL_LogTlvData(&tmp_tlv, FALSE);

    tmp_tlv.tag = E_TLV_TAG_NETWORK_KEY;
    tmp_tlv.len = OOB_KEY_SIZE;
    tmp_tlv.pData = (uint8_t *)&oob->info.key;
    SHELL_LogTlvData(&tmp_tlv, TRUE);

    tmp_tlv.tag = E_TLV_TAG_TRUST_CENTER_ADDR;
    tmp_tlv.len = OOB_ADDR_SIZE;
    tmp_tlv.pData = (uint8_t *)&oob->info.tcAddress;
    SHELL_LogTlvData(&tmp_tlv, TRUE);

    tmp_tlv.tag = E_TLV_TAG_EXT_PAN_ID;
    tmp_tlv.len = OOB_PI_SIZE;
    tmp_tlv.pData = (uint8_t *)&oob->info.panId;
    SHELL_LogTlvData(&tmp_tlv, TRUE);

    tmp_tlv.tag = E_TLV_TAG_PAN_ID;
    tmp_tlv.len = OOB_SPI_SIZE;
    tmp_tlv.pData = (uint8_t *)&oob->info.shortPanId;
    SHELL_LogTlvData(&tmp_tlv, TRUE);

    tmp_tlv.tag = E_TLV_TAG_ACTIVE_SQ_NB;
    tmp_tlv.len = OOB_SQ_SIZE;
    tmp_tlv.pData = (uint8_t *)&oob->info.activeKeySeq;
    SHELL_LogTlvData(&tmp_tlv, TRUE);

    tmp_tlv.tag = E_TLV_TAG_CHANNEL;
    tmp_tlv.len = OOB_CH_SIZE;
    tmp_tlv.pData = (uint8_t *)&oob->info.channel;
    SHELL_LogTlvData(&tmp_tlv, TRUE);

    if (enc)
    {
        tmp_tlv.tag = E_TLV_TAG_MIC;
        tmp_tlv.len = OOB_MIC_SIZE;
        tmp_tlv.pData = (uint8_t *)&oob->mic;
        SHELL_LogTlvData(&tmp_tlv, TRUE);

    }
}

static int8_t SHELL_ExtractHexValueFromChar(char ch, uint8_t *pResult)
{
    int8_t status = 0;
    if (ch >= '0' && ch <= '9')
    {
        *pResult = ch - '0';
    }
    else if (ch >= 'a' && ch <= 'f')
    {
        *pResult = ch - 'a' + 0xa;
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        *pResult = ch - 'A' + 0xa;
    }
    else
    {
        status = -1;
    }
    return status;
}

static int8_t SHELL_CopyCharTableToByteBuffer(const char *pCharTable, uint8_t charTableLen, uint8_t *pResult)
{
    int8_t status = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t value1 = 0;
    uint8_t value2 = 0;
    for (i=0; i<charTableLen; i+=2)
    {
        if (SHELL_ExtractHexValueFromChar(pCharTable[i], &value1) != 0)
        {
            status = -1;
            break;
        }
        if (SHELL_ExtractHexValueFromChar(pCharTable[i+1], &value2) != 0)
        {
            status = -1;
            break;
        }
        pResult[j++] = (value1<<4)|value2;
    }
    return status;
}

static void SHELL_LogTlvData(const structTLV_t *pTLV, bool_t printData)
{
    shell_writeHex((uint8_t *) &pTLV->tag, sizeof(pTLV->tag));
    shell_writeHex((uint8_t *) &pTLV->len, sizeof(pTLV->len));
    if (printData)
        shell_writeHex((uint8_t *) pTLV->pData, pTLV->len);
}

