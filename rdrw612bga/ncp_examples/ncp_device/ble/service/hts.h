/*
 * Copyright (c) 2019 Aaron Tsui <aaron.tsui@outlook.com>
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_HTS_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_HTS_H_
/**
 * @brief Health Thermometer Service (HTS)
 * @defgroup bt_hts Health Thermometer Service (HTS)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Definitions
******************************************************************************/
/* HTS flag values */
#define hts_unit_celsius_c        0x00U /* bit 0 unset */
#define hts_unit_fahrenheit_c     0x01U /* bit 0 set */

#define hts_include_temp_type     0x04U /* bit 2 set */


/* Temperature measurement format */
struct temp_measurement
{
    uint8_t flags;
    uint8_t temperature[4];
    uint8_t type;
};

/* Possible temperature sensor locations */
enum
{
    hts_no_temp_type = 0x00U,
    hts_armpit       = 0x01U,
    hts_body         = 0x02U,
    hts_ear          = 0x03U,
    hts_finger       = 0x04U,
    hts_gastroInt    = 0x05U,
    hts_mouth        = 0x06U,
    hts_rectum       = 0x07U,
    hts_toe          = 0x08U,
    hts_tympanum     = 0x09U,
};

/*******************************************************************************
* Prototypes
******************************************************************************/
void peripheral_hts_task(void *args);

void peripheral_hts_connect(void *args);

void peripheral_hts_disconnect(void *args);

void init_hts_service(void);




#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_HTS_H_ */
