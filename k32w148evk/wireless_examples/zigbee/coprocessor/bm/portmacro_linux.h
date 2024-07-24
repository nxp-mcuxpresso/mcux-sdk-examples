/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PORTMACRO_LINUX_H__
#define __PORTMACRO_LINUX_H__

PUBLIC uint8 ZPS_eEnterCriticalSection(void* hMutex, uint32* psIntStore);
PUBLIC uint8 ZPS_eExitCriticalSection(void* hMutex, uint32* psIntStore);


#endif /* __PORTMACRO_LINU_H__ */
