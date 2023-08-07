/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>

#include "fsl_common.h"
#include "mflash_drv.h"
#include "mcuboot_app_support.h"
#include "nx_azure_iot_adu_agent.h"
#include "fsl_debug_console.h"


static uint32_t firmware_size;

static uint32_t update_part_offset;
static uint32_t update_part_size;
static uint32_t received_data;
static uint32_t saved_data;

static uint8_t buffer[MFLASH_PAGE_SIZE];       /* one flash page */
static uint8_t tmp_buf[MFLASH_PAGE_SIZE];
static uint32_t buf_level;

static status_t _write_image(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr)
{
    uint32_t offset = (uint32_t)driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_data_offset;
    uint8_t *data_ptr = (uint8_t *)driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_data_ptr;
    int data_remaining = (int)driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_data_size;
    uint32_t flash_device_addr;
    int buf_space;
    status_t status;

    if (offset != saved_data + buf_level)
    {
        PRINTF("ERR: data offset is not right, 0x%x (0x%x + %x0x)\r\n",
                offset, saved_data, buf_level);
        return kStatus_Fail;
    }

    received_data += data_remaining;

    while (data_remaining > 0)
    {
        buf_space = sizeof(buffer) - buf_level;

        if (data_remaining >= buf_space)
        {
            memcpy(buffer + buf_level, data_ptr, buf_space);
            data_ptr += buf_space;
            data_remaining -= buf_space;
            buf_level += buf_space;
        }
        else
        {
            memcpy(buffer + buf_level, data_ptr, data_remaining);
            buf_level += data_remaining;
            data_ptr += data_remaining;
            data_remaining = 0;

            if (received_data == firmware_size)
            {

                /* the final data block */
                buf_space = sizeof(buffer) - buf_level;
                memset(buffer + buf_level, 0x0ff, buf_space);
                buf_level += buf_space;
            }
        }

        buf_space = sizeof(buffer) - buf_level;
        if (buf_space != 0)
        {
            break;
        }

        flash_device_addr = update_part_offset + saved_data;

        status = (status_t)mflash_drv_page_program(flash_device_addr, (uint32_t *)buffer);
        if (status != kStatus_Success)
        {
            PRINTF("ERR: mflash_drv_page_program(0x%x)\r\n", flash_device_addr);
            return status;
        }
        saved_data += sizeof(buffer);
        buf_level = 0;

        /* verify */
        status = (status_t)mflash_drv_read(flash_device_addr, (uint32_t *)tmp_buf, MFLASH_PAGE_SIZE);
        if (status != kStatus_Success)
        {
            PRINTF("ERR: mflash_drv_read(0x%x)\r\n", flash_device_addr);
            return status;
        }

        if (0 != memcmp(tmp_buf, buffer, MFLASH_PAGE_SIZE))
        {
            PRINTF("ERR: image verification failed\r\n", flash_device_addr);
            return kStatus_Fail;
        }
    }

    return kStatus_Success;
}

#define IMAGE_INDEX     0

void nx_azure_iot_adu_agent_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr)
{
    status_t status;

    /* Default to successful return.  */
    driver_req_ptr->nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;

    /* Process according to the driver request type.  */
    switch (driver_req_ptr->nx_azure_iot_adu_agent_driver_command)
    {

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE:
        {
            status = bl_update_image_state(0, kSwapType_Permanent);
            if (status != kStatus_Success && status != kStatus_NoData)
            {
                PRINTF("ERR: Failed to mark the new image as permanent (ret=%d)\r\n", status);
                driver_req_ptr -> nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_FAILURE;
            }

            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_PREPROCESS:
        {
            /* Process firmware preprocess requests before writing firmware.
               Such as: erase the flash at once to improve the speed.  */

            partition_t update_part;

            if (bl_get_update_partition_info(IMAGE_INDEX, &update_part) != kStatus_Success)
            {
                PRINTF("ERR: bl_get_update_partition_info()\r\n");
                driver_req_ptr -> nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_FAILURE;
            }

            update_part_offset = update_part.start;
            update_part_size = update_part.size;

            firmware_size = driver_req_ptr->nx_azure_iot_adu_agent_driver_firmware_size;

            PRINTF("firmware size: 0x%x\r\n", firmware_size);

            received_data = 0;
            saved_data = 0;

            /* no data in the buffer */
            buf_level = 0;

            PRINTF("Erase the update partition (start: 0x%x, size: 0x%x)\r\n",
                    update_part_offset, update_part_size);

            for (uint32_t offset = update_part_offset;
                 offset < update_part_offset + update_part_size;
                 offset += MFLASH_SECTOR_SIZE)
            {
                status = mflash_drv_sector_erase(offset);
                if (status != kStatus_Success)
                {
                    PRINTF("ERR: mflash_drv_sector_erase()\r\n");
                    driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
                    break;
                }
            }

            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_WRITE:
        {
            /* Process firmware write requests.  */

            status = _write_image(driver_req_ptr);
            if (status != kStatus_Success)
            {
                driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
                break;
            }

            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_INSTALL:
        {
            /* Set the new firmware for next boot.  */

            /* verify the new image */
            status = bl_verify_image(update_part_offset, firmware_size);
            if (status != 1)
            {
                PRINTF("ERR: bl_verify_image()\r\n");
                driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
                break;
            }

            /* set the flag to make the new image in effect */
            status = bl_update_image_state(0, kSwapType_ReadyForTest);
            if (status != kStatus_Success)
            {
                PRINTF("ERR: bl_update_image_state()\r\n");
                driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
                break;
            }

            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_APPLY:
        {

            break;
        }

        default:
        {
            /* Invalid driver request.  */
            driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
        }
    }
}
