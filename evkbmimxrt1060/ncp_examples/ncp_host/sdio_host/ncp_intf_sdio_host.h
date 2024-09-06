/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#if CONFIG_NCP_SDIO
#ifndef _NCP_INTF_SDIO_H_
#define _NCP_INTF_SDIO_H_

#include "ncp_tlv_adapter.h"

/*******************************************************************************
 * Public macro
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/** NCP SDIO host initialization
 *
 * \return 0 on success
 * \return 1 on failure
 */
int ncp_sdhost_init(void *argv);

/** Send data/cmd by NCP SDIO host
 *
 * \param[in] Pointer to tlv_buf.
 * \param[in] Length of tlv.
 * \return 0 on success
 * \return 1 on failure
 */
int ncp_sdhost_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb);

/** NCP SDIO host deinitialization
 *
 * \return 0 on success
 * \return 1 on failure
 */
int ncp_sdhost_deinit(void *argv);

/*! @} */

#endif /* _NCP_INTF_SDIO_H_ */
#endif /* CONFIG_NCP_SDIO */