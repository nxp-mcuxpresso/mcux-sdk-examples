/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "EmbeddedTypes.h"
#include "fsl_os_abstraction.h"
#include "Panic.h"
#include "MemManager.h"
#include "FunctionLib.h"

/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
 * \brief     Allocate a block from the memory pools. The function uses the
 *            numBytes argument to look up a pool with adequate block sizes.
 * \param[in] numBytes - Size of buffer to allocate.
 * \param[in] poolId - The ID of the pool where to search for a free buffer.
 * \param[in] pCaller - pointer to the caller function (Debug purpose)
 *
 * \return Pointer to the allocated buffer, NULL if failed.
 *
 * \pre Memory manager must be previously initialized.
 *
 ********************************************************************************** */
void *MEM_BufferAlloc(uint32_t numBytes)
{
    listHeader_t *pBlock;

    pBlock = (listHeader_t *)OSA_Malloc(sizeof(listHeader_t) + numBytes);

    return pBlock ? pBlock + 1 : NULL;
}

/*! *********************************************************************************
 * \brief     Deallocate a memory block by putting it in the corresponding pool
 *            of free blocks.
 *
 * \param[in] buffer - Pointer to buffer to deallocate.
 *
 * \pre Memory manager must be previously initialized.
 *
 * \remarks Never deallocate the same buffer twice.
 *
 ********************************************************************************** */
void MEM_BufferFree(void *buffer /* IN: Block of memory to free*/
)
{
    listHeader_t *pHeader;

    if (buffer == NULL)
    {
        return;
    }

    pHeader = (listHeader_t *)buffer - 1;

    OSA_Free(pHeader);
}
