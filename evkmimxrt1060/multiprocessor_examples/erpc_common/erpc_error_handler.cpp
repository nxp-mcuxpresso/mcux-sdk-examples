/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_error_handler.h"
#include "fsl_debug_console.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

extern bool g_erpc_error_occurred;
bool g_erpc_error_occurred = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void erpc_error_handler(erpc_status_t err, uint32_t functionID)
{
    switch (err)
    {
        case kErpcStatus_Fail:
            PRINTF("\r\nGeneric failure.");
            break;

        case kErpcStatus_InvalidArgument:
            PRINTF("\r\nArgument is an invalid value.");
            break;

        case kErpcStatus_Timeout:
            PRINTF("\r\nOperated timed out.");
            break;

        case kErpcStatus_InvalidMessageVersion:
            PRINTF("\r\nMessage header contains an unknown version.");
            break;

        case kErpcStatus_ExpectedReply:
            PRINTF("\r\nExpected a reply message but got another message type.");
            break;

        case kErpcStatus_CrcCheckFailed:
            PRINTF("\r\nMessage is corrupted.");
            break;

        case kErpcStatus_BufferOverrun:
            PRINTF("\r\nAttempt to read or write past the end of a buffer.");
            break;

        case kErpcStatus_UnknownName:
            PRINTF("\r\nCould not find host with given name.");
            break;

        case kErpcStatus_ConnectionFailure:
            PRINTF("\r\nFailed to connect to host.");
            break;

        case kErpcStatus_ConnectionClosed:
            PRINTF("\r\nConnected closed by peer.");
            break;

        case kErpcStatus_MemoryError:
            PRINTF("\r\nMemory allocation error.");
            break;

        case kErpcStatus_ServerIsDown:
            PRINTF("\r\nServer is stopped.");
            break;

        case kErpcStatus_InitFailed:
            PRINTF("\r\nTransport layer initialization failed.");
            break;

        case kErpcStatus_ReceiveFailed:
            PRINTF("\r\nFailed to receive data.");
            break;

        case kErpcStatus_SendFailed:
            PRINTF("\r\nFailed to send data.");
            break;

        /* no error occurred */
        case kErpcStatus_Success:
            return;

        /* unhandled error */
        default:
            PRINTF("\r\nUnhandled error occurred.");
            break;
    }

    /* When error occurred on client side. */
    if (functionID != 0)
    {
        PRINTF("Function id '%u'.", (unsigned int)functionID);
    }
    PRINTF("\r\n");

    /* error occurred */
    g_erpc_error_occurred = true;
}
