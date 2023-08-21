/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ELS_HASH_H_
#define _ELS_HASH_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXIT_CODE_ERROR 0x00U
#define EXIT_CODE_OK    0x01U
/*******************************************************************************
 * API
 ******************************************************************************/
extern bool mcuxClHash_sha224_oneshot_example(void);
extern bool mcuxClHash_sha256_oneshot_example(void);
extern bool mcuxClHash_sha256_longMsgOneshot_example(void);
extern bool mcuxClHash_sha256_streaming_example(void);
extern bool mcuxClHash_sha384_oneshot_example(void);
extern bool mcuxClHash_sha512_oneshot_example(void);

#endif /* _ELS_HASH_H_ */
