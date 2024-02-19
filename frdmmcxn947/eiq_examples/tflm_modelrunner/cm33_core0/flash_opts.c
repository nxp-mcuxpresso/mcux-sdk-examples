#include "flash_opts.h"
#include "fsl_debug_console.h"

status_t FlashInit(FlashConfig *config){
    if (FLASH_Init(config) == kStatus_Success)
    {
        return kStatus_Success;
    }
    PRINTF("Flash init failed!!\r\n");
    return kStatus_Fail;
}

status_t FlashErase(FlashConfig *config, uint32_t start, uint32_t length){
    status_t status = FLASH_Erase(config, (start & 0x0FFFFFFF), length, kFLASH_ApiEraseKey);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NOR Flash Erase Failed!***\r\n");
    }else{
        PRINTF("FLASH 0x%x Erased, ", start);
    }
    return status;
}

status_t FlashProgram(FlashConfig *config, uint32_t start, uint32_t *src, uint32_t length){
    status_t status = FLASH_Program(config, start, (uint8_t*)src, length );
    if (status != kStatus_Success)
    {
        PRINTF("\r\nNOR Flash %x Program Failed!***\r\n", start);
        return status;
    }
    PRINTF("Program at %x: %d bytes  ==\r\n", start, length);
    return status;
}
