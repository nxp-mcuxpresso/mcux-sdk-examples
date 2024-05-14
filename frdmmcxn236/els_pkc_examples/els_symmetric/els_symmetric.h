/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ELS_PKC_SYMMETRIC_H_
#define _ELS_PKC_SYMMETRIC_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXIT_CODE_ERROR 0x00U
#define EXIT_CODE_OK    0x01U
/*******************************************************************************
 * API
 ******************************************************************************/
extern bool mcuxClEls_Cipher_Aes128_Cbc_Encrypt_example(void);
extern bool mcuxClEls_Cipher_Aes128_Ecb_Encrypt_example(void);
extern bool mcuxClMacModes_cmac_oneshot_example(void);

#endif /* _ELS_PKC_SYMMETRIC_H_ */
