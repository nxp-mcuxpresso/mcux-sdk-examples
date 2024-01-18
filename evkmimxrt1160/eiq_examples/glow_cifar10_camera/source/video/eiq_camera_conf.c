/*
 * Copyright 2019-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eiq_camera_conf.h"
#include "fsl_gpio.h"
#include "fsl_csi.h"
#include "fsl_csi_camera_adapter.h"
#include "fsl_ov5640.h"
#include "fsl_mipi_csi2rx.h"
#include "board.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_CSI_CLK_FREQ (CLOCK_GetFreqFromObs(CCM_OBS_BUS_CLK_ROOT))
#define DEMO_MIPI_CSI2_UI_CLK_FREQ (CLOCK_GetFreqFromObs(CCM_OBS_CSI2_UI_CLK_ROOT))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Resets camera power down pin.
 * 
 * @param pullUp sets true pin
 */
static void BOARD_PullCameraPowerDownPin(bool pullUp);

/*!
 * @brief Resets camera using reset pin.
 * 
 * @param pullUp sets true pin
 */
static void BOARD_PullCameraResetPin(bool pullUp);

/*!
 * @brief Verifies camera clock source.
 * 
 * @return status code
 */
static status_t BOARD_VerifyCameraClockSource(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Camera connect to CSI. */
static csi_resource_t csiResource = {
    .csiBase = CSI,
    .dataBus = kCSI_DataBus24Bit,
};

static csi_private_data_t csiPrivateData;

camera_receiver_handle_t cameraReceiver = {
    .resource    = &csiResource,
    .ops         = &csi_ops,
    .privateData = &csiPrivateData,
};

static ov5640_resource_t ov5640Resource = {
    .i2cSendFunc      = BOARD_Camera_I2C_SendSCCB,
    .i2cReceiveFunc   = BOARD_Camera_I2C_ReceiveSCCB,
    .pullResetPin     = BOARD_PullCameraResetPin,
    .pullPowerDownPin = BOARD_PullCameraPowerDownPin,
};

camera_device_handle_t cameraDevice = {
    .resource = &ov5640Resource,
    .ops      = &ov5640_ops,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

extern void CSI_DriverIRQHandler(void);

/*!
 * @brief Handles CSI IRQ.
 */
void CSI_IRQHandler(void)
{
  CSI_DriverIRQHandler();
  __DSB();
}

/*!
 * @brief Prepares camera for initialization.
 * 
 * @return status code
 */
void BOARD_EarlyPrepareCamera(void)
{
  /* If the camera I2C bus should be released by sending I2C sequence,
   * add the code here.
   */
}

static void BOARD_PullCameraResetPin(bool pullUp)
{
  if (pullUp)
  {
    GPIO_PinWrite(BOARD_CAMERA_RST_GPIO, BOARD_CAMERA_RST_PIN, 1);
  }
  else
  {
    GPIO_PinWrite(BOARD_CAMERA_RST_GPIO, BOARD_CAMERA_RST_PIN, 0);
  }
}

static void BOARD_PullCameraPowerDownPin(bool pullUp)
{
  if (pullUp)
  {
    GPIO_PinWrite(BOARD_CAMERA_PWDN_GPIO, BOARD_CAMERA_PWDN_PIN, 1);
  }
  else
  {
    GPIO_PinWrite(BOARD_CAMERA_PWDN_GPIO, BOARD_CAMERA_PWDN_PIN, 0);
  }
}

/*!
 * @brief Prepares camera for initialization.
 */
void BOARD_EarlyInitCamera(void)
{
  /* If the camera I2C bus should be released by sending I2C sequence,
   * add the code here.
   */
}

/*!
 * @brief Initializes camera controler.
 */
void BOARD_InitCameraResource(void)
{
  BOARD_Camera_I2C_Init();

  /* CSI MCLK is connect to dedicated 24M OSC, so don't need to configure it. */
}

/*!
 * @brief Initializes Mipi Csi Clock.
 */
void BOARD_InitMipiCsi(void)
{
  csi2rx_config_t csi2rxConfig = {0};

  /* This clock should be equal or faster than the receive byte clock,
   * D0_HS_BYTE_CLKD, from the RX DPHY. For this board, there are two
   * data lanes, the MIPI CSI pixel format is 16-bit per pixel, the
   * max resolution supported is 720*1280@30Hz, so the MIPI CSI2 clock
   * should be faster than 720*1280*30 = 27.6MHz, choose 60MHz here.
   */
  const clock_root_config_t csi2ClockConfig = {
    .clockOff = false,
    .mux      = 5,
    .div      = 7,
  };

  /* ESC clock should be in the range of 60~80 MHz */
  const clock_root_config_t csi2EscClockConfig = {
    .clockOff = false,
    .mux      = 5,
    .div      = 7,
  };

  /* UI clock should be equal or faster than the input pixel clock.
   * The camera max resolution supported is 720*1280@30Hz, so this clock
   * should be faster than 720*1280*30 = 27.6MHz, choose 60MHz here.
   */
  const clock_root_config_t csi2UiClockConfig = {
    .clockOff = false,
    .mux      = 5,
    .div      = 7,
  };

  if (kStatus_Success != BOARD_VerifyCameraClockSource())
  {
    PRINTF("MIPI CSI clock source not valid\r\n");
    while (1)
    {
    }
  }

  /* MIPI CSI2 connect to CSI. */
  CLOCK_EnableClock(kCLOCK_Video_Mux);
  VIDEO_MUX->VID_MUX_CTRL.SET = (VIDEO_MUX_VID_MUX_CTRL_CSI_SEL_MASK);

  CLOCK_SetRootClock(kCLOCK_Root_Csi2, &csi2ClockConfig);
  CLOCK_SetRootClock(kCLOCK_Root_Csi2_Esc, &csi2EscClockConfig);
  CLOCK_SetRootClock(kCLOCK_Root_Csi2_Ui, &csi2UiClockConfig);

  /* The CSI clock should be faster than MIPI CSI2 clk_ui. The CSI clock
   * is bus clock.
   */
  if (DEMO_CSI_CLK_FREQ < DEMO_MIPI_CSI2_UI_CLK_FREQ)
  {
    PRINTF("CSI clock should be faster than MIPI CSI2 ui clock.\r\n");
    while (1)
    {
    }
  }

  /*
   * Initialize the MIPI CSI2
   *
   * From D-PHY specification, the T-HSSETTLE should in the range of 85ns+6*UI to 145ns+10*UI
   * UI is Unit Interval, equal to the duration of any HS state on the Clock Lane
   *
   * T-HSSETTLE = csi2rxConfig.tHsSettle_EscClk * (Tperiod of RxClkInEsc)
   *
   * csi2rxConfig.tHsSettle_EscClk setting for camera:
   *
   *    Resolution  |  frame rate  |  T_HS_SETTLE
   *  =============================================
   *     720P       |     30       |     0x12
   *  ---------------------------------------------
   *     720P       |     15       |     0x17
   *  ---------------------------------------------
   *      VGA       |     30       |     0x1F
   *  ---------------------------------------------
   *      VGA       |     15       |     0x24
   *  ---------------------------------------------
   *     QVGA       |     30       |     0x1F
   *  ---------------------------------------------
   *     QVGA       |     15       |     0x24
   *  ---------------------------------------------
   */
  static const uint32_t csi2rxHsSettle[][3] = {
    {
        kVIDEO_Resolution720P,
        30,
        0x12,
    },
    {
        kVIDEO_Resolution720P,
        15,
        0x17,
    },
    {
        kVIDEO_ResolutionVGA,
        30,
        0x1F,
    },
    {
        kVIDEO_ResolutionVGA,
        15,
        0x24,
    },
    {
        kVIDEO_ResolutionQVGA,
        30,
        0x1F,
    },
    {
        kVIDEO_ResolutionQVGA,
        15,
        0x24,
    },
  };

  csi2rxConfig.laneNum          = DEMO_CAMERA_MIPI_CSI_LANE;
  csi2rxConfig.tHsSettle_EscClk = 0x12;

  for (uint8_t i = 0; i < ARRAY_SIZE(csi2rxHsSettle); i++)
  {
    if ((FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT) == csi2rxHsSettle[i][0]) &&
        (csi2rxHsSettle[i][1] == DEMO_CAMERA_FRAME_RATE))
    {
        csi2rxConfig.tHsSettle_EscClk = csi2rxHsSettle[i][2];
        break;
    }
  }

  CSI2RX_Init(MIPI_CSI2RX, &csi2rxConfig);
}

/*!
 * @brief Verifies camera clock source.
 * 
 * @return status code
 */
static status_t BOARD_VerifyCameraClockSource(void)
{
  status_t status;
  uint32_t srcClkFreq;
  /*
   * The MIPI CSI clk_ui, clk_esc, and core_clk are all from
   * System PLL3 (PLL_480M). Verify the clock source to ensure
   * it is ready to use.
   */
  srcClkFreq = CLOCK_GetPllFreq(kCLOCK_PllSys3);

  if (480 != (srcClkFreq / 1000000))
  {
    status = kStatus_Fail;
  }
  else
  {
    status = kStatus_Success;
  }

  return status;
}
