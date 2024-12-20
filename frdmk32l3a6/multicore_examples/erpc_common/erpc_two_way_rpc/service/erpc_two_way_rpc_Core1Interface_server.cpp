/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Generated by erpcgen 1.11.0 on Fri Apr 14 15:29:25 2023.
 *
 * AUTOGENERATED - DO NOT EDIT
 */


#include "erpc_two_way_rpc_Core1Interface_server.h"
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
#include <new>
#include "erpc_port.h"
#endif
#include "erpc_manually_constructed.hpp"

#if 11100 != ERPC_VERSION_NUMBER
#error "The generated shim code version is different to the rest of eRPC code."
#endif

using namespace erpc;
using namespace std;

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
#endif

ERPC_MANUALLY_CONSTRUCTED_STATIC(Core1Interface_service, s_Core1Interface_service);


static const getNumberCallback_t _getNumberCallback_t[] = { getNumberFromCore1, getNumberFromCore0 };


// Call the correct server shim based on method unique ID.
erpc_status_t Core1Interface_service::handleInvocation(uint32_t methodId, uint32_t sequence, Codec * codec, MessageBufferFactory *messageFactory)
{
    erpc_status_t erpcStatus;
    switch (methodId)
    {
        case kCore1Interface_increaseNumber_id:
        {
            erpcStatus = increaseNumber_shim(codec, messageFactory, sequence);
            break;
        }

        case kCore1Interface_getGetCallbackFunction_id:
        {
            erpcStatus = getGetCallbackFunction_shim(codec, messageFactory, sequence);
            break;
        }

        case kCore1Interface_getNumberFromCore0_id:
        {
            erpcStatus = getNumberFromCore0_shim(codec, messageFactory, sequence);
            break;
        }

        default:
        {
            erpcStatus = kErpcStatus_InvalidArgument;
            break;
        }
    }

    return erpcStatus;
}

// Server shim for increaseNumber of Core1Interface interface.
erpc_status_t Core1Interface_service::increaseNumber_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    uint32_t number;

    // startReadMessage() was already called before this shim was invoked.

    codec->read(number);

    err = codec->getStatus();
    if (err == kErpcStatus_Success)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        increaseNumber(&number);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (err == kErpcStatus_Success)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kCore1Interface_service_id, kCore1Interface_increaseNumber_id, sequence);

        codec->write(number);

        err = codec->getStatus();
    }

    return err;
}

// Server shim for getGetCallbackFunction of Core1Interface interface.
erpc_status_t Core1Interface_service::getGetCallbackFunction_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    getNumberCallback_t *getNumberCallbackParam = NULL;

    // startReadMessage() was already called before this shim was invoked.

    getNumberCallbackParam = (getNumberCallback_t *) erpc_malloc(sizeof(getNumberCallback_t));
    if (getNumberCallbackParam == NULL)
    {
        codec->updateStatus(kErpcStatus_MemoryError);
    }

    err = codec->getStatus();
    if (err == kErpcStatus_Success)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        getGetCallbackFunction(getNumberCallbackParam);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (err == kErpcStatus_Success)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kCore1Interface_service_id, kCore1Interface_getGetCallbackFunction_id, sequence);

        codec->writeCallback((arrayOfFunPtr)(_getNumberCallback_t), 2, (funPtr)(*getNumberCallbackParam));

        err = codec->getStatus();
    }

    erpc_free(getNumberCallbackParam);

    return err;
}

// Server shim for getNumberFromCore0 of Core1Interface interface.
erpc_status_t Core1Interface_service::getNumberFromCore0_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    uint32_t number;

    // startReadMessage() was already called before this shim was invoked.

    err = codec->getStatus();
    if (err == kErpcStatus_Success)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        getNumberFromCore0(&number);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (err == kErpcStatus_Success)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kCore1Interface_service_id, kCore1Interface_getNumberFromCore0_id, sequence);

        codec->write(number);

        err = codec->getStatus();
    }

    return err;
}

erpc_service_t create_Core1Interface_service(void)
{
    erpc_service_t service;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    service = new (nothrow) Core1Interface_service();
#else
    if (s_Core1Interface_service.isUsed())
    {
        service = NULL;
    }
    else
    {
        s_Core1Interface_service.construct();
        service = s_Core1Interface_service.get();
    }
#endif

    return service;
}

void destroy_Core1Interface_service(erpc_service_t service)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(service != NULL);
    delete (Core1Interface_service *)service;
#else
    (void)service;
    erpc_assert(service == s_Core1Interface_service.get());
    s_Core1Interface_service.destroy();
#endif
}

