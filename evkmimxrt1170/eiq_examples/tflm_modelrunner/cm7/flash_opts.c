#include "flash_opts.h"
#include "fsl_debug_console.h"

status_t FlashInit(FlashConfig *config){
    static serial_nor_config_option_t option = {
        .option0.U = 0xc0000007U,
        .option1.U = 0U,
    };
    ROM_API_Init();

    status_t status = ROM_FLEXSPI_NorFlash_GetConfig(FlexSpiInstance, config, &option);
    if (status == kStatus_Success)
    {
        status = ROM_FLEXSPI_NorFlash_Init(FlexSpiInstance, config);
        if (status == kStatus_Success)
        {
            return kStatus_Success;
        }
        PRINTF("Flash init failed!!\r\n");
    }else{
        PRINTF("\r\n Get FLEXSPI NOR configuration block failure!\r\n");
    }
    return kStatus_Fail;
}

status_t FlashErase(FlashConfig *config, uint32_t start, uint32_t length){
    status_t status = ROM_FLEXSPI_NorFlash_Erase(FlexSpiInstance, config, start, length);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NOR Flash Erase Failed!***\r\n");
    }else{
        PRINTF("FLASH 0x%x Erased, ", start);
    }
    return status;
}

status_t FlashProgram(FlashConfig *config, uint32_t start, uint32_t *src, uint32_t length){

    status_t status;
    for(int i=0; i < length; i+=4096){
        status = ROM_FLEXSPI_NorFlash_ProgramPage(FlexSpiInstance,
                                                  config, start, (const uint32_t *)src+i);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n Page program failure!\r\n");
            return status;
        }
    }
    PRINTF("Program at %x: %d bytes  ==\r\n", start, length);
    return status;
}

