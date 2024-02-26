#include "fsl_flash.h"

#define FLASH_MODEL_ADDR 0x100000
#define FLASH_BASE_ADDR  0x0

typedef flash_config_t FlashConfig;

#if defined(__cplusplus)
extern "C" {
#endif

status_t FlashInit(FlashConfig *config);
status_t FlashErase(FlashConfig *config, uint32_t start, uint32_t length);
status_t FlashProgram(FlashConfig *config, uint32_t start, uint32_t *src, uint32_t length);

#if defined(__cplusplus)
}
#endif
