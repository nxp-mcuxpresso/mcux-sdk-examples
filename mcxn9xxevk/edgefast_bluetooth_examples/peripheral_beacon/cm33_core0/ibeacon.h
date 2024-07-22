/* ibeacon.h - ibeacon sample */

/*
 * Copyright (c) 2018 Henrik Brix Andersen <henrik@brixandersen.dk>
 * Copyright 2022 NXP
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(IBEACON_APP) && (IBEACON_APP == 1)

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
void beacon_task(void *pvParameters);
#ifdef __cplusplus
}
#endif

#endif /* IBEACON_APP */