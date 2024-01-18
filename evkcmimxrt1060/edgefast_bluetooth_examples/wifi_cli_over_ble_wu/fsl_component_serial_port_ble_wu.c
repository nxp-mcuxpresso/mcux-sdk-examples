/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_component_serial_manager.h"
#include "fsl_component_serial_port_internal.h"

#if (defined(SERIAL_PORT_TYPE_BLE_WU) && (SERIAL_PORT_TYPE_BLE_WU > 0U))

#include "fsl_component_serial_port_ble_wu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef NDEBUG
#if (defined(DEBUG_CONSOLE_ASSERT_DISABLE) && (DEBUG_CONSOLE_ASSERT_DISABLE > 0U))
#undef assert
#define assert(n)
#else
/* MISRA C-2012 Rule 17.2 */
#undef assert
#define assert(n) \
    while (!(n))  \
    {             \
        ;         \
    }
#endif
#endif

/* Weak function. */
#if defined(__GNUC__)
#define __WEAK_FUNC __attribute__((weak))
#elif defined(__ICCARM__)
#define __WEAK_FUNC __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __WEAK_FUNC __attribute__((weak))
#endif

typedef struct _serial_port_ble_wu_send_state
{
    serial_manager_callback_t callback;
    void *callbackParam;
} serial_port_ble_wu_send_state_t;

typedef struct _serial_port_ble_wu_recv_state
{
    serial_manager_callback_t callback;
    void *callbackParam;
} serial_port_ble_wu_recv_state_t;

/* Define the types for application */
typedef struct _serial_port_ble_wu_state
{
    struct _serial_port_ble_wu_state *next; /* Next pointer of the port */
    serial_port_ble_wu_send_state_t tx;
    serial_port_ble_wu_recv_state_t rx;
} serial_port_ble_wu_state_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static serial_port_ble_wu_state_t *s_serialPortBleWuHead;

/*******************************************************************************
 * Code
 ******************************************************************************/

static serial_manager_status_t Serial_PortBleWuAddItem(serial_port_ble_wu_state_t **head,
                                                       serial_port_ble_wu_state_t *node)
{
    serial_port_ble_wu_state_t *p = *head;
    uint32_t regPrimask;

    regPrimask = DisableGlobalIRQ();

    if (NULL == p)
    {
        *head = node;
    }
    else
    {
        while (NULL != p->next)
        {
            if (p == node)
            {
                EnableGlobalIRQ(regPrimask);
                return kStatus_SerialManager_Error;
            }
            p = p->next;
        }

        p->next = node;
    }
    node->next = NULL;
    EnableGlobalIRQ(regPrimask);
    return kStatus_SerialManager_Success;
}

static serial_manager_status_t Serial_PortBleWuRemoveItem(serial_port_ble_wu_state_t **head,
                                                          serial_port_ble_wu_state_t *node)
{
    serial_port_ble_wu_state_t *p = *head;
    serial_port_ble_wu_state_t *q = NULL;
    uint32_t regPrimask;

    regPrimask = DisableGlobalIRQ();
    while (NULL != p)
    {
        if (p == node)
        {
            if (NULL == q)
            {
                *head = p->next;
            }
            else
            {
                q->next = p->next;
            }
            break;
        }
        else
        {
            q = p;
            p = p->next;
        }
    }
    EnableGlobalIRQ(regPrimask);
    return kStatus_SerialManager_Success;
}

static serial_manager_status_t Serial_PortBleWuGetInstance(serial_port_ble_wu_state_t **handle)
{
#if 0
    serial_port_ble_wu_state_t *p = s_serialPortBleWuHead;
    uint32_t regPrimask;

    *handle    = NULL;
    regPrimask = DisableGlobalIRQ();
    while (NULL != p)
    {
        if (p->connection == connection)
        {
            *handle = p;
            break;
        }
    }
    EnableGlobalIRQ(regPrimask);
    return (NULL != p) ? kStatus_SerialManager_Success : kStatus_SerialManager_Error;
#else
    *handle = s_serialPortBleWuHead;
    return kStatus_SerialManager_Success;
#endif
}

static void Serial_PortBleWuTxCallback(void *callbackParam,
                                       serial_manager_callback_message_t *message,
                                       serial_manager_status_t status)
{
    serial_port_ble_wu_state_t *serialPortBleWu;

    if (NULL != callbackParam)
    {
        serialPortBleWu = (serial_port_ble_wu_state_t *)callbackParam;
        if (NULL != serialPortBleWu->tx.callback)
        {
            serialPortBleWu->tx.callback(serialPortBleWu->tx.callbackParam, message, status);
        }
    }
}

static void Serial_PortBleWuRxCallback(void *callbackParam,
                                       serial_manager_callback_message_t *message,
                                       serial_manager_status_t status)
{
    serial_port_ble_wu_state_t *serialPortBleWu;

    if (NULL != callbackParam)
    {
        serialPortBleWu = (serial_port_ble_wu_state_t *)callbackParam;
        if (NULL != serialPortBleWu->rx.callback)
        {
            serialPortBleWu->rx.callback(serialPortBleWu->rx.callbackParam, message, status);
        }
    }
}

__WEAK_FUNC status_t BLE_WuInit(void);
__WEAK_FUNC status_t BLE_WuInit(void)
{
    return kStatus_Success;
}

__WEAK_FUNC status_t BLE_WuDeinit(void);
__WEAK_FUNC status_t BLE_WuDeinit(void)
{
    return kStatus_Success;
}

__WEAK_FUNC status_t BLE_WuWrite(uint8_t *buffer, uint32_t length);
__WEAK_FUNC status_t BLE_WuWrite(uint8_t *buffer, uint32_t length)
{
    serial_port_ble_wu_state_t *serialPortBleWu;
    serial_manager_callback_message_t msg;

    if (kStatus_SerialManager_Success != Serial_PortBleWuGetInstance(&serialPortBleWu))
    {
        return kStatus_Fail;
    }

    if ((NULL != serialPortBleWu->tx.callback))
    {
        msg.buffer = buffer;
        msg.length = length;
        serialPortBleWu->tx.callback(serialPortBleWu->tx.callbackParam, &msg, kStatus_SerialManager_Success);
    }
    return kStatus_Success;
}

__WEAK_FUNC status_t BLE_WuRead(uint8_t *buffer, uint32_t length);
__WEAK_FUNC status_t BLE_WuRead(uint8_t *buffer, uint32_t length)
{
    return kStatus_Fail;
}

__WEAK_FUNC status_t BLE_WuCancelWrite(void);
__WEAK_FUNC status_t BLE_WuCancelWrite(void)
{
    return kStatus_Success;
}

__WEAK_FUNC status_t BLE_WuInstallTxCallback(serial_manager_callback_t callback, void *callbackParam);
__WEAK_FUNC status_t BLE_WuInstallTxCallback(serial_manager_callback_t callback, void *callbackParam)
{
    return kStatus_Success;
}

__WEAK_FUNC status_t BLE_WuInstallRxCallback(serial_manager_callback_t callback, void *callbackParam);
__WEAK_FUNC status_t BLE_WuInstallRxCallback(serial_manager_callback_t callback, void *callbackParam)
{
    return kStatus_Success;
}

__WEAK_FUNC void BLE_WuIsrFunction(void);
__WEAK_FUNC void BLE_WuIsrFunction(void)
{
}

serial_manager_status_t Serial_PortBleWuInit(serial_handle_t serialHandle, void *config)
{
    serial_port_ble_wu_state_t *serialPortBleWu;
    serial_manager_status_t ret;

    assert(NULL != serialHandle);
    assert(SERIAL_PORT_BLE_WU_HANDLE_SIZE >= sizeof(serial_port_ble_wu_state_t));

    (void)config;

    serialPortBleWu = (serial_port_ble_wu_state_t *)serialHandle;

    if ((status_t)kStatus_Success != BLE_WuInit())
    {
        return kStatus_SerialManager_Error;
    }

    ret = Serial_PortBleWuAddItem(&s_serialPortBleWuHead, serialPortBleWu);

    if (kStatus_SerialManager_Success != ret)
    {
        return ret;
    }

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_PortBleWuDeinit(serial_handle_t serialHandle)
{
    serial_port_ble_wu_state_t *serialPortBleWu;

    assert(serialHandle);

    serialPortBleWu = (serial_port_ble_wu_state_t *)serialHandle;

    (void)BLE_WuDeinit();

    (void)Serial_PortBleWuRemoveItem(&s_serialPortBleWuHead, serialPortBleWu);

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_PortBleWuWrite(serial_handle_t serialHandle, uint8_t *buffer, uint32_t length)
{
    (void)serialHandle;

    if (kStatus_Success != BLE_WuWrite(buffer, length))
    {
        return kStatus_SerialManager_Error;
    }

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_PortBleWuRead(serial_handle_t serialHandle, uint8_t *buffer, uint32_t length)
{
    (void)serialHandle;

    if (kStatus_Success != BLE_WuRead(buffer, length))
    {
        return kStatus_SerialManager_Error;
    }

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_PortBleWuCancelWrite(serial_handle_t serialHandle)
{
    (void)serialHandle;

    if (kStatus_Success != BLE_WuCancelWrite())
    {
        return kStatus_SerialManager_Error;
    }

    return kStatus_SerialManager_Success;
}

serial_manager_status_t Serial_PortBleWuInstallTxCallback(serial_handle_t serialHandle,
                                                          serial_manager_callback_t callback,
                                                          void *callbackParam)
{
    serial_port_ble_wu_state_t *serialPortBleWu;

    assert(serialHandle);

    serialPortBleWu = (serial_port_ble_wu_state_t *)serialHandle;

    serialPortBleWu->tx.callback      = callback;
    serialPortBleWu->tx.callbackParam = callbackParam;

    return (serial_manager_status_t)BLE_WuInstallTxCallback(Serial_PortBleWuTxCallback, serialHandle);
}

serial_manager_status_t Serial_PortBleWuInstallRxCallback(serial_handle_t serialHandle,
                                                          serial_manager_callback_t callback,
                                                          void *callbackParam)
{
    serial_port_ble_wu_state_t *serialPortBleWu;

    assert(serialHandle);

    serialPortBleWu = (serial_port_ble_wu_state_t *)serialHandle;

    serialPortBleWu->rx.callback      = callback;
    serialPortBleWu->rx.callbackParam = callbackParam;

    return (serial_manager_status_t)BLE_WuInstallRxCallback(Serial_PortBleWuRxCallback, serialHandle);
}

void Serial_PortBleWuIsrFunction(serial_handle_t serialHandle)
{
    (void)serialHandle;

    BLE_WuIsrFunction();
}

#endif /* SERIAL_PORT_TYPE_BLE_WU */
