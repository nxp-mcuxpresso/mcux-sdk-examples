/*!\file bas.h
 *\brief Battery Service Profile definitions.
 */

/*
 *  Copyright 2024 NXP
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BAS_H_
#define __BAS_H_

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Definitions
******************************************************************************/
/** BAS write characteristic response event ID */
#define BAS_EVENT_WRITE_CHRA_RSP                0x01

/** Battery Service UUID value */
#define UUID_BAS 0x180f

/** BAS Characteristic Battery Level UUID value  */
#define UUID_BAS_BATTERY_LEVEL 0x2a19


/*******************************************************************************
* Prototypes
******************************************************************************/


/*******************************************************************************
 * API
 ******************************************************************************/
/**
 * Init BAS Service
 * 
 * \return void
 */
void bas_init(void);
/**
 * Count binary semaphore to wait for BAS write characteristic response event
 * 
 * \param[in] flag flag to wait
 * 
 * \return void
 */
void peripheral_bas_event_put(osa_event_flags_t flag);
/**
 * Start peripheral BAS Service
 * 
 * \return void
 */
void peripheral_bas_start(void);
/**
 * Indicate BAS characteristic value change event 
 * 
 * \param[in] value bas value
 * 
 * \return void
 */
void peripheral_bas_indicate(uint8_t value);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __BAS_H_ */
