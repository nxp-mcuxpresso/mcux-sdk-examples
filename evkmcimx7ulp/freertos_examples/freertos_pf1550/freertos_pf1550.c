/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c_freertos.h"
#include "fsl_pf1550.h"

#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PF1550_LPI2C_BASE        LPI2C3
#define DEMO_PF1550_LPI2C_IRQ         LPI2C3_IRQn
#define DEMO_PF1550_LPI2C_PRIO        (5U)
#define DEMO_PF1550_LPI2C_CLKSRC_FREQ CLOCK_GetIpFreq(kCLOCK_Lpi2c3)
#define DEMO_PF1550_LPI2C_BAUDRATE    (400000U)
#define DEMO_PF1550_LPI2C_SCL_GPIO    GPIOB
#define DEMO_PF1550_LPI2C_SCL_PIN     (12U)
#define DEMO_PF1550_LPI2C_SDA_GPIO    GPIOB
#define DEMO_PF1550_LPI2C_SDA_PIN     (13U)
#define DEMO_PF1550_LPI2C_DELAY       (100U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
status_t I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static pf1550_handle_t pf1550Handle;
static lpi2c_rtos_handle_t lpi2c_rtos_handle;
static uint8_t buffer[256U];

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_I2C_ReleaseBusDelay(void)
{
    uint32_t i = 0U;
    for (i = 0U; i < DEMO_PF1550_LPI2C_DELAY; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0U;
    gpio_pin_config_t pin_config;

    /* Initialize PTB12/PTB13 as GPIO */
    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic  = 1U;
    IOMUXC_SetPinMux(IOMUXC_PTB12_PTB12, 0U);
    IOMUXC_SetPinMux(IOMUXC_PTB13_PTB13, 0U);
    IOMUXC_SetPinConfig(IOMUXC_PTB12_PTB12, IOMUXC0_SW_MUX_CTL_PAD_OBE_MASK);
    IOMUXC_SetPinConfig(IOMUXC_PTB13_PTB13, IOMUXC0_SW_MUX_CTL_PAD_OBE_MASK);
    CLOCK_EnableClock(kCLOCK_Rgpio2p0);
    GPIO_PinInit(DEMO_PF1550_LPI2C_SCL_GPIO, DEMO_PF1550_LPI2C_SCL_PIN, &pin_config);
    GPIO_PinInit(DEMO_PF1550_LPI2C_SDA_GPIO, DEMO_PF1550_LPI2C_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(DEMO_PF1550_LPI2C_SDA_GPIO, DEMO_PF1550_LPI2C_SDA_PIN, 0U);
    BOARD_I2C_ReleaseBusDelay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0U; i < 9U; i++)
    {
        GPIO_PinWrite(DEMO_PF1550_LPI2C_SCL_GPIO, DEMO_PF1550_LPI2C_SCL_PIN, 0U);
        BOARD_I2C_ReleaseBusDelay();

        GPIO_PinWrite(DEMO_PF1550_LPI2C_SDA_GPIO, DEMO_PF1550_LPI2C_SDA_PIN, 1U);
        BOARD_I2C_ReleaseBusDelay();

        GPIO_PinWrite(DEMO_PF1550_LPI2C_SCL_GPIO, DEMO_PF1550_LPI2C_SCL_PIN, 1U);
        BOARD_I2C_ReleaseBusDelay();
        BOARD_I2C_ReleaseBusDelay();
    }

    /* Send stop */
    GPIO_PinWrite(DEMO_PF1550_LPI2C_SCL_GPIO, DEMO_PF1550_LPI2C_SCL_PIN, 0U);
    BOARD_I2C_ReleaseBusDelay();

    GPIO_PinWrite(DEMO_PF1550_LPI2C_SDA_GPIO, DEMO_PF1550_LPI2C_SDA_PIN, 0U);
    BOARD_I2C_ReleaseBusDelay();

    GPIO_PinWrite(DEMO_PF1550_LPI2C_SCL_GPIO, DEMO_PF1550_LPI2C_SCL_PIN, 1U);
    BOARD_I2C_ReleaseBusDelay();

    GPIO_PinWrite(DEMO_PF1550_LPI2C_SDA_GPIO, DEMO_PF1550_LPI2C_SDA_PIN, 1U);
    BOARD_I2C_ReleaseBusDelay();
}

static uint32_t DEMO_GetDemoToRun(void)
{
    uint8_t demoSel;

    PRINTF("\r\n-------------- PF1550 on board PMIC RTOS driver example --------------\r\n");
    PRINTF("\r\nPlease select the PMIC example you want to run:\r\n");
    PRINTF("[1]. Setting Regulator Output Voltage\r\n");
    PRINTF("[2]. Dumping Regulator Output Voltage\r\n");
    PRINTF("[3]. Dumping Selected Register Content\r\n");

    while (1)
    {
        demoSel = GETCHAR();
        if ((demoSel >= '1') && (demoSel <= '3'))
        {
            break;
        }
    }

    return demoSel;
}

static uint32_t DEMO_GetInputNumWithEcho(uint32_t length, bool allowZero)
{
    uint8_t ch;
    uint8_t digBuffer[10U];
    uint8_t i, j;
    uint8_t digCount = 0U;
    uint32_t temp1, temp2;
    uint32_t result  = 0U;
    bool getFirstDig = false;

    assert(length <= (sizeof(digBuffer) / sizeof(digBuffer[0U])));

    /* Get user input and echo it back to terminal. */
    for (;;)
    {
        ch = GETCHAR();
        if ((('0' <= ch) && ('9' >= ch)) && (digCount < length))
        {
            if (false == getFirstDig)
            {
                if (allowZero || (('0' < ch) && ('9' >= ch)))
                {
                    getFirstDig = true;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if ('0' == digBuffer[0U])
                {
                    continue;
                }
            }
            PUTCHAR(ch);
            digBuffer[digCount] = ch;
            digCount++;
        }

        if ('\r' == ch)
        {
            break;
        }
    }

    /* Reconstruct user input number. */
    temp1 = digCount - 1;
    for (i = 0; i < digCount; i++)
    {
        temp2 = digBuffer[i] - '0';
        for (j = 0U; j < temp1; j++)
        {
            temp2 *= 10U;
        }
        temp1--;
        result += temp2;
    }

    return result;
}

static void DEMO_SetRegulatorOutput(void)
{
    uint8_t regulatorSel;
    uint32_t outputVoltage = 0;
    pf1550_module_t module;

    for (;;)
    {
        PRINTF("\f");
        PRINTF("\r\n-------------- Setting Regulator Output Voltage  --------------\r\n");
        PRINTF("Please select which regulator to set:\r\n");
        PRINTF("[a]. Buck Switch 1\r\n");
        PRINTF("[b]. Buck Switch 2\r\n");
        PRINTF("[c]. Buck Switch 3\r\n");
        PRINTF("[d]. LDO 1\r\n");
        PRINTF("[e]. LDO 2\r\n");
        PRINTF("[f]. LDO 3\r\n");
        PRINTF("[g]. Back to Home Page\r\n");

        for (;;)
        {
            regulatorSel = GETCHAR();
            if ((regulatorSel >= 'a') && (regulatorSel <= 'g'))
            {
                break;
            }
        }

        if ('g' != regulatorSel)
        {
            switch (regulatorSel)
            {
                case 'a':
                    if (PF1550_IsRegulatorSupportDvs(&pf1550Handle, kPF1550_ModuleSwitch1))
                    {
                        module = kPF1550_ModuleSwitch1;
                    }
                    else
                    {
                        PRINTF("\f");
                        PRINTF("%s does not support DVS!!!\r\n", "Switch1");
                        PRINTF("Press Any Key to Continue...\r\n");
                        GETCHAR();
                        PRINTF("\f");
                        continue;
                    }
                    break;

                case 'b':
                    if (PF1550_IsRegulatorSupportDvs(&pf1550Handle, kPF1550_ModuleSwitch2))
                    {
                        module = kPF1550_ModuleSwitch2;
                    }
                    else
                    {
                        PRINTF("\f");
                        PRINTF("%s does not support DVS!!!\r\n", "Switch2");
                        PRINTF("Press Any Key to Continue...\r\n");
                        GETCHAR();
                        PRINTF("\f");
                        continue;
                    }
                    break;

                case 'c':
                    PRINTF("\f");
                    PRINTF("%s does not support DVS!!!\r\n", "Switch3");
                    PRINTF("Press Any Key to Continue...\r\n");
                    GETCHAR();
                    PRINTF("\f");
                    continue;

                case 'd':
                    module = kPF1550_ModuleLdo1;
                    break;

                case 'e':
                    module = kPF1550_ModuleLdo2;
                    break;

                default:
                    module = kPF1550_ModuleLdo3;
                    break;
            }

            PRINTF("\f");
            PRINTF("Please input the regulator output(mV):");
            outputVoltage = DEMO_GetInputNumWithEcho(4U, false);
            PRINTF("\r\n");

            PRINTF("User desired regulator output:%dmV\r\n", outputVoltage);
            PRINTF("Please confirm the desired output will not damage to the hardware!!!\r\n");
            PRINTF("Press any key to update regulator output...\r\n");
            GETCHAR();

            /* Convert user input unit to uV. */
            outputVoltage *= 1000U;
        }
        else
        {
            break;
        }

        PF1550_SetRegulatorOutputVoltage(&pf1550Handle, module, PF1550_GetOperatingStatus(&pf1550Handle),
                                         outputVoltage);
        outputVoltage =
            PF1550_GetRegulatorOutputVoltage(&pf1550Handle, module, PF1550_GetOperatingStatus(&pf1550Handle));

        /* Convert actual output unit to mV. */
        outputVoltage /= 1000U;
        PRINTF("Actual Regulator Output:%dmV\r\n", outputVoltage);
        PRINTF("Press Any Key to Continue...\r\n");
        GETCHAR();
        PRINTF("\f");
    }
}

static void DEMO_PrintRegulatorInfo(const char *regulatorName, pf1550_module_t module)
{
    pf1550_operating_status_t status;

    PRINTF("[Name:%s]", regulatorName);
    status = PF1550_GetOperatingStatus(&pf1550Handle);
    switch (status)
    {
        case kPF1550_OperatingStatusCoreOff:
            PRINTF("[Status:CoreOff]");
            break;

        case kPF1550_OperatingStatusShipMode:
            PRINTF("[Status:ShipMode]");
            break;

        case kPF1550_OperatingStatusRegsDisable:
            PRINTF("[Status:RegsDisable]");
            break;

        case kPF1550_OperatingStatusRun:
            PRINTF("[Status:Run]");
            break;

        case kPF1550_OperatingStatusStandby:
            PRINTF("[Status:Standby]");
            break;

        case kPF1550_OperatingStatusSleep:
            PRINTF("[Status:Sleep]");
            break;

        default:
            break;
    }

    PRINTF("[Enable:%s][Voltage:%dmV]\r\n", PF1550_IsRegulatorEnabled(&pf1550Handle, module, status) ? "Yes" : "No",
           PF1550_GetRegulatorOutputVoltage(&pf1550Handle, module, status) / 1000U);
}

static void DEMO_DumpRegulatorOutput(void)
{
    /* Scroll to a new page. */
    PRINTF("\f");
    PRINTF("\r\n-------------- Dumping PF1550 Regulator Output --------------\r\n");
    PRINTF("All the Regulator Output Parameters are listed here:\r\r\n\n");

    /* Print All Regulator Output Voltages. */
    DEMO_PrintRegulatorInfo("Switch1", kPF1550_ModuleSwitch1);
    DEMO_PrintRegulatorInfo("Switch2", kPF1550_ModuleSwitch2);
    DEMO_PrintRegulatorInfo("Switch3", kPF1550_ModuleSwitch3);
    DEMO_PrintRegulatorInfo(" LDO1  ", kPF1550_ModuleLdo1);
    DEMO_PrintRegulatorInfo(" LDO2  ", kPF1550_ModuleLdo2);
    DEMO_PrintRegulatorInfo(" LDO3  ", kPF1550_ModuleLdo3);
    DEMO_PrintRegulatorInfo(" Vsnvs ", kPF1550_ModuleVsnvs);
    PRINTF("\r\n");
}

static void DEMO_DumpRegisterContent(void)
{
    uint32_t address;
    uint8_t number;
    uint8_t i;

    for (;;)
    {
        /* Scroll to a new page. */
        PRINTF("\f");
        PRINTF("\r\n-------------- Dumping PF1550 Register Content --------------\r\n\r\n");
        PRINTF("Please select the first address to dump(0~255):");
        address = DEMO_GetInputNumWithEcho(3U, true);
        if (255U >= address)
        {
            break;
        }
    }
    PRINTF("\r\n");

    for (;;)
    {
        /* Scroll to a new page. */
        PRINTF("\f");
        PRINTF("Please select the number of registers to dump(1~256):");
        number = DEMO_GetInputNumWithEcho(3U, false);
        if ((256U > (address + number)) && (0U != number))
        {
            break;
        }
    }
    PRINTF("\r\n");

    /* Read register from PF1550 */
    PF1550_DumpReg(&pf1550Handle, address, buffer, number);

    PRINTF("\r\nDump Format: [Register Address] = Register Content:\r\n");
    for (i = 0; i < number; i++)
    {
        PRINTF("[0x%x] = 0x%x\r\n", address + i, buffer[i]);
    }
    PRINTF("\r\n");
}

/*! @brief User Application task */
void User_Application_Task(void *pvParameters)
{
    uint32_t demoSelection;
    pf1550_config_t pf1550Config;

    /* Initialize PF1550 */
    PF1550_GetDefaultConfig(&pf1550Config);
    pf1550Config.I2C_SendFunc    = I2C_SendFunc;
    pf1550Config.I2C_ReceiveFunc = I2C_ReceiveFunc;
    PF1550_Init(&pf1550Handle, &pf1550Config);

    for (;;)
    {
        demoSelection = DEMO_GetDemoToRun();

        switch (demoSelection)
        {
            /* Setting Regulator Output Voltage: */
            case '1':
                DEMO_SetRegulatorOutput();
                break;

            /* Dumping all Regulator Output Voltage: */
            case '2':
                DEMO_DumpRegulatorOutput();
                PRINTF("Press Any Key to Home Page...");
                GETCHAR();
                break;

            /* Dumping Selected Register Content: */
            case '3':
                DEMO_DumpRegisterContent();
                PRINTF("Press Any Key to Home Page...");
                GETCHAR();
                break;

            default:
                break;
        }

        /* Scroll to a new page. */
        PRINTF("\f");
    }
}

/*! @brief Main function */
int main(void)
{
    lpi2c_master_config_t lpi2cMasterConfig;

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_PctlB);
    CLOCK_EnableClock(kCLOCK_Rgpio2p0);
    CLOCK_SetIpSrc(kCLOCK_Lpi2c3, kCLOCK_IpSrcSircAsync);

    /*
     * lpi2cMasterConfig.debugEnable = false;
     * lpi2cMasterConfig.ignoreAck = false;
     * lpi2cMasterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * lpi2cMasterConfig.baudRate_Hz = 100000U;
     * lpi2cMasterConfig.busIdleTimeout_ns = 0;
     * lpi2cMasterConfig.pinLowTimeout_ns = 0;
     * lpi2cMasterConfig.sdaGlitchFilterWidth_ns = 0;
     * lpi2cMasterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&lpi2cMasterConfig);
    lpi2cMasterConfig.baudRate_Hz = DEMO_PF1550_LPI2C_BAUDRATE;
    /*  Set LPI2C Master Interrupt Priority. */
    NVIC_SetPriority(DEMO_PF1550_LPI2C_IRQ, DEMO_PF1550_LPI2C_PRIO);
    /* Initialize LPI2C RTOS driver. */
    LPI2C_RTOS_Init(&lpi2c_rtos_handle, DEMO_PF1550_LPI2C_BASE, &lpi2cMasterConfig, DEMO_PF1550_LPI2C_CLKSRC_FREQ);

    /* Create User Application Task. */
    xTaskCreate(User_Application_Task, "User Application Task", configMINIMAL_STACK_SIZE * 2U, NULL,
                tskIDLE_PRIORITY + 1U, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}

status_t I2C_SendFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    lpi2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = subAddress;
    masterXfer.subaddressSize = subAddressSize;
    masterXfer.data           = (void *)txBuff;
    masterXfer.dataSize       = txBuffSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Calling I2C Transfer API to start send. */
    return LPI2C_RTOS_Transfer(&lpi2c_rtos_handle, &masterXfer);
}

status_t I2C_ReceiveFunc(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    lpi2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = subAddress;
    masterXfer.subaddressSize = subAddressSize;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = rxBuffSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Calling I2C Transfer API to start receive. */
    return LPI2C_RTOS_Transfer(&lpi2c_rtos_handle, &masterXfer);
}
