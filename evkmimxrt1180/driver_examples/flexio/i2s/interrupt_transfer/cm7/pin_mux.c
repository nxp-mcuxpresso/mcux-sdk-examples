/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v15.0
processor: MIMXRT1189xxxxx
package_id: MIMXRT1189CVM8B
mcu_data: ksdk2_0
processor_version: 0.15.9
board: MIMXRT1180-EVK
pin_labels:
- {pin_num: B1, pin_signal: GPIO_AON_08}
- {pin_num: A5, pin_signal: GPIO_AON_09}
- {pin_num: B3, pin_signal: GPIO_AON_15}
- {pin_num: C5, pin_signal: GPIO_AON_16}
- {pin_num: F6, pin_signal: GPIO_AON_21}
- {pin_num: C3, pin_signal: GPIO_AON_22}
- {pin_num: C2, pin_signal: GPIO_AON_23}
- {pin_num: C1, pin_signal: GPIO_AON_24}
- {pin_num: D2, pin_signal: GPIO_AON_25}
- {pin_num: R13, pin_signal: GPIO_AD_02}
- {pin_num: R15, pin_signal: GPIO_AD_03}
- {pin_num: P15, pin_signal: GPIO_AD_04}
- {pin_num: P13, pin_signal: GPIO_AD_05}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 * 
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 * 
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void) {
    BOARD_InitPins();
    BOARD_InitDEBUG_UARTPins();
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: F6, peripheral: SAI1, signal: sai_tx_data0, pin_signal: GPIO_AON_21, software_input_on: Enable, pull_up_down_config: Pull_Down}
  - {pin_num: C3, peripheral: SAI1, signal: sai_tx_sync, pin_signal: GPIO_AON_22, software_input_on: Enable, pull_up_down_config: Pull_Down}
  - {pin_num: C2, peripheral: SAI1, signal: sai_tx_bclk, pin_signal: GPIO_AON_23, software_input_on: Enable, pull_up_down_config: Pull_Down}
  - {pin_num: C1, peripheral: SAI1, signal: sai_mclk, pin_signal: GPIO_AON_24, software_input_on: Enable, pull_up_down_config: Pull_Down}
  - {pin_num: D2, peripheral: SAI1, signal: sai_rx_data0, pin_signal: GPIO_AON_25, software_input_on: Enable, pull_up_down_config: Pull_Down}
  - {pin_num: C5, peripheral: LPI2C2, signal: SCL, pin_signal: GPIO_AON_16, software_input_on: Enable, pull_up_down_config: Pull_Up, pull_keeper_select: Keeper, open_drain: Enable}
  - {pin_num: B3, peripheral: LPI2C2, signal: SDA, pin_signal: GPIO_AON_15, software_input_on: Enable, pull_up_down_config: Pull_Up, pull_keeper_select: Keeper, open_drain: Enable}
  - {pin_num: R13, peripheral: FLEXIO2, signal: 'IO, 02', pin_signal: GPIO_AD_02, pull_keeper_select: Keeper}
  - {pin_num: R15, peripheral: FLEXIO2, signal: 'IO, 03', pin_signal: GPIO_AD_03, pull_keeper_select: Keeper}
  - {pin_num: P15, peripheral: FLEXIO2, signal: 'IO, 04', pin_signal: GPIO_AD_04, pull_keeper_select: Keeper}
  - {pin_num: P13, peripheral: FLEXIO2, signal: 'IO, 05', pin_signal: GPIO_AD_05, pull_keeper_select: Keeper}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc1);          /* Turn on LPCG: LPCG is ON. */
  CLOCK_EnableClock(kCLOCK_Iomuxc2);          /* Turn on LPCG: LPCG is ON. */

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_02_FLEXIO2_FLEXIO02,     /* GPIO_AD_02 is configured as FLEXIO2_FLEXIO02 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_03_FLEXIO2_FLEXIO03,     /* GPIO_AD_03 is configured as FLEXIO2_FLEXIO03 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_04_FLEXIO2_FLEXIO04,     /* GPIO_AD_04 is configured as FLEXIO2_FLEXIO04 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_05_FLEXIO2_FLEXIO05,     /* GPIO_AD_05 is configured as FLEXIO2_FLEXIO05 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_15_LPI2C2_SDA,          /* GPIO_AON_15 is configured as LPI2C2_SDA */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_15 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_16_LPI2C2_SCL,          /* GPIO_AON_16 is configured as LPI2C2_SCL */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_16 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_21_SAI1_TX_DATA00,      /* GPIO_AON_21 is configured as SAI1_TX_DATA00 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_21 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_22_SAI1_TX_SYNC,        /* GPIO_AON_22 is configured as SAI1_TX_SYNC */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_22 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_23_SAI1_TX_BCLK,        /* GPIO_AON_23 is configured as SAI1_TX_BCLK */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_23 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_24_SAI1_MCLK,           /* GPIO_AON_24 is configured as SAI1_MCLK */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_24 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_25_SAI1_RX_DATA00,      /* GPIO_AON_25 is configured as SAI1_RX_DATA00 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_AON_25 */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_02_FLEXIO2_FLEXIO02,     /* GPIO_AD_02 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_03_FLEXIO2_FLEXIO03,     /* GPIO_AD_03 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_04_FLEXIO2_FLEXIO04,     /* GPIO_AD_04 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_05_FLEXIO2_FLEXIO05,     /* GPIO_AD_05 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_15_LPI2C2_SDA,          /* GPIO_AON_15 PAD functional properties : */
      0x1AU);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull up
                                                 Open Drain Field: Enabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_16_LPI2C2_SCL,          /* GPIO_AON_16 PAD functional properties : */
      0x1AU);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull up
                                                 Open Drain Field: Enabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_21_SAI1_TX_DATA00,      /* GPIO_AON_21 PAD functional properties : */
      0x06U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_22_SAI1_TX_SYNC,        /* GPIO_AON_22 PAD functional properties : */
      0x06U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_23_SAI1_TX_BCLK,        /* GPIO_AON_23 PAD functional properties : */
      0x06U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_24_SAI1_MCLK,           /* GPIO_AON_24 PAD functional properties : */
      0x06U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_25_SAI1_RX_DATA00,      /* GPIO_AON_25 PAD functional properties : */
      0x06U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
}


/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitDEBUG_UARTPins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: A5, peripheral: LPUART1, signal: RXD, pin_signal: GPIO_AON_09, pull_up_down_config: Pull_Down, pull_keeper_select: Keeper, open_drain: Disable, drive_strength: High,
    slew_rate: Slow}
  - {pin_num: B1, peripheral: LPUART1, signal: TXD, pin_signal: GPIO_AON_08, pull_up_down_config: Pull_Down, pull_keeper_select: Keeper, open_drain: Disable, drive_strength: High,
    slew_rate: Slow}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitDEBUG_UARTPins, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitDEBUG_UARTPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc2);          /* Turn on LPCG: LPCG is ON. */

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_08_LPUART1_TX,          /* GPIO_AON_08 is configured as LPUART1_TX */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_09_LPUART1_RX,          /* GPIO_AON_09 is configured as LPUART1_RX */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_08_LPUART1_TX,          /* GPIO_AON_08 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_09_LPUART1_RX,          /* GPIO_AON_09 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled */
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
