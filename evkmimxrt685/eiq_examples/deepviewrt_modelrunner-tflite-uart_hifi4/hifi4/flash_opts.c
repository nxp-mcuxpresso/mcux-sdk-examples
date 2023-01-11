#include "flash_opts.h"
#include "fsl_debug_console.h"
#include "timer.h"

#define SYSTICK_PRESCALE 1U

#if defined(__ARMCC_VERSION)
__attribute__((weak))
size_t __aeabi_read_tp(void)
{
  return 0;
}
#endif

int TIMER_Stop(void){
#ifndef __XCC__
		uint32_t priorityGroup = NVIC_GetPriorityGrouping();
		SysTick_Config(CLOCK_GetFreq(kCLOCK_CoreSysClk) / (SYSTICK_PRESCALE * 1000U));
		NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(priorityGroup, 10000, 0U));
		SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
				SysTick_CTRL_TICKINT_Msk   |
				0;                         /*  Enable SysTick IRQ and SysTick Timer */
#endif
		return 0;
}

AT_QUICKACCESS_SECTION_CODE(status_t BOARD_FlexspiInit(uint32_t instance,
					    FlashConfig *config,
						serial_nor_config_option_t *option))
{
		// Reset external flash
		GPIO->CLR[2] = 1 << 12;
		for (uint32_t i = 0; i < 6000; i++)
				__NOP();
		GPIO->SET[2] = 1 << 12;
		// Clear FLEXSPI NOR flash configure context
		SYSCTL0->FLEXSPI_BOOTROM_SCRATCH0 = 0;
		status_t status = IAP_FlexspiNorAutoConfig(instance, config, option);
		if ((CACHE64->CCR & CACHE64_CTRL_CCR_ENCACHE_MASK) != 0)
		{
				/* set command to invalidate all ways and write GO bit to initiate command */
				CACHE64->CCR = CACHE64_CTRL_CCR_INVW1_MASK | CACHE64_CTRL_CCR_INVW0_MASK;
				CACHE64->CCR |= CACHE64_CTRL_CCR_GO_MASK;
				/* Wait until the command completes */
				while (CACHE64->CCR & CACHE64_CTRL_CCR_GO_MASK)
				{
				}
				/* As a precaution clear the bits to avoid inadvertently re-running this command. */
				CACHE64->CCR &= ~(CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK);
		}
		return status;
}

status_t FlashInit(FlashConfig *config){
		status_t status;
		serial_nor_config_option_t option;
		option.option0.U = 0xC1503051U;
		option.option1.U = 0x20000014U;
		status = BOARD_FlexspiInit(INSTANCE, config, &option);
		if (status != kStatus_Success)
		{
				PRINTF("\r\n***NOR Flash Initialization Failed!***\r\n");

		}
		if (status == kStatus_Success){
				return 1;
		} else {
				return 0;
		}


}

status_t FlashErase(FlashConfig *config, uint32_t start, uint32_t length){
		TIMER_Stop();
		status_t status = IAP_FlexspiNorErase(INSTANCE, config, start, length);
		if (status != kStatus_Success)
		{
				PRINTF("\r\n***NOR Flash Erase Failed!***\r\n");
		}else{
				PRINTF("FLASH 0x%x Erased, ", start);
		}
		TIMER_Init();
		return status;
}

status_t FlashProgram(FlashConfig *config, uint32_t start, uint32_t *src, uint32_t length){
		TIMER_Stop();
		uint32_t pages = length / config->pageSize;
		status_t status;
		for (int j = 0; j < pages; j++)
		{
				status = IAP_FlexspiNorPageProgram(INSTANCE, config,
								start + j * config->pageSize, (uint32_t*)((char*)src + j* config->pageSize));
				if (status != kStatus_Success)
				{
						PRINTF("\r\nNOR Flash %d Page %d Program Failed!***\r\n",pages, j);
						break;
				}
		}
		PRINTF("Program at %x: 16384 bytes  ==\r\n", start);
		TIMER_Init();
		return status;
}
