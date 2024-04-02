/*! *********************************************************************************
* \defgroup GATT_DB
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2014 Freescale Semiconductor, Inc.
* Copyright 2016-2019, 2021, 2023 NXP
*
*
* \file
*
* This file is the interface file for the creation of a Dynamic Gatt Database
* in application.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef DYNAMIC_GATT_DATABASE_H
#define DYNAMIC_GATT_DATABASE_H


#define service_gatt                       1
#define char_service_changed               2
#define value_service_changed              3
#define cccd_service_changed               4

#define service_gap                        6
#define char_device_name                   7
#define value_device_name                  8
#define char_appearance                    9
#define value_appearance                  10

#define service_throughput                12
#define char_throughput_stream            13
#define value_throughput_stream           14

#define service_battery                   16
#define char_battery_level                17
#define value_battery_level               18
#define desc_bat_level                    19
#define cccd_battery_level                20

#define service_device_info               22
#define char_manuf_name                   23
#define value_manuf_name                  24
#define char_model_no                     25
#define value_model_no                    26
#define char_serial_no                    27
#define value_serial_no                   28
#define char_hw_rev                       29
#define value_hw_rev                      30
#define char_fw_rev                       31
#define value_fw_rev                      32
#define char_sw_rev                       33
#define value_sw_rev                      34


#ifdef __cplusplus
extern "C" {
#endif

bleResult_t GattDbDynamic_CreateDatabase(void);

#ifdef __cplusplus
}
#endif


#endif /* DYNAMIC_GATT_DATABASE_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
