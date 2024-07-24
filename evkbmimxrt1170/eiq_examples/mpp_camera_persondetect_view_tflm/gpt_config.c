/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * General purpose timer setup for supporting FreeRTOS runtime
 * task statistics
 */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#if (configGENERATE_RUN_TIME_STATS == 1)
#include "fsl_gpt.h"

volatile unsigned long ulHighFrequencyTimerTicks;

void GPT2_IRQHandler(void) {
  /* Clear interrupt flag.*/
  GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);
  ulHighFrequencyTimerTicks++;
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
  __DSB();
#endif
}

void vConfigureTimerForRunTimeStats(void) {
  uint32_t gptFreq;
  gpt_config_t gptConfig;

  GPT_GetDefaultConfig(&gptConfig);

  /* Initialize GPT module */
  GPT_Init(GPT2, &gptConfig);

  /* Divide GPT clock source frequency by 3 inside GPT module */
  GPT_SetClockDivider(GPT2, 3);

  /* Get GPT clock frequency */
  gptFreq = CLOCK_GetFreq(kCLOCK_OscRc48MDiv2);

  /* GPT frequency is divided by 3 inside module */
  gptFreq /= 3;

  /* Set GPT module to 10x of the FreeRTOS tick counter */
  gptFreq = USEC_TO_COUNT((100 * 1000)/configTICK_RATE_HZ, gptFreq);
  GPT_SetOutputCompareValue(GPT2, kGPT_OutputCompare_Channel1, gptFreq);

  /* Enable GPT Output Compare1 interrupt */
  GPT_EnableInterrupts(GPT2, kGPT_OutputCompare1InterruptEnable);

  /* Enable at the Interrupt and start timer */
  EnableIRQ(GPT2_IRQn);
  GPT_StartTimer(GPT2);
}

unsigned long vGetTimerForRunTimeStats()
{
    return ulHighFrequencyTimerTicks;
}
#endif
