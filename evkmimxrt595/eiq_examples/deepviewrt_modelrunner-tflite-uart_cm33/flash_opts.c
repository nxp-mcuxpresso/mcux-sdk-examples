#include "flash_opts.h"
#include "fsl_debug_console.h"

#if defined(__ARMCC_VERSION)
__attribute__((weak))
size_t __aeabi_read_tp(void)
{
  return 0;
}
#endif


status_t FlashInit(FlashConfig *config){
		serial_nor_config_option_t option;
		option.option0.U = 0xC0403000U;
		option.option1.U = 0U;
		status_t status = IAP_FlexspiNorAutoConfig(INSTANCE, config, &option);
		if (status == kStatus_Success){
				return 1;
		} else {
				return 0;
		}
}

status_t FlashErase(FlashConfig *config, uint32_t start, uint32_t length){
		status_t status = IAP_FlexspiNorErase(INSTANCE, config, start, length);
		if (status != kStatus_Success)
		{
				PRINTF("\r\n***NOR Flash Erase Failed!***\r\n");
		}else{
				PRINTF("FLASH 0x%x Erased, ", start);
		}
		return status;
}

status_t FlashProgram(FlashConfig *config, uint32_t start, uint32_t *src, uint32_t length){
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
		return status;
}
