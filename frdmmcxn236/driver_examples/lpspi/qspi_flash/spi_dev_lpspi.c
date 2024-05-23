/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "spi_dev_lpspi.h"
#include "spi_dev.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPSPI_MAX_FRAME_BYTE (4096U / 8U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t SPI_DEV_Receive(LPSPI_Type *lpspi, uint8_t* data, uint32_t len, uint32_t lpspi_cmd);
static status_t SPI_DEV_Send(LPSPI_Type *lpspi, const uint8_t* data, uint32_t len, uint32_t lpspi_cmd);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t SPI_DEV_Init(spi_dev_t *spi_dev, LPSPI_Type* lpspi, lpspi_cs_func_t cs_func)
{
    /*
     * Read TCR to get the CPOL, CPHA, PRESCALE, PCS, these won't change
     * during the transfer.
     * Wait for TX FIFO empty to read TCR, to make sure TCR value is valid.
     */
    while (LPSPI_GetTxFifoCount(lpspi) != 0U);
    uint32_t LPSPI_TCR = lpspi->TCR & (LPSPI_TCR_CPOL_MASK | LPSPI_TCR_CPHA_MASK | LPSPI_TCR_PRESCALE_MASK | LPSPI_TCR_PCS_MASK);

    spi_dev->lpspi   = lpspi;
    spi_dev->reg_tcr = LPSPI_TCR;
    spi_dev->cs_func = cs_func;

    return kStatus_Success;
}

static status_t SPI_DEV_Send(LPSPI_Type *lpspi, const uint8_t* data, uint32_t len, uint32_t lpspi_cmd)
{
    uint32_t dataWord;

    lpspi_cmd |= LPSPI_TCR_RXMSK_MASK;

    if (len >= 4)
    {
        /* Wait for TX FIFO ready for send command. */
        while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_TxDataRequestFlag) == 0U);
        lpspi->TCR = lpspi_cmd | LPSPI_TCR_FRAMESZ(32U - 1U);

        /* Send data word by word. */
        while (len >= 4)
        {
            len -= 4;

            dataWord  = (uint32_t)(*data++) << 24U;
            dataWord |= (uint32_t)(*data++) << 16U;
            dataWord |= (uint32_t)(*data++) << 8U;
            dataWord |= (uint32_t)(*data++) << 0U;

            while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_TxDataRequestFlag) == 0U);
            LPSPI_WriteData(lpspi, dataWord);
        }
    }

    if (len > 0)
    {
        /* Wait for TX FIFO ready for send command. */
        while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_TxDataRequestFlag) == 0U);
        lpspi->TCR = lpspi_cmd | LPSPI_TCR_FRAMESZ((len * 8U) - 1U);

        dataWord = 0U;

        while (len > 0)
        {
            len--;
            dataWord  |= (uint32_t)(*data++) << (8U * len);
        }

        while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_TxDataRequestFlag) == 0U);
        LPSPI_WriteData(lpspi, dataWord);
    }

    return kStatus_Success;
}

static status_t SPI_DEV_Receive(LPSPI_Type *lpspi, uint8_t* data, uint32_t len, uint32_t lpspi_cmd)
{
    uint32_t dataWord;
    uint32_t curReadLen;

    lpspi_cmd |= LPSPI_TCR_TXMSK_MASK;

    while (len > 0)
    {
        curReadLen = MIN(len, LPSPI_MAX_FRAME_BYTE);
        len -= curReadLen;

        /* Wait for TX FIFO ready for send command. */
        while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_TxDataRequestFlag) == 0U);
        lpspi->TCR = lpspi_cmd | LPSPI_TCR_FRAMESZ((curReadLen * 8U) - 1U);

        if (curReadLen >= 4)
        {
            /* Read data word by word. */
            while (curReadLen >= 4)
            {
                curReadLen -= 4;
                while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_RxDataReadyFlag) == 0U);
                dataWord = LPSPI_ReadData(lpspi);

                *data++ = (uint8_t)(dataWord >> 24U);
                *data++ = (uint8_t)(dataWord >> 16U);
                *data++ = (uint8_t)(dataWord >> 8U);
                *data++ = (uint8_t)(dataWord >> 0U);
            }
        }

        if (curReadLen > 0)
        {
            while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_RxDataReadyFlag) == 0U);
            dataWord = LPSPI_ReadData(lpspi);

            while (curReadLen > 0)
            {
                curReadLen--;
                *data++ = (uint8_t)(dataWord >> (8U * curReadLen));
            }
        }
    }

    return kStatus_Success;
}

status_t SPI_DEV_SendReceive(spi_dev_t *spi_dev, const spi_tx_rx_info_t *infos, uint8_t infoLen)
{
    status_t status;
    uint32_t lpspi_cmd;

    LPSPI_Type *lpspi = spi_dev->lpspi;

    spi_dev->cs_func(0);

    for (uint8_t i = 0; i<infoLen; i++)
    {
        /*
         * Prepare the command
         */
        lpspi_cmd = spi_dev->reg_tcr;

        /* Data width. */
        if (2 == infos[i].dataWidth)
        {
            lpspi_cmd |= LPSPI_TCR_WIDTH(1U);
        }
        else if (4 == infos[i].dataWidth)
        {
            lpspi_cmd |= LPSPI_TCR_WIDTH(2U);
        }
        else
        {
        }

        if (NULL != infos[i].txData)
        {
            status = SPI_DEV_Send(lpspi, infos[i].txData, infos[i].dataLen, lpspi_cmd);
        }
        else
        {
            status = SPI_DEV_Receive(lpspi, infos[i].rxData, infos[i].dataLen, lpspi_cmd);
        }
    }

    /* Wait for transfer done. */
    while (LPSPI_GetTxFifoCount(lpspi) != 0U);
    while ((LPSPI_GetStatusFlags(lpspi) & (uint32_t)kLPSPI_ModuleBusyFlag) != 0U);

    spi_dev->cs_func(1);

    return status;
}
