/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_DEFINITIONS_H_
#define _APP_DEFINITIONS_H_

/*${header:start}*/
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define DEMO_I2C                        (I2C4)
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY (24576000)
#define DEMO_I2S_TX                     (I2S7)
#define DEMO_DMA                        (DMA0)
#define DEMO_I2S_TX_CHANNEL             (19)
#define DEMO_I2S_TX_MODE                (kI2S_MasterSlaveNormalMaster)
#define DEMO_CHANNEL_NUM                (2)
/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
