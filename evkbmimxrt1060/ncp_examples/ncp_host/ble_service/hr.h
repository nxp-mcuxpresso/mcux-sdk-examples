/** @file ht.h
 *
 *  @brief  Health Thermometer Profile defineations.
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __HR_H_
#define __HR_H_

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Definitions
******************************************************************************/
#define HRC_EVENT_DEVICE_FOUND                  0x01
#define HRC_EVENT_CONNECTED                     0x02
#define HRC_EVENT_GET_PRIMARY_SERVICE           0x03
#define HRC_EVENT_GET_CHARACTERISTICS           0x04
#define HRC_EVENT_GET_CCC                       0x05

#define HRS_EVENT_WRITE_CHRA_RSP                0x01


/**
 *  @brief Heart Rate Service UUID value
 */
#define UUID_HRS 0x180d
/**
 *  @brief HRS Characteristic Measurement Interval UUID value
 */
#define UUID_HRS_MEASUREMENT 0x2a37
/**
 *  @brief HRS Characteristic Body Sensor Location
 */
#define UUID_HRS_BODY_SENSOR 0x2a38
/**
 *  @brief HRS Characteristic Control Point UUID value
 */
#define UUID_HRS_CONTROL_POINT 0x2a39


/* HTS flag values */
#define hrs_unit_celsius_c        0x00U /* bit 0 unset */
#define hrs_unit_fahrenheit_c     0x01U /* bit 0 set */

#define hrs_include_temp_type     0x04U /* bit 2 set */


/*******************************************************************************
* Prototypes
******************************************************************************/


/*******************************************************************************
 * API
 ******************************************************************************/
void central_hrc_start(void);
void hrc_init(void);
#if 0
void central_htc_event_put(osa_event_flags_t flag);
void central_htc_found(NCP_DEVICE_ADV_REPORT_EV * data);
void central_htc_get_primary_service(NCP_DISC_PRIM_RP * param);
void central_htc_get_characteristics(NCP_DISC_CHRC_RP * param);
void central_htc_get_ccc(NCP_DISC_ALL_DESC_RP * param);
void central_notify(uint8_t *data);
#endif

void hrs_init(void);
void peripheral_hrs_event_put(osa_event_flags_t flag);
void peripheral_hrs_start(void);
void peripheral_hrs_indicate(uint8_t value);



#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __HR_H_ */
