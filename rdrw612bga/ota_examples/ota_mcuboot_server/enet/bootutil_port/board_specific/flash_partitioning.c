#include "flash_partitioning.h"
#include "flash_map.h"
#include "mcuboot_config.h"
#include "sysflash/sysflash.h"

#if !(defined MONOLITHIC_APP && (MONOLITHIC_APP != 0))
const char *boot_image_names[MCUBOOT_IMAGE_NUMBER] = {"APP", "WIFI"};
struct flash_area boot_flash_map[MCUBOOT_IMAGE_SLOT_NUMBER] = {
    /* Image 0; slot 0 - Main Application Active Image  */
    {.fa_id        = 0,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_ACT_APP - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP,
     .fa_name      = "APP_PRIMARY"
    },

    /* Image 0; slot 1 - Main Application Candidate Image  */
    {.fa_id        = 1,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_CAND_APP - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP,
     .fa_name      = "APP_SECONDARY"
    },


    /* Image 1; slot 2 - WiFi CPU1 Firmware Active Image  */
    {.fa_id        = 2,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_ACT_WIFI - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_WIFI - BOOT_FLASH_ACT_WIFI,
     .fa_name      = "WIFI_PRIMARY"
    },

    /* Image 1; slot 3 - WiFi CPU1 Firmware Candidate Image  */
    {.fa_id        = 3,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_CAND_WIFI - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_WIFI - BOOT_FLASH_ACT_WIFI,
     .fa_name      = "WIFI_SECONDARY"
    }
};
#else
const char *boot_image_names[MCUBOOT_IMAGE_NUMBER] = {"APP"};
struct flash_area boot_flash_map[MCUBOOT_IMAGE_SLOT_NUMBER] = {
    /* Image 0; slot 0 - Main Application Active Image  */
    {.fa_id        = 0,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_ACT_APP - BOOT_FLASH_BASE,
     .fa_size      = (BOOT_MAX_IMG_NB_SECTORS-1)*4096u,
     .fa_name      = "APP_PRIMARY"
    },

    /* Image 0; slot 1 - Main Application Candidate Image  */
    {.fa_id        = 1,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_CAND_APP - BOOT_FLASH_BASE,
     .fa_size      = (BOOT_MAX_IMG_NB_SECTORS-1)*4096u,
     .fa_name      = "APP_SECONDARY"
    }
};
#endif

#if !defined(MCUBOOT_SWAP_USING_MOVE) && !defined(MCUBOOT_OVERWRITE_ONLY)
#warning "Make sure scratch area is defined if required by defined swap mechanism"
#endif
