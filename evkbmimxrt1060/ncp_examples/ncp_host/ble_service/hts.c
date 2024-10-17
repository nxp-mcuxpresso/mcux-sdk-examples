/** @file hts.c
 *
 *  @brief  This file provides the health thermometer service profile.
 *
 *  Copyright 2023 - 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#include "fsl_os_abstraction.h"
#include "ncp_host_command_ble.h"

#include "service.h"
#include "ht.h"

#if (CONFIG_NCP_HTS)

/**
 * @brief Health Thermometer Service (HTS)
 * @defgroup bt_hts Health Thermometer Service (HTS)
 * @ingroup bluetooth
 * @{
 */

/*******************************************************************************
 * Variables
 ******************************************************************************/
static OSA_TASK_HANDLE_DEFINE(hts_service_thread);
static void peripheral_hts_task(void *pvParameters);
static OSA_TASK_DEFINE(peripheral_hts_task, NCP_BLE_SERVICE_PRIO, 1, 1024, 0);
static OSA_EVENT_HANDLE_DEFINE(hts_events);

// ble-set-value command paramters
NCP_SET_VALUE_CMD hts_value;
bool hts_cccd_written = false;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void write_charateristic_command_local(NCP_SET_VALUE_CMD *param);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static struct host_gatt_attr hts_profile [] = {
    /* HTS Promary Serivce Declaration */
    GATT_PRIMARY_SERVICE(UUID_HTS),
    /* HTS Measurement Charatristic Declaration */
    GATT_CHARACTERISTIC(UUID_HTS_MEASUREMENT, BT_GATT_CHRC_INDICATE, BT_GATT_PERM_NONE),
    /* Client Characteristic Configuration of HTS MeasurementDeclaration */
    GATT_CCC(BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), 
};
/*******************************************************************************
 * Code
 ******************************************************************************/
static void peripheral_hts_event_get(osa_event_flags_t flag)
{
    uint32_t Events;
    (void)OSA_EventWait((osa_event_handle_t)hts_events, flag, 0, osaWaitForever_c, &Events);
}

void peripheral_hts_event_put(osa_event_flags_t flag)
{
    (void)OSA_EventSet((osa_event_handle_t)hts_events, flag);
}

void peripheral_hts_start(void)
{
    static bool is_started = false;
    if(!is_started)
    {
        is_started = true;
        hts_init();
        printf("HTS profile at host side starting...\n");
        return;
    }
    printf("HTS profile at host side already started\n");
    return;
}

void peripheral_hts_indicate(uint8_t value)
{
    hts_cccd_written = (value == BT_GATT_CCC_INDICATE) ? true : false;
}

static void peripheral_hts_task(void *pvParameters)
{
    /* Temperature measurements simulation */
    static uint32_t temperature = 20U;
    static uint8_t temp_type = hts_no_temp_type;
    struct temp_measurement temp_measurement;

    int argc = ARRAY_SIZE(hts_profile);
    int arg = 0;

    MCU_NCPCmd_DS_COMMAND *host_service_add_command = ncp_host_get_cmd_buffer_ble();
    NCP_CMD_SERVICE_ADD *host_service_add_tlv       = (NCP_CMD_SERVICE_ADD *)&host_service_add_command->params.host_svc_add;
    uint8_t *ptlv_pos                               = host_service_add_tlv->tlv_buf;
    uint32_t tlv_buf_len                            = 0;
    gatt_add_service_cmd_t *add_service_tlv         = NULL;
    gatt_add_characteristic_cmd_t *add_chrc_tlv     = NULL;
    gatt_add_descriptor_cmd_t *add_desc_tlv         = NULL;

    /*Wait for command response semaphore.*/
    mcu_get_command_resp_sem();
    printf("Send 'ble-host-svc-add prim 1809 chrc 2a1c 20 00 ccc 2902 03 start' to add HTS profile to ncp device side\n");

    do
    {
        if(hts_profile[arg].type == NCP_CMD_GATT_ADD_SERVICE_TLV)
        {
            add_service_tlv = (gatt_add_service_cmd_t *)ptlv_pos;
            add_service_tlv->type = 0;// primary service
            add_service_tlv->uuid_length = hts_profile[arg].uuid_length;
            memcpy(add_service_tlv->uuid, hts_profile[arg].uuid, add_service_tlv->uuid_length);
            add_service_tlv->header.type = NCP_CMD_GATT_ADD_SERVICE_TLV;
            add_service_tlv->header.size = sizeof(gatt_add_service_cmd_t) - NCP_TLV_HEADER_LEN - (SERVER_MAX_UUID_LEN - add_service_tlv->uuid_length);

            ptlv_pos += add_service_tlv->header.size + NCP_TLV_HEADER_LEN;
            tlv_buf_len += add_service_tlv->header.size + NCP_TLV_HEADER_LEN;
            arg ++;
        }
        else if(hts_profile[arg].type == NCP_CMD_GATT_ADD_CHRC_TLV)
        {
            add_chrc_tlv = (gatt_add_characteristic_cmd_t *)ptlv_pos;
            add_chrc_tlv->svc_id = 0;
            add_chrc_tlv->uuid_length = hts_profile[arg].uuid_length;
            memcpy(add_chrc_tlv->uuid, hts_profile[arg].uuid, add_chrc_tlv->uuid_length);
            add_chrc_tlv->properties = hts_profile[arg].properties;
            add_chrc_tlv->permissions = hts_profile[arg].permissions;

            add_chrc_tlv->header.type = NCP_CMD_GATT_ADD_CHRC_TLV;
            add_chrc_tlv->header.size = sizeof(gatt_add_characteristic_cmd_t) - NCP_TLV_HEADER_LEN - (SERVER_MAX_UUID_LEN - add_chrc_tlv->uuid_length);
            ptlv_pos += add_chrc_tlv->header.size + NCP_TLV_HEADER_LEN;
            tlv_buf_len += add_chrc_tlv->header.size + NCP_TLV_HEADER_LEN;
            arg ++;

        }
        else if(hts_profile[arg].type == NCP_CMD_GATT_ADD_DESC_TLV)
        {
            add_desc_tlv = (gatt_add_descriptor_cmd_t *)ptlv_pos;
            add_desc_tlv->char_id = 0;
            add_desc_tlv->uuid_length = hts_profile[arg].uuid_length;
            memcpy(add_desc_tlv->uuid, hts_profile[arg].uuid, add_desc_tlv->uuid_length);
            add_desc_tlv->permissions = hts_profile[arg].permissions;

            add_desc_tlv->header.type = NCP_CMD_GATT_ADD_DESC_TLV;
            add_desc_tlv->header.size = sizeof(gatt_add_descriptor_cmd_t) - NCP_TLV_HEADER_LEN - (SERVER_MAX_UUID_LEN - add_desc_tlv->uuid_length);
            ptlv_pos += add_desc_tlv->header.size + NCP_TLV_HEADER_LEN;
            tlv_buf_len += add_desc_tlv->header.size + NCP_TLV_HEADER_LEN;
            arg ++;
        }
    } while (arg < argc);

#if defined(NCP_BLE_HOST_SERVICE_AUTO_START)
    gatt_start_service_cmd_t *start_service_tlv = (gatt_start_service_cmd_t *)ptlv_pos;

    start_service_tlv->started = 1;
    start_service_tlv->header.type = NCP_CMD_GATT_START_SVC_TLV;
    start_service_tlv->header.size = sizeof(gatt_start_service_cmd_t) - NCP_TLV_HEADER_LEN;
    ptlv_pos += sizeof(gatt_start_service_cmd_t);
    tlv_buf_len += sizeof(gatt_start_service_cmd_t);
#endif

    host_service_add_tlv->tlv_buf_len = tlv_buf_len;

    host_service_add_command->header.cmd      = NCP_CMD_BLE_HOST_SERVICE_ADD;
    host_service_add_command->header.size     = NCP_CMD_HEADER_LEN + sizeof(host_service_add_tlv->tlv_buf_len) + tlv_buf_len;
    host_service_add_command->header.result   = NCP_CMD_RESULT_OK;

    /* Send host service add command */
    ncp_host_send_tlv_command();

    while(1)
    {
        if(hts_cccd_written)
        {
            printf("temperature is %ldC\n", temperature);
            temp_measurement.flags = hts_unit_celsius_c;
            temp_measurement.flags += hts_include_temp_type;
            temp_measurement.type = temp_type;
            sys_put_le32(temperature, temp_measurement.temperature);

            hts_value.uuid_length = 2;
            hts_value.uuid[0] = (UUID_HTS_MEASUREMENT >> 0) & 0xFF;
            hts_value.uuid[1] = (UUID_HTS_MEASUREMENT >> 8) & 0xFF;
            hts_value.len = sizeof(temp_measurement);
            memcpy(&hts_value.value, (uint8_t *)&temp_measurement, sizeof(temp_measurement));

            //send Set value command
            write_charateristic_command_local(&hts_value);
            ncp_host_send_tlv_command();

            temperature++;
            if (temperature == 25U)
            {
                temperature = 20U;
            }
            temp_type++;
            if (temp_type > hts_tympanum)
            {
                temp_type = hts_no_temp_type;
            }
            peripheral_hts_event_get(HTS_EVENT_WRITE_CHRA_RSP);
        }
        OSA_TimeDelay(1000);
    }
}

void hts_init(void)
{
    (void)OSA_EventCreate(hts_events, 1U);
    (void)OSA_TaskCreate((osa_task_handle_t)hts_service_thread, OSA_TASK(peripheral_hts_task), (osa_task_param_t)NULL);
}
#endif
