/*
 * Copyright 2022 NXP
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
product: Pins v13.0
processor: MIMXRT1189xxxxx
package_id: MIMXRT1189CVM8B
mcu_data: ksdk2_0
processor_version: 0.13.1
pin_labels:
- {pin_num: T17, pin_signal: GPIO_AD_07, label: SDIO_RST, identifier: SDIO_RST;sdio_}
- {pin_num: M17, pin_signal: GPIO_AD_17, label: WL_RST, identifier: WL_RST}
- {pin_num: N14, pin_signal: GPIO_AD_14, label: SD_PWREN_B, identifier: SD_PWREN_B}
- {pin_num: N16, pin_signal: GPIO_AD_15, label: SD1_CD_B, identifier: SD1_CD_B}
- {pin_num: L15, pin_signal: GPIO_AD_29, label: SD1_VSELECT, identifier: SD1_VSELECT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_rgpio.h"
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
    BOARD_InitM2UARTPins();
    BOARD_InitPinsM2();
    BOARD_InitUSDHCPins();
    BOARD_InitSPIPins();
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: B4, peripheral: RGPIO1, signal: 'gpio_io, 04', pin_signal: GPIO_AON_04, pull_up_down_config: Pull_Up, drive_strength: High}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc2);          /* Turn on LPCG: LPCG is ON. */

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AON_04_GPIO1_IO04,          /* GPIO_AON_04 is configured as GPIO1_IO04 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AON_04_GPIO1_IO04,          /* GPIO_AON_04 PAD functional properties : */
      0x0EU);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Enable
                                                 Pull Up / Down Config. Field: Weak pull up
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


/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitM2UARTPins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: J15, peripheral: LPUART10, signal: CTS_B, pin_signal: GPIO_AD_34, pull_keeper_select: Keeper}
  - {pin_num: J16, peripheral: LPUART10, signal: RXD, pin_signal: GPIO_AD_33, pull_keeper_select: Keeper}
  - {pin_num: H17, peripheral: LPUART10, signal: RTS_B, pin_signal: GPIO_AD_35, pull_keeper_select: Keeper}
  - {pin_num: J17, peripheral: LPUART10, signal: TXD, pin_signal: GPIO_AD_32, pull_keeper_select: Keeper}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitM2UARTPins, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitM2UARTPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc1);          /* Turn on LPCG: LPCG is ON. */

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_32_LPUART10_TX,          /* GPIO_AD_32 is configured as LPUART10_TX */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_33_LPUART10_RX,          /* GPIO_AD_33 is configured as LPUART10_RX */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_34_LPUART10_CTS_B,       /* GPIO_AD_34 is configured as LPUART10_CTS_B */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_35_LPUART10_RTS_B,       /* GPIO_AD_35 is configured as LPUART10_RTS_B */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_32_LPUART10_TX,          /* GPIO_AD_32 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_33_LPUART10_RX,          /* GPIO_AD_33 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_34_LPUART10_CTS_B,       /* GPIO_AD_34 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_AD_35_LPUART10_RTS_B,       /* GPIO_AD_35 PAD functional properties : */
      0x02U);                                 /* Slew Rate Field: Fast Slew Rate
                                                 Drive Strength Field: high driver
                                                 Pull / Keep Select Field: Pull Disable, Highz
                                                 Pull Up / Down Config. Field: Weak pull down
                                                 Open Drain Field: Disabled
                                                 Force ibe off Field: Disabled */
}


/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPinsM2:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: T17, peripheral: RGPIO4, signal: 'gpio_io, 07', pin_signal: GPIO_AD_07, identifier: SDIO_RST, direction: OUTPUT}
  - {pin_num: M17, peripheral: RGPIO4, signal: 'gpio_io, 17', pin_signal: GPIO_AD_17, direction: OUTPUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPinsM2, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPinsM2(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc1);          /* Turn on LPCG: LPCG is ON. */

  /* GPIO configuration of SDIO_RST on GPIO_AD_07 (pin T17) */
  rgpio_pin_config_t SDIO_RST_config = {
      .pinDirection = kRGPIO_DigitalOutput,
      .outputLogic = 0U,
  };
  /* Initialize GPIO functionality on GPIO_AD_07 (pin T17) */
  RGPIO_PinInit(RGPIO4, 7U, &SDIO_RST_config);

  /* GPIO configuration of WL_RST on GPIO_AD_17 (pin M17) */
  rgpio_pin_config_t WL_RST_config = {
      .pinDirection = kRGPIO_DigitalOutput,
      .outputLogic = 0U,
  };
  /* Initialize GPIO functionality on GPIO_AD_17 (pin M17) */
  RGPIO_PinInit(RGPIO4, 17U, &WL_RST_config);

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_07_GPIO4_IO07,           /* GPIO_AD_07 is configured as GPIO4_IO07 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_17_GPIO4_IO17,           /* GPIO_AD_17 is configured as GPIO4_IO17 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
}


/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitUSDHCPins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: D15, peripheral: USDHC1, signal: usdhc_clk, pin_signal: GPIO_SD_B1_01, software_input_on: Enable, pull_down_pull_up_config: No_Pull, pdrv_config: High_Driver,
    open_drain: Disable}
  - {pin_num: B16, peripheral: USDHC1, signal: usdhc_cmd, pin_signal: GPIO_SD_B1_00, software_input_on: Enable, pull_down_pull_up_config: Pull_Up}
  - {pin_num: D14, peripheral: USDHC1, signal: 'usdhc_data, 0', pin_signal: GPIO_SD_B1_02, software_input_on: Enable, pdrv_config: High_Driver}
  - {pin_num: C15, peripheral: USDHC1, signal: 'usdhc_data, 1', pin_signal: GPIO_SD_B1_03, software_input_on: Enable, pdrv_config: High_Driver}
  - {pin_num: B15, peripheral: USDHC1, signal: 'usdhc_data, 2', pin_signal: GPIO_SD_B1_04, software_input_on: Enable, pdrv_config: High_Driver}
  - {pin_num: A16, peripheral: USDHC1, signal: 'usdhc_data, 3', pin_signal: GPIO_SD_B1_05, software_input_on: Enable, pdrv_config: High_Driver}
  - {pin_num: N16, peripheral: RGPIO4, signal: 'gpio_io, 15', pin_signal: GPIO_AD_15, direction: OUTPUT, software_input_on: Disable, pull_up_down_config: no_init}
  - {pin_num: N14, peripheral: RGPIO4, signal: 'gpio_io, 14', pin_signal: GPIO_AD_14, direction: OUTPUT, pull_up_down_config: no_init}
  - {pin_num: L15, peripheral: RGPIO4, signal: 'gpio_io, 29', pin_signal: GPIO_AD_29, direction: OUTPUT, pull_up_down_config: no_init}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitUSDHCPins, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitUSDHCPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc1);          /* Turn on LPCG: LPCG is ON. */

  /* GPIO configuration of SD_PWREN_B on GPIO_AD_14 (pin N14) */
  rgpio_pin_config_t SD_PWREN_B_config = {
      .pinDirection = kRGPIO_DigitalOutput,
      .outputLogic = 0U,
  };
  /* Initialize GPIO functionality on GPIO_AD_14 (pin N14) */
  RGPIO_PinInit(RGPIO4, 14U, &SD_PWREN_B_config);

  /* GPIO configuration of SD1_CD_B on GPIO_AD_15 (pin N16) */
  rgpio_pin_config_t SD1_CD_B_config = {
      .pinDirection = kRGPIO_DigitalOutput,
      .outputLogic = 0U,
  };
  /* Initialize GPIO functionality on GPIO_AD_15 (pin N16) */
  RGPIO_PinInit(RGPIO4, 15U, &SD1_CD_B_config);

  /* GPIO configuration of SD1_VSELECT on GPIO_AD_29 (pin L15) */
  rgpio_pin_config_t SD1_VSELECT_config = {
      .pinDirection = kRGPIO_DigitalOutput,
      .outputLogic = 0U,
  };
  /* Initialize GPIO functionality on GPIO_AD_29 (pin L15) */
  RGPIO_PinInit(RGPIO4, 29U, &SD1_VSELECT_config);

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_14_GPIO4_IO14,           /* GPIO_AD_14 is configured as GPIO4_IO14 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_15_GPIO4_IO15,           /* GPIO_AD_15 is configured as GPIO4_IO15 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_AD_29_GPIO4_IO29,           /* GPIO_AD_29 is configured as GPIO4_IO29 */
      0U);                                    /* Software Input On Field: Input Path is determined by functionality */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_B1_00_USDHC1_CMD,        /* GPIO_SD_B1_00 is configured as USDHC1_CMD */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_B1_00 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_B1_01_USDHC1_CLK,        /* GPIO_SD_B1_01 is configured as USDHC1_CLK */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_B1_01 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_B1_02_USDHC1_DATA0,      /* GPIO_SD_B1_02 is configured as USDHC1_DATA0 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_B1_02 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_B1_03_USDHC1_DATA1,      /* GPIO_SD_B1_03 is configured as USDHC1_DATA1 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_B1_03 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_B1_04_USDHC1_DATA2,      /* GPIO_SD_B1_04 is configured as USDHC1_DATA2 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_B1_04 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3,      /* GPIO_SD_B1_05 is configured as USDHC1_DATA3 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_SD_B1_05 */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_B1_00_USDHC1_CMD,        /* GPIO_SD_B1_00 PAD functional properties : */
      0x04U);                                 /* PDRV Field: high driver
                                                 Pull Down Pull Up Field: PU
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_B1_01_USDHC1_CLK,        /* GPIO_SD_B1_01 PAD functional properties : */
      0x0CU);                                 /* PDRV Field: high driver
                                                 Pull Down Pull Up Field: No Pull
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_B1_02_USDHC1_DATA0,      /* GPIO_SD_B1_02 PAD functional properties : */
      0x04U);                                 /* PDRV Field: high driver
                                                 Pull Down Pull Up Field: PU
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_B1_03_USDHC1_DATA1,      /* GPIO_SD_B1_03 PAD functional properties : */
      0x04U);                                 /* PDRV Field: high driver
                                                 Pull Down Pull Up Field: PU
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_B1_04_USDHC1_DATA2,      /* GPIO_SD_B1_04 PAD functional properties : */
      0x04U);                                 /* PDRV Field: high driver
                                                 Pull Down Pull Up Field: PU
                                                 Open Drain Field: Disabled */
  IOMUXC_SetPinConfig(
      IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3,      /* GPIO_SD_B1_05 PAD functional properties : */
      0x04U);                                 /* PDRV Field: high driver
                                                 Pull Down Pull Up Field: PU
                                                 Open Drain Field: Disabled */
}


/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitSPIPins:
- options: {callFromInitBoot: 'true', coreID: cm33, enableClock: 'true'}
- pin_list:
  - {pin_num: A10, peripheral: FLEXSPI1, signal: FLEXSPI_A_DATA0, pin_signal: GPIO_B2_10, software_input_on: Enable}
  - {pin_num: B9, peripheral: FLEXSPI1, signal: FLEXSPI_A_DATA1, pin_signal: GPIO_B2_11, software_input_on: Enable}
  - {pin_num: A8, peripheral: FLEXSPI1, signal: FLEXSPI_A_DATA2, pin_signal: GPIO_B2_12, software_input_on: Enable}
  - {pin_num: B8, peripheral: FLEXSPI1, signal: FLEXSPI_A_DATA3, pin_signal: GPIO_B2_13, software_input_on: Enable}
  - {pin_num: A6, peripheral: FLEXSPI1, signal: FLEXSPI_A_DQS, pin_signal: GPIO_B2_07, software_input_on: Enable}
  - {pin_num: A7, peripheral: FLEXSPI1, signal: FLEXSPI_A_SCLK, pin_signal: GPIO_B2_08, software_input_on: Enable}
  - {pin_num: D10, peripheral: FLEXSPI1, signal: FLEXSPI_A_SS0_B, pin_signal: GPIO_B2_09, software_input_on: Enable}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitSPIPins, assigned for the Cortex-M33 core.
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitSPIPins(void) {
  CLOCK_EnableClock(kCLOCK_Iomuxc1);          /* Turn on LPCG: LPCG is ON. */

  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_07_FLEXSPI1_BUS2BIT_A_DQS,  /* GPIO_B2_07 is configured as FLEXSPI1_BUS2BIT_A_DQS */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_07 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_08_FLEXSPI1_BUS2BIT_A_SCLK,  /* GPIO_B2_08 is configured as FLEXSPI1_BUS2BIT_A_SCLK */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_08 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_09_FLEXSPI1_BUS2BIT_A_SS0_B,  /* GPIO_B2_09 is configured as FLEXSPI1_BUS2BIT_A_SS0_B */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_09 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_10_FLEXSPI1_BUS2BIT_A_DATA00,  /* GPIO_B2_10 is configured as FLEXSPI1_BUS2BIT_A_DATA00 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_10 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_11_FLEXSPI1_BUS2BIT_A_DATA01,  /* GPIO_B2_11 is configured as FLEXSPI1_BUS2BIT_A_DATA01 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_11 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_12_FLEXSPI1_BUS2BIT_A_DATA02,  /* GPIO_B2_12 is configured as FLEXSPI1_BUS2BIT_A_DATA02 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_12 */
  IOMUXC_SetPinMux(
      IOMUXC_GPIO_B2_13_FLEXSPI1_BUS2BIT_A_DATA03,  /* GPIO_B2_13 is configured as FLEXSPI1_BUS2BIT_A_DATA03 */
      1U);                                    /* Software Input On Field: Force input path of pad GPIO_B2_13 */
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
