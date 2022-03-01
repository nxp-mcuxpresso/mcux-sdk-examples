/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_uart_cmsis.h"
#include "fsl_rdc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART                Driver_USART4
#define EXAMPLE_UART_DMA_BASEADDR SDMAARM1
#define ECHO_BUFFER_LENGTH 8

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

AT_NONCACHEABLE_SECTION_INIT(uint8_t g_tipString[]) =
    "USART CMSIS SDMA example.\r\nBoard receives 8 characters then sends them out.\r\nNow please input:\r\n";
AT_NONCACHEABLE_SECTION_ALIGN_INIT(uint8_t g_txBuffer[ECHO_BUFFER_LENGTH], 4) = {0};
AT_NONCACHEABLE_SECTION_ALIGN_INIT(uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH], 4) = {0};

volatile bool rxBufferEmpty = true;
volatile bool txBufferFull  = false;
volatile bool txOnGoing     = false;
volatile bool rxOnGoing     = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t UART4_GetFreq(void)
{
    return BOARD_DEBUG_UART_CLK_FREQ;
}

/* USART  callback */
void USART_Callback(uint32_t event)
{
    if (event == ARM_USART_EVENT_SEND_COMPLETE)
    {
        txBufferFull = false;
        txOnGoing    = false;
    }

    if (event == ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        rxBufferEmpty = false;
        rxOnGoing     = false;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    sdma_config_t sdmaConfig;

    /* Only configure the RDC if RDC peripheral write access is allowed. */
    if ((0x1U & RDC_GetPeriphAccessPolicy(RDC, kRDC_Periph_RDC, RDC_GetCurrentMasterDomainId(RDC))) != 0U)
    {
        /*set SDMA1 PERIPH to M7 Domain(DID=1),due to UART not be accessible by DID=0 by default*/
        rdc_domain_assignment_t assignment = {0};
        assignment.domainId                = BOARD_DOMAIN_ID;
        RDC_SetMasterDomainAssignment(RDC, kRDC_Master_SDMA1_PERIPH, &assignment);
    }

    /* M7 has its local cache and enabled by default,
     * need to set smart subsystems (0x28000000 ~ 0x3FFFFFFF)
     * non-cacheable before accessing this address region */
    BOARD_InitMemory();

    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();

    /* Init the SDMA module */
    SDMA_GetDefaultConfig(&sdmaConfig);
    SDMA_Init(EXAMPLE_UART_DMA_BASEADDR, &sdmaConfig);
    DEMO_USART.Initialize(USART_Callback);
    DEMO_USART.PowerControl(ARM_POWER_FULL);

    /* Set baudrate. */
    DEMO_USART.Control(ARM_USART_MODE_ASYNCHRONOUS, BOARD_DEBUG_UART_BAUDRATE);

    /* Send g_tipString out. */
    txOnGoing = true;

    DEMO_USART.Send(g_tipString, sizeof(g_tipString) - 1);

    /* Wait send finished */
    while (txOnGoing)
    {
    }

    while (1)
    {
        /* If RX is idle and g_rxBuffer is empty, start to read data to g_rxBuffer. */
        if ((!rxOnGoing) && rxBufferEmpty)
        {
            rxOnGoing = true;
            DEMO_USART.Receive(g_rxBuffer, ECHO_BUFFER_LENGTH);
        }

        /* If TX is idle and g_txBuffer is full, start to send data. */
        if ((!txOnGoing) && txBufferFull)
        {
            txOnGoing = true;
            DEMO_USART.Send(g_txBuffer, ECHO_BUFFER_LENGTH);
        }

        /* If g_txBuffer is empty and g_rxBuffer is full, copy g_rxBuffer to g_txBuffer. */
        if ((!rxBufferEmpty) && (!txBufferFull))
        {
            memcpy(g_txBuffer, g_rxBuffer, ECHO_BUFFER_LENGTH);
            rxBufferEmpty = true;
            txBufferFull  = true;
        }
    }
}
