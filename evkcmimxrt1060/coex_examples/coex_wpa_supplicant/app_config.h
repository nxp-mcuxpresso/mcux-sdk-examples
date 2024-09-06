/* @TEST_ANCHOR */
#define WIFI_IW612_BOARD_MURATA_2EL_M2
/* @END_TEST_ANCHOR */

/*#define WIFI_IW612_BOARD_RD_USD*/
/*#define WIFI_IW612_BOARD_MURATA_2EL_M2*/
/*#define WIFI_IW416_BOARD_MURATA_1XK_M2*/
/*#define WIFI_88W8987_BOARD_MURATA_1ZM_M2*/

/* Enable for BT indpendent firmware download */
/*#define CONFIG_BT_IND_DNLD*/

#define APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK 1U
#define APP_CONFIG_ENABLE_MALLOC_FAILURE_FREERTOS_HOOK 1U

#ifdef WIFI_IW612_BOARD_MURATA_2EL_M2
#define PCAL6408A_IO_EXP_ENABLE
#endif /*WIFI_IW612_BOARD_MURATA_2EL_M2*/

/* Enable coex_examples for board */
#define CONFIG_DISABLE_BLE 0
#define CONFIG_WIFI_BLE_COEX_APP 1

#include "wifi_bt_module_config.h"

#ifdef CONFIG_BT_IND_DNLD
#define ENABLE_BT_IND_RESET
#endif

/* Enable dcache using COEX_PERF_MODE=1 */
