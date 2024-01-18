/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eiq_audio_worker.h"
#include "eiq_speaker.h"
#include "eiq_micro.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief AudioWorker instance. */
static EIQ_AudioWorker_t s_worker;

/*! @brief AudioWorker s_workerHandler. It can be called when data are ready.*/
static EIQ_IWorkerUpdater_t s_workerHandler;

/*!
 * @brief Starts data transfer.
 *
 * This function starts microphone and speaker audio transfer. It is not blocking function.
 */
static void start(void)
{
    s_worker.receiver->base.start();
    s_worker.sender->base.start();
}

/*!
 * @brief Checks if data is ready.
 *
 * This function returns ready flag.
 *
 * @return True in case data is ready
 */
static bool isReady(void)
{
    return s_worker.receiver->isReady();
}

/*!
 * @brief Notifies s_worker.
 *
 * This function notifies s_worker to start new data transfer.
 *
 */
static void notify(void)
{
    s_worker.receiver->base.notify();
}

/*!
 * @brief Gets rectangle dimensions.
 *
 * This function returns dimensions of the captured data.
 *
 * @return Dimensions of the captured data
 */
static Dims_t getResolution(void)
{
    return s_worker.receiver->base.getResolution();
}

/*!
 * @brief Gets extracted data from microphone.
 *
 * This function returns extracted data from microphone.
 *
 * @return extracted data from microphone
 */
static uint8_t* getData(void)
{
    return s_worker.receiver->getReadyBuff();
}

/*!
 * @brief Refreshes data in the speaker.
 *
 * This function sets speaker buffer address and it notifies
 * audio speaker immediately. It also checks received buffer
 * from microphone and calls registrated callback.
 *
 * @param bufferAddr speaker buffer address with stored data
 *
 */
static void refresh(uint32_t bufferAddr)
{
    s_worker.sender->setBuffer(bufferAddr);
    s_worker.sender->base.notify();

    if (s_workerHandler != NULL && s_worker.receiver->isReady())
    {
        s_workerHandler((EIQ_IWorker_t*)&s_worker);
    }
}

/*!
 * @brief Sets ready Callback
 *
 * This function stores address of external function which
 * is called when data are ready.
 *
 * @param iworker address of external s_workerHandler.
 *
 */
static void setReadyCallback(EIQ_IWorkerUpdater_t iworker)
{
    if (iworker != NULL)
    {
        s_workerHandler = iworker;
    }
}

/*!
 * @brief Initializes the AudioWorker.
 *
 * This function initializes microphone and speaker.
 *
 * @return Pointer to initialized AudioWorker.
 */
EIQ_AudioWorker_t* EIQ_AudioWorkerInit(void)
{
    s_worker.base.start = start;
    s_worker.base.isReady = isReady;
    s_worker.base.getResolution = getResolution;
    s_worker.base.notify = notify;
    s_worker.base.getData = getData;
    s_worker.base.refresh = refresh;
    s_worker.base.setReadyCallback = setReadyCallback;
    s_worker.receiver = EIQ_MicroInit();
    s_worker.sender = EIQ_SpeakerInit();

    s_worker.receiver->setReadyCallback(s_worker.base.refresh);

    return &s_worker;
}

