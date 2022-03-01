/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "audio_usbd.h"
#include "fsl_iocon.h"
#include "fsl_i2s.h"
#include "i2s_if.h"
#include "sine_file.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define AUDIO_I2S_TX_FLEXCOMM_CLOCK_SOURCE (kAUDIO_PLL_to_FLEXCOMM6)
#define AUDIO_I2S_TX_RESET                 (kFC6_RST_SHIFT_RSTn)
#define AUDIO_I2S_TX_IRQn                  (FLEXCOMM6_IRQn)
#define AUDIO_I2S_TX                       (I2S0)
#define AUDIO_I2S_TX_ISR                   FLEXCOMM6_IRQHandler

#define AUDIO_I2S_RX_FLEXCOMM_CLOCK_SOURCE (kAUDIO_PLL_to_FLEXCOMM7)
#define AUDIO_I2S_RX_RESET                 (kFC7_RST_SHIFT_RSTn)
#define AUDIO_I2S_RX_IRQn                  (FLEXCOMM7_IRQn)
#define AUDIO_I2S_RX                       (I2S1)
#define AUDIO_I2S_RX_ISR                   FLEXCOMM7_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void i2s_tx_init(void)
{
    i2s_config_t txConfig;

    /* attach 12 MHz clock to FLEXCOMM6 */
    CLOCK_AttachClk(AUDIO_I2S_TX_FLEXCOMM_CLOCK_SOURCE);
    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(AUDIO_I2S_TX_RESET);

    /* initialize I2S */
    I2S_TxGetDefaultConfig(&txConfig);
    txConfig.divider = 16;
    I2S_TxInit(AUDIO_I2S_TX, &txConfig);
    AUDIO_I2S_TX->FIFOCFG |= (0x01 << 2);

    SYSCON->STARTERSET[0] = SYSCON_STARTER_FLEXCOMM6_MASK;
}

static void i2s_rx_init(void)
{
    i2s_config_t rxConfig;

    /* attach 12 MHz clock to FLEXCOMM6 */
    CLOCK_AttachClk(AUDIO_I2S_RX_FLEXCOMM_CLOCK_SOURCE);
    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(AUDIO_I2S_RX_RESET);

    /* initialize I2S */
    I2S_RxGetDefaultConfig(&rxConfig);
    rxConfig.divider = 1;
    I2S_RxInit(AUDIO_I2S_RX, &rxConfig);

    SYSCON->STARTERSET[0] = SYSCON_STARTER_FLEXCOMM7_MASK;
}

static void i2s_port_init(void)
{
    i2s_tx_init();
    i2s_rx_init();

    I2S_EnableInterrupts(AUDIO_I2S_TX, kI2S_TxErrorFlag | kI2S_TxLevelFlag);
    I2S_EnableInterrupts(AUDIO_I2S_RX, kI2S_RxErrorFlag | kI2S_RxLevelFlag);
}

void I2S_Start(void)
{
    /* Enable interrupts for I2S */
    EnableIRQ(AUDIO_I2S_TX_IRQn);
    I2S_Enable(AUDIO_I2S_TX);

    /* Enable interrupts for I2S */
    EnableIRQ(AUDIO_I2S_RX_IRQn);
    I2S_Enable(AUDIO_I2S_RX);
}

static inline void i2s_ClrFIFOStatus(I2S_Type *i2s, uint32_t mask)
{
    i2s->FIFOSTAT = mask;
}

void I2S_Stop(void)
{
    DisableIRQ(AUDIO_I2S_TX_IRQn);
    DisableIRQ(AUDIO_I2S_RX_IRQn);

    I2S_Disable(AUDIO_I2S_TX);
    i2s_ClrFIFOStatus(AUDIO_I2S_TX, kI2S_TxErrorFlag | kI2S_TxLevelFlag);

    I2S_Disable(AUDIO_I2S_RX);
    i2s_ClrFIFOStatus(AUDIO_I2S_RX, kI2S_RxErrorFlag | kI2S_RxLevelFlag);
}

void I2S_Init(void)
{
    i2s_port_init(); //	initialize I2S port
    I2S_Start();     //	start I2S TX
}

void I2S_SetSampleRate(uint32_t rate)
{
    return;
}

static inline uint8_t i2s_GetFIFOTxLevel(I2S_Type *i2s)
{
    return (i2s->FIFOSTAT >> 8) & 0xF;
}

static inline uint8_t i2s_GetFIFORxLevel(I2S_Type *i2s)
{
    return (i2s->FIFOSTAT >> 16) & 0xF;
}

static inline uint32_t i2s_GetFIFOStatus(I2S_Type *i2s)
{
    return (i2s->FIFOSTAT);
}

static volatile int32_t prev_play_ct, prev_rec_ct, curr_play_ct, curr_rec_ct;
static bool sample_reset;

bool I2S_SampleChk(void)
{
    if ((g_AdcCtrl.flags & (ADC_PLAY_DATAVALID | ADC_REC_DATAVALID)) == 0)
    {
        USB0->INTEN &= ~(1UL << 30);
        sample_reset = false;
        return false;
    }

    if (sample_reset == false)
    {
        sample_reset = true;
        if (g_AdcCtrl.flags & ADC_PLAY_DATAVALID)
        {
            prev_play_ct = curr_play_ct - i2s_GetFIFOTxLevel(AUDIO_I2S_TX);
        }
        else if (g_AdcCtrl.flags & ADC_REC_DATAVALID)
        {
            prev_rec_ct = curr_rec_ct - i2s_GetFIFORxLevel(AUDIO_I2S_RX);
        }
        return false;
    }

    return true;
}

int32_t I2S_SampleDiff(void)
{
    int32_t ct;
    int32_t diff;

    if (g_AdcCtrl.flags & ADC_PLAY_DATAVALID)
    {
        ct           = curr_play_ct - i2s_GetFIFOTxLevel(AUDIO_I2S_TX);
        diff         = ct - prev_play_ct;
        prev_play_ct = ct;
        if ((diff < 460) || (diff > 500))
        {
            diff = 480;
        }
    }
    else if (g_AdcCtrl.flags & ADC_REC_DATAVALID)
    {
        ct          = curr_rec_ct - i2s_GetFIFORxLevel(AUDIO_I2S_RX);
        diff        = ct - prev_rec_ct;
        prev_rec_ct = ct;
        if ((diff < 460) || (diff > 500))
        {
            diff = 480;
        }
    }
    else
    {
        diff = 480;
    }
    return diff;
}

static uint32_t usb_get_data(void)
{
    uint32_t data;
    ADC_SUBSTREAM_T *pSubs = &g_AdcCtrl.subs[SUBS_SPEAKER];
    uint8_t *idx           = (uint8_t *)&pSubs->rd_idx;
    const int clen         = pSubs->eTD[*idx].buf_length / 4;
    uint32_t *ptr          = (uint32_t *)pSubs->eTD[*idx].buf_ptr;
    static int cidx;

    data = ptr[cidx++];
    if (cidx >= clen)
    {
        cidx = 0;
        (*idx)++;
        if (*idx >= NUM_DTDS)
        {
            *idx = 0;
        }
    }
    pSubs->rd_count += 1;
    curr_play_ct += 1;
    return data;
}

static inline void i2s_Send(I2S_Type *i2s, uint32_t data)
{
    i2s->FIFOWR = data;
}

static void i2s_write(void)
{
    uint32_t data;
    while (i2s_GetFIFOStatus(AUDIO_I2S_TX) & I2S_FIFOSTAT_TXNOTFULL_MASK) // if FIFO is not full
    {
        if (g_AdcCtrl.flags & ADC_PLAY_DATAVALID) // if there's valid data...
        {
            data = usb_get_data(); // get data from the USB link
            // data = get_sine_data(); //get sine wave
        }
        else
        {
            data = SILENCE_DATA; // set data to silence
        }
        i2s_Send(AUDIO_I2S_TX, data); // send the data (or silence)
    }
}

static void usb_put_data(uint32_t sample)
{
    ADC_SUBSTREAM_T *pSubs = &g_AdcCtrl.subs[SUBS_MIC];            //	get the line-in/mic handle
    uint8_t *idx           = (uint8_t *)&pSubs->wr_idx;            //	point to write index
    uint32_t *ptr          = (uint32_t *)pSubs->eTD[*idx].buf_ptr; //	point to buffer
    static uint32_t cidx;                                          //	local index
    const uint32_t pkt_ct = DEF_SAMPLE_RATE / 1000;                //	calculate packet count

    ptr[cidx++] = sample; //	store the sample
    if (cidx >= pkt_ct)   //	if the cindx is greater than packet count...
    {
        pSubs->eTD[*idx].buf_length = pkt_ct * AUDIO_BYTES_PER_SAMPLE; //	set the length of this packet
        (*idx)++, cidx = 0;                                            //	go on to the next packet...
        if (*idx >= NUM_DTDS)
            *idx = 0; //	if we are past the packet count, then reset to the first packet again
    }

    if (g_AdcCtrl.flags & ADC_RECORDING)
    {
        if (g_AdcCtrl.flags & ADC_REC_DATAVALID)
        {
            pSubs->wr_count++;
            curr_rec_ct += 1;
            return;
        }
        if ((*idx == 0) || (*idx == NUM_DTDS))
        {
            pSubs->rd_idx = *idx ? 0 : NUM_DTDS;
            g_AdcCtrl.flags |= ADC_REC_DATAVALID;
            ADC_start_xfr(&g_AdcCtrl, pSubs, ADC_USB_READER);
            USB0->INTEN |= (1UL << 30);
        }
    }
}

static inline uint32_t i2s_Receive(I2S_Type *i2s)
{
    return i2s->FIFORD;
}

static void i2s_read(void)
{
    uint32_t sample;
    while (i2s_GetFIFOStatus(AUDIO_I2S_RX) & I2S_FIFOSTAT_RXNOTEMPTY_MASK) //	if FIFO is not empty
    {
        sample = i2s_Receive(AUDIO_I2S_RX); //	read the data
        // sample = get_sine_data(); //  get sine wave
        usb_put_data(sample); //	store the sample
    }
}

void i2s_ErrorHandler(I2S_Type *base)
{
    uint32_t intstat;
    uint32_t i2sstat;

    intstat = base->FIFOINTSTAT;              /* get FIFO pending interrupt status */
    if (intstat & I2S_FIFOINTSTAT_TXERR_MASK) /* process tx error */
    {
        /* Clear TX error interrupt flag */
        base->FIFOSTAT = I2S_FIFOSTAT_TXERR(1);
    }
    if (intstat & I2S_FIFOINTSTAT_RXERR_MASK) /* process rx error */
    {
        /* Clear TX error interrupt flag */
        base->FIFOSTAT = 0x02U;
    }

    if (intstat & I2S_FIFOINTSTAT_PERINT_MASK) /* process peripheral interrupt */
    {
        i2sstat = base->STAT;                  /* get peripheral status */
        if (i2sstat & I2S_STAT_SLVFRMERR_MASK) /* test for slave frame error status */
        {
            base->STAT = I2S_STAT_SLVFRMERR_MASK;
        }
    }
}

void AUDIO_I2S_TX_ISR(void)
{
    i2s_ErrorHandler(AUDIO_I2S_TX); //	Accumulate TX errors then, clear status
    i2s_write();                    // read data from I2S, write to USB
}

void AUDIO_I2S_RX_ISR(void) //	I2S receive interrupt
{
    i2s_ErrorHandler(AUDIO_I2S_RX); //	Accumulate RX errors then, clear status
    i2s_read();                     // read data from USB, write to I2S
}
