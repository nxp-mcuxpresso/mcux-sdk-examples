/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 * 
 */

/*!\file ht.h
 * \brief Health Thermometer Profile definitions.
 */

#ifndef __HT_H_
#define __HT_H_

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Definitions
******************************************************************************/
/** HTC device found event ID */
#define HTC_EVENT_DEVICE_FOUND                  0x01
/** HTC connected event ID */
#define HTC_EVENT_CONNECTED                     0x02
/** HTC get primary service event ID */
#define HTC_EVENT_GET_PRIMARY_SERVICE           0x03
/** HTC get characteristic ID */
#define HTC_EVENT_GET_CHARACTERISTICS           0x04
/** HTC get characteristic configuration changed event ID */
#define HTC_EVENT_GET_CCC                       0x05
/** HTC write characteristic response event ID */
#define HTS_EVENT_WRITE_CHRA_RSP                0x01

/** GATT Primary Service UUID */
#define UUID_GATT_PRIMARY 0x2800

/** Health Thermometer Service UUID */
#define UUID_HTS 0x1809

/** HTS Characteristic Measurement Value UUID */
#define UUID_HTS_MEASUREMENT 0x2a1c

/** GATT Client Characteristic Configuration UUID */
#define UUID_GATT_CCC 0x2902

/** Client Characteristic Configuration Values */


/** Client Characteristic Configuration Notification.
 * If set, changes to Characteristic Value are notified.
 */
#define BT_GATT_CCC_NOTIFY			0x0001
/** Client Characteristic Configuration Indication.
 * If set, changes to Characteristic Value are indicated.
 */
#define BT_GATT_CCC_INDICATE			0x0002

/** HTS flag values */
#define hts_unit_celsius_c        0x00U /* bit 0 unset */
#define hts_unit_fahrenheit_c     0x01U /* bit 0 set */

#define hts_include_temp_type     0x04U /* bit 2 set */


/** Temperature measurement format */
struct temp_measurement
{
    /** temperature type flag 
     * 0: Fahrenheit
     * 1: Celsius
    */
    uint8_t flags;
    /** temperature */
    uint8_t temperature[4];
    /** Possible temperature sensor locations
     * 0: hts_no_temp_type
     * 1: hts_armpit
     * 2: hts_body
     * 3: hts_ear
     * 4: hts_finger
     * 5: hts_gastroInt
     * 6: hts_mouth
     * 7: hts_rectum
     * 8: hts_toe
     * 9: hts_tympanum
    */
    uint8_t type;
};

/** Heart Rate format */
struct hr_measurement
{
    /** sensor id */
    uint8_t sensor;
    /** heart rate */
    uint8_t rate;
};

/*******************************************************************************
* Prototypes
******************************************************************************/
/** Possible temperature sensor locations */
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
 * API
 ******************************************************************************/
/**
 * Start central HTC (Health Thermometer) service
 * 
 * \return void
 */
void central_htc_start(void);
/**
 * Init HTC Service
 * 
 * \return void
 */
void htc_init(void);
#if 0
void central_htc_event_put(osa_event_flags_t flag);
void central_htc_found(NCP_DEVICE_ADV_REPORT_EV * data);
void central_htc_get_primary_service(NCP_DISC_PRIM_RP * param);
void central_htc_get_characteristics(NCP_DISC_CHRC_RP * param);
void central_htc_get_ccc(NCP_DISC_ALL_DESC_RP * param);
void central_notify(uint8_t *data);
#endif

/**
 * Init HTS Service
 * 
 * \return void
 */
void hts_init(void);
/**
 * Count binary semaphore to wait for HTS write characteristic response event
 * 
 * \param[in] flag flag to wait
 * 
 * \return void
 */
void peripheral_hts_event_put(osa_event_flags_t flag);
/**
 * Start Peripheral HTS Service
 * 
 * \return void
 */
void peripheral_hts_start(void);
/**
 * Indicate HTS characteristic value change event 
 * 
 * \param[in] value hts value
 * 
 * \return void
 */
void peripheral_hts_indicate(uint8_t value);



#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __HT_H_ */
