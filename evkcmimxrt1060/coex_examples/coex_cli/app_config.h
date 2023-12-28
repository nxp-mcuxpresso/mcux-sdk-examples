/* @TEST_ANCHOR */
#define WIFI_IW612_BOARD_MURATA_2EL_M2
/* @END_TEST_ANCHOR */

/*#define WIFI_IW612_BOARD_RD_USD*/
/*#define WIFI_IW612_BOARD_MURATA_2EL_M2*/
/*#define WIFI_IW416_BOARD_MURATA_1XK_M2*/
/*#define WIFI_88W8987_BOARD_MURATA_1ZM_M2*/

/*        Feature        */
#ifdef WIFI_IW612_BOARD_MURATA_2EL_M2
#define PCAL6408A_IO_EXP_ENABLE
#endif /*WIFI_IW612_BOARD_MURATA_2EL_M2*/

#include "wifi_bt_module_config.h"
#include "wifi_config.h"

#ifdef CONFIG_BT_IND_DNLD
#define ENABLE_BT_IND_RESET
#endif
