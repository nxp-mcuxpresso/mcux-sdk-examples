/* @TEST_ANCHOR */
#define WIFI_IW416_BOARD_MURATA_1XK_USD
/* @END_TEST_ANCHOR */

/*        Module        */
/*#define WIFI_IW416_BOARD_AW_AM457_USD*/
/*#define WIFI_IW416_BOARD_AW_AM457MA*/
/*#define WIFI_88W8977_BOARD_PAN9026_SDIO*/
/*#define WIFI_88W8801_BOARD_AW_NM191MA*/
/*#define WIFI_BOARD_AW_AM281SM*/
/*#define WIFI_88W8987_BOARD_AW_CM358MA*/
/*#define WIFI_88W8987_BOARD_AW_CM358_USD*/
/*#define WIFI_IW416_BOARD_MURATA_1XK_USD*/
/*#define WIFI_88W8987_BOARD_MURATA_1ZM_USD*/

/*        Feature        */
/*#define CONFIG_BT_IND_DNLD*/

#include "wifi_bt_module_config.h"
#include "wifi_config.h"

#ifdef CONFIG_BT_IND_DNLD
#define ENABLE_BT_IND_RESET
#endif
