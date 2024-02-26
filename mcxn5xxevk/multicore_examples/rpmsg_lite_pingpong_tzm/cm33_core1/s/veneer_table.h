/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VENEER_TABLE_H_
#define _VENEER_TABLE_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_STRING_LENGTH 0x400

/*! \typedef rl_ept_rx_cb_ns_t
    \brief Non-secure receive callback function type.
*/
#if (__ARM_FEATURE_CMSE & 0x2)
#if defined(__IAR_SYSTEMS_ICC__)
typedef __cmse_nonsecure_call int32_t (*rl_ept_rx_cb_ns_t)(void *payload,
                                                           int32_t payload_len,
                                                           uint32_t src,
                                                           void *priv);
#else
typedef int32_t (*rl_ept_rx_cb_ns_t)(void *payload, int32_t payload_len, uint32_t src, void *priv)
    __attribute__((cmse_nonsecure_call));
#endif
#else
typedef int32_t (*rl_ept_rx_cb_ns_t)(void *payload, int32_t payload_len, uint32_t src, void *priv);
#endif
/*!
 * Structure describing endpoint callback data
 */
struct rpmsg_lite_endpoint_callback_data_descr_ns
{
    void *app_data;   /*!< non-secure ISR callback data */
    void *app_buffer; /*!< non-secure application buffer to copy data into */
};

/*!
 * Structure describing endpoint callback properties
 */
struct rpmsg_lite_endpoint_callback_descr_ns
{
    rl_ept_rx_cb_ns_t rx_cb;                                       /*!< non-secure ISR callback function */
    struct rpmsg_lite_endpoint_callback_data_descr_ns *rx_cb_data; /*!< non-secure ISR callback data structure */
};

/*!
 * Structure describing parameters of a message to be sent
 */
struct rpmsg_lite_send_params_ns
{
    uint32_t dst;  /*!< Remote endpoint address */
    char *data;    /*!< Payload buffer pointer */
    uint32_t size; /*!< Size of payload, in bytes */
};

/*******************************************************************************
 * API
 ******************************************************************************/

/* Exported API functions */

/*!
 * @brief Entry function for creating a new rpmsg ns endpoint.
 * NS endpoint callback function pointer and data are passed via the
 * rpmsg_lite_endpoint_callback_descr_ns structure.
 *
 * This function provides interface between secure and normal worlds
 * This function is called from normal world only
 *
 * @param addr               Desired address, RL_ADDR_ANY for automatic selection
 * @param ept_callback_descr Pointer to the rpmsg_lite_endpoint_callback_descr_ns struct
 *
 * @return RL_NULL on error, new endpoint pointer on success.
 *
 */
struct rpmsg_lite_endpoint *rpmsg_lite_create_ept_nse(uint32_t addr,
                                                      struct rpmsg_lite_endpoint_callback_descr_ns *ept_callback_descr);

/*!
 * @brief Entry function for deleting rpmsg ns endpoint and performing cleanup.
 *
 * This function provides interface between secure and normal worlds
 * This function is called from normal world only
 *
 * @param rl_ept            Pointer to endpoint to destroy
 *
 */
int32_t rpmsg_lite_destroy_ept_nse(struct rpmsg_lite_endpoint *rl_ept);

/*!
 *
 * @brief Entry function for sending a message to the remote endpoint.
 * Due to the limited number of passing function parameters the remote endpoint
 * address, payload buffer pointer and the payload size is passed via the
 * rpmsg_lite_send_params_ns structure.
 *
 * This function provides interface between secure and normal worlds
 * This function is called from normal world only
 *
 * @param ept               Sender endpoint
 * @param message_params    Pointer to the rpmsg_lite_send_params_ns struct
 * @param timeout           Timeout in ms, 0 if nonblocking
 *
 * @return Status of function execution, RL_SUCCESS on success.
 *
 */
int32_t rpmsg_lite_send_nse(struct rpmsg_lite_endpoint *ept,
                            struct rpmsg_lite_send_params_ns *message_params,
                            uint32_t timeout);

/*!
 * @brief Entry function for debug PRINTF (DbgConsole_Printf)
 *
 * This function provides interface between secure and normal worlds
 * This function is called from normal world only
 *
 * @param s     String to be printed
 *
 */
void DbgConsole_Printf_NSE(char const *s);

#if defined(__cplusplus)
}
#endif

#endif /* _VENEER_TABLE_H_ */
