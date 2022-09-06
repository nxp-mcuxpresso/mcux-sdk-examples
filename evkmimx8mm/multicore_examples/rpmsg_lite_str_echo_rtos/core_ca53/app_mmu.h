/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_MMU_H_
#define _APP_MMU_H_

#ifndef KB
#define KB(x)                  ((x) << 10)
#endif

#ifndef MB
#define MB(x)                  ((x) << 20)
#endif

#ifndef GB
#define GB(x)                  ((x) << 30)
#endif

#define APP_MMU_ENTRIES		\
	MMU_REGION_FLAT_ENTRY("SGIMBOX",				\
			      0xb8500000, KB(4),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("RPMSG",					\
			      0xb8600000, KB(64),			\
			      MT_DEVICE_nGnRE | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("RSCTABLE",				\
			      0xb86ff000, KB(4),			\
			      MT_NORMAL_NC | MT_P_RW_U_RW | MT_NS),	\
	MMU_REGION_FLAT_ENTRY("VRINGBUF",				\
			      0xb8700000, MB(1),			\
			      MT_NORMAL_NC | MT_P_RW_U_RW | MT_NS),

#endif /* _APP_MMU_H_ */
