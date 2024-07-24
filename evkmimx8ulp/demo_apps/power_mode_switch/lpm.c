/*
 * Copyright 2017-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "clock_config.h"
#include "board.h"
#include "fsl_lptmr.h"
#include "fsl_mu.h"
#include "fsl_rtd_cmc.h"
#include "fsl_upower.h"
#include "fsl_sentinel.h"

#include "lpm.h"
#include "fsl_debug_console.h"
#include "fsl_cache.h"
#include "app_srtm.h"
#include "fsl_flexspi.h"
#include "fsl_iomuxc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSTICK_SOURCE_CLOCK   (CLOCK_GetRtcOscFreq() / 32)
#define SYSTICK_TICKLESS_CLOCK (CLOCK_GetRtcOscFreq() / 32)

#define FLEXSPI_LUT_KEY_VAL (0x5AF05AF0UL)
#define CUSTOM_LUT_LENGTH   (64U)

void SysTick_Handler(void);

/* Define the count per tick of the systick(LPTMR) in run mode. For accuracy purpose,
 * please make SYSTICK_SOURCE_CLOCK times of configTICK_RATE_HZ.
 */
#define SYSTICK_COUNT_PER_TICK (SYSTICK_SOURCE_CLOCK / configTICK_RATE_HZ)

extern cgc_fro_config_t g_cgcFroConfig;
extern cgc_rtd_sys_clk_config_t g_sysClkConfigFroSource;
extern cgc_sosc_config_t g_cgcSysOscConfig;
extern lpm_ad_power_mode_e AD_CurrentMode;

struct _lpm_power_mode_listener
{
    lpm_power_mode_callback_t callback;
    void *data;
    struct _lpm_power_mode_listener *next;
};

typedef struct _lpm_power_mode_listener lpm_power_mode_listener_t;

typedef struct _lpm_nvic_context
{
    uint32_t PriorityGroup;
    uint32_t ISER[8];
    uint8_t IPR[240]; /* ULP CM33 max IRQn is 191 */
    uint8_t SHPR[12];
    uint32_t ICSR;
    uint32_t VTOR;
    uint32_t AIRCR;
    uint32_t SCR;
    uint32_t CCR;
    uint32_t SHCSR;
    uint32_t MMFAR;
    uint32_t BFAR;
    uint32_t CPACR;
    uint32_t NSACR;

} lpm_nvic_context_t;

lpm_rtd_power_mode_e s_curMode;
lpm_rtd_power_mode_e s_lastMode;

/* Save latest address of __vecotr_table */
volatile uint32_t s_vector_table_addr;

/* Store Stack Pointer during RTD in Power Down */
volatile uint32_t s_psp;
volatile uint32_t s_msp;
volatile uint32_t s_control;

static SemaphoreHandle_t s_mutex;
#if configSUPPORT_STATIC_ALLOCATION
static StaticSemaphore_t s_staticMutex;
#endif
static lpm_power_mode_listener_t *s_listenerHead;
static lpm_power_mode_listener_t *s_listenerTail;

/* RTD Power Down Power mode */
static ps_rtd_pwr_mode_cfgs_t rtd_pwr_mode_cfgs = {
    /* PD */
    [PD_RTD_PWR_MODE] =
        {
            .in_reg_cfg = IN_REG_CFG(0x00000000, 0x00000000),
            .pmic_cfg   = PMIC_CFG(0x00000023, 0x00000000),
            .pad_cfg    = PAD_CFG(0x00000003, 0x00000000, 0x00000000),
            .mon_cfg    = MON_CFG(0x00000000, 0x0, 0x0),
            /*
             * bias mode: 0b00 - NBB, 0b01 - RBB, 0b10 - AFBB, 0b11 - ARBB
             * 0x0000 0001
             *   |  | |  |
             *   +--+ +--+
             *   |    |
             *   |    +--> RTD bias mode: RBB
             *   +--> LPAV bias mode: NBB
             *
             * RTD rbbn/rbbp: 0b00010 - 100 mV
             *           ...
             *           0b11010 - 1300 mV
             * LPAV rbbn/rbbp: 0b0 - 1000 mV, 0b1 - 1300 mV
             * 0x0001 001a
             *   |  | |  |
             *   +--+ +--+
             *   |    |
             *   |    +--> RTD rbbn: 0b11010 - 1300 mV
             *   +--> LPAV rbbn: 0b1 - 1300 mV
             *
             * LPAV rbbn/rbbp: 0b0 - 1000 mV, 0b1 - 1300 mV
             * 0x0001 001a
             *   |  | |  |
             *   +--+ +--+
             *   |    |
             *   |    +--> RTD rbbp: 0b11010 - 1300 mV
             *   +--> LPAV rbbp: 0b1 - 1300 mV
             */
            .bias_cfg       = BIAS_CFG(0x00000001, 0x0001001a, 0x0001001a, 0x00000001),
            .pwrsys_lpm_cfg = PWRSYS_LPM_CFG(0),
        },
    /* DPD */
    [DPD_RTD_PWR_MODE] =
        {
            /* Reuse power down mode's settings */
            .in_reg_cfg = IN_REG_CFG(
                0x00000000, 0x00000000), /* vol = 0[595.833mv], mode = 0x0 [regulator is off] internal regulator ? */
            .pmic_cfg = PMIC_CFG(0x00000023, 0x00000000),
            .pad_cfg  = PAD_CFG(0x00000003, 0x00000000, 0x00000000), /* PAD_CLOSE0(PTA) ~ PAD_CLOSE1(PTB) is isolated */
            .mon_cfg  = MON_CFG(0x00000000, 0x0, 0x0),
            .bias_cfg = BIAS_CFG(0x00000001, 0x0001001a, 0x0001001a, 0x00000001),
            .pwrsys_lpm_cfg = PWRSYS_LPM_CFG(0),
        },
    /* DSL */
    [DSL_RTD_PWR_MODE] =
        {
            .in_reg_cfg = IN_REG_CFG(0x1c, 0),
            .pmic_cfg   = PMIC_CFG(0x23, 0x0),
            // clang-format off
	    /*
	     * For PTA/PTB/PTE/PTF
	     * control                 pad state
	     * pad_reset   pad_close
	     * 0           0           Functional
	     * 0           1           State Retention
	     * 1           0/1         Weak0 Drive
	     *
	     * For PTC/PTD
	     * control                 pad state
	     * pad_tqsleep
	     * 0                       Functional
	     * 1                       Functional(Degradated Performance ?)
	     */
             /*
	      * PAD_CLOSE0(PTA) = 1, PAD_RESET0 = 0: PTA is in State Retention,[PTA/PTB/PTE/PTF are Fail Safe GPIO, PTC/PTD are High Speed GPIO, PTA/PTB/PTC is used by RTD, PTD/PTE/PTF are used by APD/LPAV]
	      * PAD_CLOSE1(PTB) = 1, PAD_RESET1 = 0: PTB is in State Retention.
	      * PAD_TQ_SLEEP0(PTC)
	      * PAD_TQ_SLEEP1(PTD)
	      * PAD_CLOSE2(PTE)
	      * PAD_CLOSE3(PTF)
	      */
            // clang-format on
            .pad_cfg        = PAD_CFG(0x3, 0x0, 0x0), /* PTA: State Retention, PTB: State Retention */
            .mon_cfg        = MON_CFG(0x0, 0x0, 0x0),
            .bias_cfg       = BIAS_CFG(0x00000001, 0x0001001a, 0x0001001a, 0x00000001),
            .pwrsys_lpm_cfg = PWRSYS_LPM_CFG(0),
        },
    /* SLP */
    [SLP_RTD_PWR_MODE] =
        {
            .in_reg_cfg     = IN_REG_CFG(0x1c, 0x3),
            .pmic_cfg       = PMIC_CFG(0x23, 0x901),
            .pad_cfg        = PAD_CFG(0x0, 0x0, 0x0),
            .mon_cfg        = MON_CFG(0x0deb3a00, 0x0, 0x0),
            .bias_cfg       = BIAS_CFG(0x00000000, 0x00000002, 0x00000002, 0x00000000),
            .pwrsys_lpm_cfg = PWRSYS_LPM_CFG(0),
        },
    /* ACT */
    [ACT_RTD_PWR_MODE] =
        {
            .in_reg_cfg     = IN_REG_CFG(0x0000001c, 0x3),
            .pmic_cfg       = PMIC_CFG(0x23, 0x00000000),
            .pad_cfg        = PAD_CFG(0x0, 0x0, 0x0),
            .mon_cfg        = MON_CFG(0x0, 0x0, 0x0),
            .bias_cfg       = BIAS_CFG(0x00020002, 0x00000002, 0x00000002, 0x0000000),
            .pwrsys_lpm_cfg = PWRSYS_LPM_CFG(0),
        },
};

static ps_rtd_swt_cfgs_t rtd_swt_cfgs = {
    /* PD */
    [PD_RTD_PWR_MODE] =
        {
            .swt_board[0] = SWT_BOARD(0x0, 0x00060003),
            .swt_mem[0]   = SWT_MEM(0x003fe000, 0x0, 0x003ff3ff),
            .swt_mem[1]   = SWT_MEM(0x00000000, 0x00000000, 0x00000000),
        },
    /* DPD */
    [DPD_RTD_PWR_MODE] =
        {
            /*
             * Reuse Power Down Mode's settings
             * on = 0x0 = 0b0000 0000 0000 0000 0000
             * mask = 0x60003 = 0b0110 0000 0000 0000 0011
             * switch close(on Power Switch):
             * switch open(off Power Switch): PS0(RTD A, RTD B), PS1(Fusion), PS17(Fusion AO),PS18(FUSE)
             */
            .swt_board[0] = SWT_BOARD(0x0, 0x00060003),
            .swt_mem[0]   = SWT_MEM(0x003fe000, 0x0, 0x003ff3ff),
            .swt_mem[1]   = SWT_MEM(0x00000000, 0x00000000, 0x00000000),
        },
    /* DSL */
    [DSL_RTD_PWR_MODE] =
        {
            .swt_board[0] = SWT_BOARD(0x1, 0x60003),
            .swt_mem[0]   = SWT_MEM(0x0, 0x0, 0xfffe0000),
            .swt_mem[1]   = SWT_MEM(0x003fffff, 0x003fffff, 0x003ff3ff),
        },
    /* SLP */
    [SLP_RTD_PWR_MODE] =
        {
            .swt_board[0] = SWT_BOARD(0x00060003, 0x0007ff83),
            .swt_mem[0]   = SWT_MEM(0x0, 0x0, 0xfffe0000),
            .swt_mem[1]   = SWT_MEM(0x003fffff, 0x003fffff, 0x003ff3ff),
        },
    /* ACT */
    [ACT_RTD_PWR_MODE] =
        {
            /*
             * RTD ACT settings
             * swt_board bit can refer into upower_ps_mask_t, power on PS0(RTD A, RTD B), PS1(Fusion), PS17(Fusion AO),PS18(FUSE).
             * swt_mem[1] refer into upower_mp1_mask_t, power on RTD domain periperal which are needed.
             * such as PS1(DMA0) to assure PDM edma adapter service is available after RTD resume from low power mode.
             */
            .swt_board[0] = SWT_BOARD(0x00060003, 0x0007ff83),
            .swt_mem[0]   = SWT_MEM(0x0, 0x0, 0x0),
            .swt_mem[1]   = SWT_MEM(0x003fff2a, 0x003fff2a, 0x003fffff),
        },
};

/* Deep sleep mode, reduce BUCK2OUT_DVS0 to 0.75V */
static ps_rtd_pmic_reg_data_cfgs_t rtd_pmic_reg_cfgs_dsl = {
    /* RTD DeepSleep: set BUCK2OUT_DVS0 to 0.75V */
    [0] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = DSL_RTD_PWR_MODE,
            .i2c_addr   = 0x15, /* BUCK2OUT_DVS0 of PCA9460 */
            .i2c_data   = 0x0C, /* 0.75 */
        },
    /* RTD Active: set BUCK2OUT_DVS0 to 1.0V */
    [1] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = ACT_RTD_PWR_MODE,
            .i2c_addr   = 0x15,
            .i2c_data   = 0x20,
        },
};

/* In order to reduce power consumption, poweroff LSW1, reduce BUCK2OUT_DVS0 to 0.65 V */
ps_rtd_pmic_reg_data_cfgs_t rtd_pmic_reg_cfgs_pd = {
    [0] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = PD_RTD_PWR_MODE,
            .i2c_addr   = 0x15, /* BUCK2OUT_DVS0 of PCA9460 */
            .i2c_data   = 0x04, /* 0.65 V */
        },
    /* RTD Power Down: off LSW1 */
    [1] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = PD_RTD_PWR_MODE,
            .i2c_addr   = 0x40, /* LSW1_CTRL of PCA9460 */
            .i2c_data   = 0x0,  /* LSW1_EN[1:0] = 00b(OFF) */
        },

    /* RTD Active: BUCK2OUT_DVS0 to 1.0 V  */
    [2] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = ACT_RTD_PWR_MODE,
            .i2c_addr   = 0x15, /* BUCK2OUT_DVS0 of PCA9460 */
            .i2c_data   = 0x20, /* 1.0 V */
        },
    /* RTD Active: on LSW1 */
    [3] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = ACT_RTD_PWR_MODE,
            .i2c_addr   = 0x40, /* LSW1_CTRL of PCA9460 */
            .i2c_data   = 0x11, /* LSW1_EN[1:0] = 01b(ON at RUN state) */
        },
};

/* In order to reduce power consumption, poweroff LSW1, reduce BUCK2OUT_DVS0 to 0.65 V */
ps_rtd_pmic_reg_data_cfgs_t rtd_pmic_reg_cfgs_dpd = {
    /* RTD Deep Power Down: BUCK2OUT_DVS0 to 0.65 V */
    [0] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = DPD_RTD_PWR_MODE,
            .i2c_addr   = 0x15, /* BUCK2OUT_DVS0 of PCA9460 */
            .i2c_data   = 0x04, /* 0.65 V */
        },
    /* RTD Deep Power Down: off LSW1 */
    [1] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = DPD_RTD_PWR_MODE,
            .i2c_addr   = 0x40, /* LSW1_CTRL of PCA9460 */
            .i2c_data   = 0x0,  /* LSW1_EN[1:0] = 00b(OFF) */
        },
    /* RTD Active: BUCK2OUT_DVS0 to 1.0 V  */
    [2] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = ACT_RTD_PWR_MODE,
            .i2c_addr   = 0x15, /* BUCK2OUT_DVS0 of PCA9460 */
            .i2c_data   = 0x20, /* 1.0 V */
        },
    /* RTD Active: on LSW1 */
    [3] =
        {
            .tag        = PMIC_REG_VALID_TAG,
            .power_mode = ACT_RTD_PWR_MODE,
            .i2c_addr   = 0x40, /* LSW1_CTRL of PCA9460 */
            .i2c_data   = 0x11, /* LSW1_EN[1:0] = 01b(ON at RUN state) */
        },
};

static lpm_nvic_context_t s_nvicContext;

/*
   Make sure the size of context buffer is not less
   than the register data plan to preserve
*/
static uint32_t s_iomuxc0Context[0xD6];
static uint32_t s_iomuxc0ContextIndex = 0;

static uint32_t s_rgpiobContext[0x7];
static uint32_t s_rgpiobContextIndex = 0;

static uint32_t s_pcc0Context[0x41];
static uint32_t s_pcc0ContextIndex = 0;

static uint32_t s_pcc1Context[0x14];
static uint32_t s_pcc1ContextIndex = 0;

static uint32_t s_flexspi0Context[0x72];
static uint32_t s_flexspi0ContextIndex = 0;

static uint32_t s_simsecContext[0x9];
static uint32_t s_simsecContextIndex = 0;

static uint32_t s_cgc0Context[1];
static uint32_t s_cgc0ContextIndex = 0;

uint32_t tmp_stack[0x100];

/* FreeRTOS implemented Systick handler. */
extern void xPortSysTickHandler(void);
extern void __vector_table(void);

bool LPM_Init(void)
{
#if configSUPPORT_STATIC_ALLOCATION
    s_mutex = xSemaphoreCreateMutexStatic(&s_staticMutex);
#else
    s_mutex = xSemaphoreCreateMutex();
#endif

    if (s_mutex == NULL)
    {
        return false;
    }

    s_listenerHead = s_listenerTail = NULL;
    s_curMode                       = LPM_PowerModeActive;

    return true;
}

void LPM_Deinit(void)
{
    if (s_mutex != NULL)
    {
        vSemaphoreDelete(s_mutex);
        s_mutex = NULL;
    }
}

bool LPM_SetPowerMode(lpm_rtd_power_mode_e mode)
{
    lpm_power_mode_listener_t *l1, *l2;
    bool ret = true;

    if (mode == s_curMode)
    {
        return ret;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    for (l1 = s_listenerHead; l1 != NULL; l1 = l1->next)
    {
        if (l1->callback == NULL)
        {
            continue;
        }

        if (!l1->callback(s_curMode, mode, l1->data))
        {
            /* One stakeholder doesn't allow new mode */
            ret = false;
            break;
        }
    }

    if (ret)
    {
        s_curMode = mode;
    }
    else
    {
        /* roll back the state change of previous listeners */
        for (l2 = s_listenerHead; l2 != l1; l2 = l2->next)
        {
            if (l2->callback == NULL)
            {
                continue;
            }

            l2->callback(mode, s_curMode, l2->data);
        }
    }

    xSemaphoreGive(s_mutex);

    return ret;
}

lpm_rtd_power_mode_e LPM_GetPowerMode(void)
{
    return s_curMode;
}

bool LPM_SystemSleep(void)
{
    /* Initialize upower config data */
    struct ps_pwr_mode_cfg_t *pwr_sys_cfg = (struct ps_pwr_mode_cfg_t *)UPWR_DRAM_SHARED_BASE_ADDR;

    /* Copy upower config data to target memory */
    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[SLP_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[SLP_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[SLP_RTD_PWR_MODE], &rtd_swt_cfgs[SLP_RTD_PWR_MODE], sizeof(swt_config_t));

    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[ACT_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[ACT_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[ACT_RTD_PWR_MODE], &rtd_swt_cfgs[ACT_RTD_PWR_MODE], sizeof(swt_config_t));

    for (uint32_t rtd_mode = 0; rtd_mode < NUM_RTD_PWR_MODES; rtd_mode++)
    {
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_board =
            (struct upwr_switch_board_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_board);
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_mem =
            (struct upwr_mem_switches_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_mem);
    }

    RTDCMC_SetPowerModeProtection(CMC_RTD, CMC_RTD_PMPROT_AS_MASK);
    RTDCMC_EnterLowPowerMode(CMC_RTD, kRTDCMC_SleepMode);

    return true;
}

bool LPM_SystemDeepSleep(void)
{
    /* Initialize upower config data */
    struct ps_pwr_mode_cfg_t *pwr_sys_cfg = (struct ps_pwr_mode_cfg_t *)UPWR_DRAM_SHARED_BASE_ADDR;

    /* Copy upower config data to target memory */
    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[DSL_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[DSL_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[DSL_RTD_PWR_MODE], &rtd_swt_cfgs[DSL_RTD_PWR_MODE], sizeof(swt_config_t));

    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[ACT_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[ACT_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[ACT_RTD_PWR_MODE], &rtd_swt_cfgs[ACT_RTD_PWR_MODE], sizeof(swt_config_t));

    memcpy(&pwr_sys_cfg->ps_rtd_pmic_reg_data_cfg, &rtd_pmic_reg_cfgs_dsl, sizeof(ps_rtd_pmic_reg_data_cfgs_t));

    for (uint32_t rtd_mode = 0; rtd_mode < NUM_RTD_PWR_MODES; rtd_mode++)
    {
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_board =
            (struct upwr_switch_board_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_board);
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_mem =
            (struct upwr_mem_switches_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_mem);
    }

    /*
     * Switch clock source from PLL0/1 to FRO to save more power during Power Down,
     * then switch back after wakeup in function BOARD_ResumeClockInit()
     */
    BOARD_SwitchToFROClk(BOARD_GetRtdDriveMode());
    BOARD_DisablePlls();

    RTDCMC_SetClockMode(CMC_RTD, kRTDCMC_GateAllSystemClocks);
    RTDCMC_SetPowerModeProtection(CMC_RTD, CMC_RTD_PMPROT_ADS_MASK);
    RTDCMC_EnableDebugOperation(CMC_RTD, false);
    RTDCMC_EnterLowPowerMode(CMC_RTD, kRTDCMC_DeepSleepMode);

    /* Clear clock mode after exiting from deep sleep mode */
    RTDCMC_SetClockMode(CMC_RTD, kRTDCMC_GateNoneClock);

    return true;
}

void LPM_NvicStateSave(void)
{
    uint32_t i;
    uint32_t irqRegs;
    uint32_t irqNum;

    irqRegs = (SCnSCB->ICTR & SCnSCB_ICTR_INTLINESNUM_Msk) + 1;
    irqNum  = irqRegs * 32;

    s_nvicContext.PriorityGroup = NVIC_GetPriorityGrouping();

    for (i = 0; i < irqRegs; i++)
    {
        s_nvicContext.ISER[i] = NVIC->ISER[i];
    }

    for (i = 0; i < irqNum; i++)
    {
        s_nvicContext.IPR[i] = NVIC->IPR[i];
    }

    s_nvicContext.SHPR[0]  = SCB->SHPR[0];  /* MemManage */
    s_nvicContext.SHPR[1]  = SCB->SHPR[1];  /* BusFault */
    s_nvicContext.SHPR[2]  = SCB->SHPR[2];  /* UsageFault */
    s_nvicContext.SHPR[7]  = SCB->SHPR[7];  /* SVCall */
    s_nvicContext.SHPR[8]  = SCB->SHPR[8];  /* DebugMonitor */
    s_nvicContext.SHPR[10] = SCB->SHPR[10]; /* PendSV */
    s_nvicContext.SHPR[11] = SCB->SHPR[11]; /* SysTick */

    s_nvicContext.ICSR  = SCB->ICSR;
    s_nvicContext.VTOR  = SCB->VTOR;
    s_nvicContext.AIRCR = SCB->AIRCR;
    s_nvicContext.SCR   = SCB->SCR;
    s_nvicContext.CCR   = SCB->CCR;
    s_nvicContext.SHCSR = SCB->SHCSR;
    s_nvicContext.MMFAR = SCB->MMFAR;
    s_nvicContext.BFAR  = SCB->BFAR;
    s_nvicContext.CPACR = SCB->CPACR;
    s_nvicContext.NSACR = SCB->NSACR;
}

void LPM_NvicStateRestore(void)
{
    uint32_t i;
    uint32_t irqRegs;
    uint32_t irqNum;

    irqRegs = (SCnSCB->ICTR & SCnSCB_ICTR_INTLINESNUM_Msk) + 1;
    irqNum  = irqRegs * 32;

    NVIC_SetPriorityGrouping(s_nvicContext.PriorityGroup);

    for (i = 0; i < irqRegs; i++)
    {
        NVIC->ISER[i] = s_nvicContext.ISER[i];
    }

    for (i = 0; i < irqNum; i++)
    {
        NVIC->IPR[i] = s_nvicContext.IPR[i];
    }

    SCB->SHPR[0]  = s_nvicContext.SHPR[0];  /* MemManage */
    SCB->SHPR[1]  = s_nvicContext.SHPR[1];  /* BusFault */
    SCB->SHPR[2]  = s_nvicContext.SHPR[2];  /* UsageFault */
    SCB->SHPR[7]  = s_nvicContext.SHPR[7];  /* SVCall */
    SCB->SHPR[8]  = s_nvicContext.SHPR[8];  /* DebugMonitor */
    SCB->SHPR[10] = s_nvicContext.SHPR[10]; /* PendSV */
    SCB->SHPR[11] = s_nvicContext.SHPR[11]; /* SysTick */

    SCB->ICSR  = s_nvicContext.ICSR;
    SCB->VTOR  = s_nvicContext.VTOR;
    SCB->AIRCR = s_nvicContext.AIRCR;
    SCB->SCR   = s_nvicContext.SCR;
    SCB->CCR   = s_nvicContext.CCR;
    SCB->SHCSR = s_nvicContext.SHCSR;
    SCB->MMFAR = s_nvicContext.MMFAR;
    SCB->BFAR  = s_nvicContext.BFAR;
    SCB->CPACR = s_nvicContext.CPACR;
    SCB->NSACR = s_nvicContext.NSACR;
}

void LPM_SaveRegister(uint32_t *buf, uint32_t *index, uint32_t base, uint32_t begin, uint32_t end, uint32_t bitmap)
{
    uint32_t offset = begin;

    while (offset <= end)
    {
        buf[*index] = (*((volatile uint32_t *)(base + offset))) & bitmap;
        (*index)++;
        offset += 4;
    }
}

AT_QUICKACCESS_SECTION_CODE(void LPM_RestoreRegister(
    uint32_t *buf, uint32_t *index, uint32_t base, uint32_t begin, uint32_t end, uint32_t bitmap))
{
    uint32_t offset = begin;
    uint32_t tmp;

    while (offset <= end)
    {
        tmp = *((volatile uint32_t *)(base + offset));
        tmp &= ~bitmap;
        tmp |= buf[*index];
        *((volatile uint32_t *)(base + offset)) = tmp;
        (*index)++;
        offset += 4;
    }
}

AT_QUICKACCESS_SECTION_CODE(void LPM_SaveFlexspi0(void))
{
    s_flexspi0ContextIndex = 0;
    LPM_SaveRegister(s_flexspi0Context, &s_flexspi0ContextIndex, FLEXSPI0_BASE, 0x0, 0x0,
                     0xFFFFFFF0); /* Skip MDIS and SWRESET */
    LPM_SaveRegister(s_flexspi0Context, &s_flexspi0ContextIndex, FLEXSPI0_BASE, 0x4, 0xC4, 0xFFFFFFFF);
    LPM_SaveRegister(s_flexspi0Context, &s_flexspi0ContextIndex, FLEXSPI0_BASE, 0x200, 0x2FC,
                     0xFFFFFFFF); /* LUT table */
}

void LPM_ModuleStateSave(void)
{
    /* IOMUXC0 */
    s_iomuxc0ContextIndex = 0;
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x0, 0xBC, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x100, 0x15C, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x400, 0x404, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x800, 0x810, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x898, 0x89C, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x8A0, 0x8FC, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x928, 0x9F0, 0xFFFFFFFF);
    LPM_SaveRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0xA04, 0xAE8, 0xFFFFFFFF);

    /* RGPIO B */
    s_rgpiobContextIndex = 0;
    LPM_SaveRegister(s_rgpiobContext, &s_rgpiobContextIndex, GPIOB_BASE, 0x10, 0x1C, 0xFFFFFFFF);
    LPM_SaveRegister(s_rgpiobContext, &s_rgpiobContextIndex, GPIOB_BASE, 0x40, 0x40, 0xFFFFFFFF);
    LPM_SaveRegister(s_rgpiobContext, &s_rgpiobContextIndex, GPIOB_BASE, 0x54, 0x58, 0xFFFFFFFF);

    /* PCC0 */
    s_pcc0ContextIndex = 0;
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x4, 0x90, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x98, 0x98, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0xA0, 0xA4, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0xB0, 0xB4, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0xC4, 0x110, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x118, 0x118, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x130, 0x130, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x138, 0x13c, 0xFFFFFFFF);

    /* PCC1 */
    s_pcc1ContextIndex = 0;
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0xC, 0xC, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x18, 0x18, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x48, 0x58, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x60, 0x7C, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x88, 0x88, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0xA0, 0xA0, 0xFFFFFFFF);
    LPM_SaveRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0xB4, 0xBC, 0xFFFFFFFF);

    /* FlexSPI0(save settings of flexspi0 to TCM/SSRAM when running XiP) */
    /*
     * Dual boot type/low power boot type: M33 BOOTROM will initialize flexspi0;
     * Single boot type: M33 BOOTROM don't initialize flexspi0.
     * So M33 can access and save registers of flexspi0 in dual boot type and low power boot type.
     */
    if (BOARD_IS_XIP_FLEXSPI0() || !BOARD_IsSingleBootType())
    {
        LPM_SaveFlexspi0();
    }

    /* SIM_SEC */
    s_simsecContextIndex = 0;
    LPM_SaveRegister(s_simsecContext, &s_simsecContextIndex, SIM_SEC_BASE, 0x40, 0x60, 0xFFFFFFFF);

    /* CGC0 FRORDTRIM */
    s_cgc0ContextIndex = 0;
    LPM_SaveRegister(s_cgc0Context, &s_cgc0ContextIndex, CGC_RTD_BASE, 0x210, 0x210, 0xFFFFFFFF);
}

AT_QUICKACCESS_SECTION_CODE(void LPM_RestoreFlexspi0(void))
{
    s_flexspi0ContextIndex = 0;

    if (s_flexspi0Context[0] == 0U)
        return;

    /* Restore generic register */
    LPM_RestoreRegister(s_flexspi0Context, &s_flexspi0ContextIndex, FLEXSPI0_BASE, 0x0, 0x0,
                        0xFFFFFFF0); /* Skip MDIS and SWRESET bits */
    LPM_RestoreRegister(s_flexspi0Context, &s_flexspi0ContextIndex, FLEXSPI0_BASE, 0x4, 0xc4, 0xFFFFFFFF);

    /* Enable FlexSPI module */
    FLEXSPI0->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    /* Restore LUT */
    FLEXSPI0->LUTKEY = FLEXSPI_LUT_KEY_VAL;
    FLEXSPI0->LUTCR  = FLEXSPI_LUTCR_UNLOCK_MASK;
    for (uint32_t i = 0; i < CUSTOM_LUT_LENGTH; i++)
    {
        FLEXSPI0->LUT[i] = s_flexspi0Context[s_flexspi0ContextIndex + i];
    }
    FLEXSPI0->LUTKEY = FLEXSPI_LUT_KEY_VAL;
    FLEXSPI0->LUTCR  = FLEXSPI_LUTCR_LOCK_MASK;

    /* Software reset */
    FLEXSPI0->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
    while (FLEXSPI0->MCR0 & FLEXSPI_MCR0_SWRESET_MASK)
    {
    }
}

AT_QUICKACCESS_SECTION_CODE(void LPM_ModuleStateRestore(void))
{
    /* IOMUXC0 */
    s_iomuxc0ContextIndex = 0;
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x0, 0xBC, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x100, 0x15C, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x400, 0x404, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x800, 0x810, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x898, 0x89C, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x8A0, 0x8FC, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0x928, 0x9F0, 0xFFFFFFFF);
    LPM_RestoreRegister(s_iomuxc0Context, &s_iomuxc0ContextIndex, IOMUXC0_BASE, 0xA04, 0xAE8, 0xFFFFFFFF);

    /* RGPIO B */
    s_rgpiobContextIndex = 0;
    LPM_RestoreRegister(s_rgpiobContext, &s_rgpiobContextIndex, GPIOB_BASE, 0x10, 0x1C, 0xFFFFFFFF);
    LPM_RestoreRegister(s_rgpiobContext, &s_rgpiobContextIndex, GPIOB_BASE, 0x40, 0x40, 0xFFFFFFFF);
    LPM_RestoreRegister(s_rgpiobContext, &s_rgpiobContextIndex, GPIOB_BASE, 0x54, 0x58, 0xFFFFFFFF);

    /* PCC0 */
    s_pcc0ContextIndex = 0;
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x4, 0x90, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x98, 0x98, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0xA0, 0xA4, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0xB0, 0xB4, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0xC4, 0x110, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x118, 0x118, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x130, 0x130, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc0Context, &s_pcc0ContextIndex, PCC0_BASE, 0x138, 0x13c, 0xFFFFFFFF);

    /* PCC1 */
    s_pcc1ContextIndex = 0;
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0xC, 0xC, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x18, 0x18, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x48, 0x58, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x60, 0x7C, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0x88, 0x88, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0xA0, 0xA0, 0xFFFFFFFF);
    LPM_RestoreRegister(s_pcc1Context, &s_pcc1ContextIndex, PCC1_BASE, 0xB4, 0xBC, 0xFFFFFFFF);

    /* FlexSPI0 */
    /*
     * Dual boot type/low power boot type: M33 BOOTROM will initialize flexspi0;
     * Single boot type: M33 BOOTROM don't initialize flexspi0.
     * So M33 can access and save registers of flexspi0 in dual boot type and low power boot type.
     */
    if (BOARD_IS_XIP_FLEXSPI0() || !BOARD_IsSingleBootType())
    {
        LPM_RestoreFlexspi0();
    }

    /* SIM_SEC */
    s_simsecContextIndex = 0;
    LPM_RestoreRegister(s_simsecContext, &s_simsecContextIndex, SIM_SEC_BASE, 0x40, 0x60, 0xFFFFFFFF);

    /* CGC0 FRORDTRIM */
    s_cgc0ContextIndex = 0;
    LPM_RestoreRegister(s_cgc0Context, &s_cgc0ContextIndex, CGC_RTD_BASE, 0x210, 0x210, 0xFFFFFFFF);
}

bool LPM_Suspend()
{
    __asm volatile(
        "loop:\n"
        "PUSH    {R4-R11, LR}\n" /* #1 Save core registers to current stack */

        "MRS     R0, PSP\n"      /* #2 Save PSP register to global variable s_psp */
        "LDR     R1, =s_psp\n"
        "STR     R0, [R1]\n"

        "MRS     R0, MSP\n" /* #3 Save MSP register to global variable s_msp */
        "LDR     R1, =s_msp\n"
        "STR     R0, [R1]\n"

        "MRS     R0, CONTROL\n" /* #4 Save CONTROL register to global variable s_control */
        "LDR     R1, =s_control\n"
        "STR     R0, [R1]\n");

    LPM_NvicStateSave();   /* #5 Save NVIC setting */
    LPM_ModuleStateSave(); /* #6 Save peripheral setting */

    /* Switch clock source from PLL0/1 to FRO to save more power during Power Down,
     * then switch back after wakeup in function BOARD_ResumeClockInit() ``*/
    BOARD_SwitchToFROClk(BOARD_GetRtdDriveMode());
    BOARD_DisablePlls();

    /* Program CMC0 to notify system (upower) to enter Power Down */
    RTDCMC_SetClockMode(CMC_RTD, kRTDCMC_GateAllSystemClocks);
    RTDCMC_SetPowerModeProtection(CMC_RTD, CMC_RTD_PMPROT_APD_MASK);
    RTDCMC_EnableDebugOperation(CMC_RTD, false);
    RTDCMC_EnterLowPowerMode(CMC_RTD, kRTDCMC_PowerDownMode);
    /* Successful Power Down exit will never come here, but jump to LPM_Resume() */

    /* Power Down entering fail, retore core register berore return */
    __asm volatile("POP    {R4-R11, LR}\n");
    return false;
}

AT_QUICKACCESS_SECTION_CODE(bool LPM_Resume(void))
{
    /* Re-entry point of Power Down wake*/
    __asm volatile(
        "resume:\n"
        "CPSID   I\n"              /* Mask interrupts */
        "LDR     R2, =tmp_stack\n" /* Setup temp stack for following init */
        "ADD     R2, R2, #0x300\n"
        "MSR     MSP, R2\n");

    /* Clear Power Down Resume Entry. */
    SIM_SEC->DGO_GP0   = 0;
    SIM_SEC->DGO_CTRL0 = SIM_SEC_DGO_CTRL0_UPDATE_DGO_GP0_MASK;
    /* Wait DGO GP0 updated */
    while ((SIM_SEC->DGO_CTRL0 & SIM_SEC_DGO_CTRL0_WR_ACK_DGO_GP0_MASK) == 0)
    {
    }
    /* Clear DGO GP0 ACK and UPDATE bits */
    SIM_SEC->DGO_CTRL0 =
        (SIM_SEC->DGO_CTRL0 & ~(SIM_SEC_DGO_CTRL0_UPDATE_DGO_GP0_MASK)) | SIM_SEC_DGO_CTRL0_WR_ACK_DGO_GP0_MASK;

    LPM_ModuleStateRestore();           /* #6 Restore peripheral setting */
                                        //__WFI(); //pass
    LPM_NvicStateRestore();             /* #5 Restore NVIC setting */

    CACHE64_EnableCache(CACHE64_CTRL0); /* enable code bus cache(I-Cache) */

    __asm volatile(
        "LDR     R1, =s_control\n" /* #4 Restore CONTROL register from global variable s_control */
        "LDR     R0, [R1]\n"
        "MSR     CONTROL, R0\n"

        "LDR     R1, =s_msp\n" /* #3 Restore MSP register from global variable s_msp */
        "LDR     R0, [R1]\n"
        "MSR     MSP, R0\n"

        "LDR     R1, =s_psp\n" /* #2 Restore PSP register from global variable s_psp */
        "LDR     R0, [R1]\n"
        "MSR     PSP, R0\n"

        "POP    {R4-R11, LR}\n" /* #1 Restore core registers from current stack */
    );

    return true;
}

bool LPM_SystemPowerDown(void)
{
    bool status;
    uint32_t code_start_addr;
    uint32_t code_size;

    /* Setup Power Down Resume Entry. */
    SIM_SEC->DGO_GP0   = (uint32_t)LPM_Resume;
    SIM_SEC->DGO_CTRL0 = SIM_SEC_DGO_CTRL0_UPDATE_DGO_GP0_MASK;
    /* Wait DGO GP0 updated */
    while ((SIM_SEC->DGO_CTRL0 & SIM_SEC_DGO_CTRL0_WR_ACK_DGO_GP0_MASK) == 0)
    {
    }
    /* Clear DGO GP0 ACK and UPDATE bits */
    SIM_SEC->DGO_CTRL0 =
        (SIM_SEC->DGO_CTRL0 & ~(SIM_SEC_DGO_CTRL0_UPDATE_DGO_GP0_MASK)) | SIM_SEC_DGO_CTRL0_WR_ACK_DGO_GP0_MASK;

    /* Unlock and disabled in powerdown and deep sleep */
    CGC_RTD->LPOSCCSR &= ~(1 << 23);
    CGC_RTD->LPOSCCSR &= ~(0x3 << 1);
    while (!(CGC_RTD->LPOSCCSR & (1 << 24)))
        ;

    /* Initialize upower config data */
    struct ps_pwr_mode_cfg_t *pwr_sys_cfg = (struct ps_pwr_mode_cfg_t *)UPWR_DRAM_SHARED_BASE_ADDR;

    /* Copy upower config data to target memory */
    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[PD_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[PD_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[PD_RTD_PWR_MODE], &rtd_swt_cfgs[PD_RTD_PWR_MODE], sizeof(swt_config_t));

    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[ACT_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[ACT_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[ACT_RTD_PWR_MODE], &rtd_swt_cfgs[ACT_RTD_PWR_MODE], sizeof(swt_config_t));

    for (uint32_t rtd_mode = 0; rtd_mode < NUM_RTD_PWR_MODES; rtd_mode++)
    {
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_board =
            (struct upwr_switch_board_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_board);
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_mem =
            (struct upwr_mem_switches_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_mem);
    }

    memcpy(&pwr_sys_cfg->ps_rtd_pmic_reg_data_cfg, &rtd_pmic_reg_cfgs_pd, sizeof(ps_rtd_pmic_reg_data_cfgs_t));

#if defined(__ICCARM__)
    extern void __stext;
    extern void __etext;
#elif defined(__GNUC__)
    extern const void __stext;
    extern const void __etext;
#else
#error Not support the compiler.
#endif
    code_start_addr = (uint32_t)&__stext;
    code_size       = (uint32_t)&__etext - (uint32_t)&__stext + 1;
    SENTINEL_SetPowerDown(code_start_addr, code_size);

    status = LPM_Suspend();

    BOARD_ConfigMPU();
    // BOARD_ResumeClockInit();

    return status;
}

AT_QUICKACCESS_SECTION_CODE(status_t flexspi_nor_exec_op(FLEXSPI_Type *base,
                                                         uint32_t deviceAddr,
                                                         flexspi_port_t port,
                                                         flexspi_command_type_t cmdType,
                                                         uint8_t seqIndex,
                                                         uint8_t seqNumber,
                                                         uint32_t *data,
                                                         size_t dataSize))
{
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = deviceAddr;
    flashXfer.port          = port;
    flashXfer.cmdType       = cmdType;
    flashXfer.SeqNumber     = seqNumber;
    flashXfer.seqIndex      = seqIndex;
    flashXfer.data          = data;
    flashXfer.dataSize      = dataSize;

    return FLEXSPI_TransferBlocking(base, &flashXfer);
}

AT_QUICKACCESS_SECTION_CODE(bool LPM_SystemDeepPowerDown(void))
{
    uint32_t tempLUT[4] = {0};
    status_t status     = kStatus_Fail;
    int i               = 0;

    /* Unlock and disabled in powerdown and deep sleep */
    CGC_RTD->LPOSCCSR &= ~(1 << 23);
    CGC_RTD->LPOSCCSR &= ~(0x3 << 1);
    while (!(CGC_RTD->LPOSCCSR & (1 << 24)))
        ;

    /* Initialize upower config data */
    struct ps_pwr_mode_cfg_t *pwr_sys_cfg = (struct ps_pwr_mode_cfg_t *)UPWR_DRAM_SHARED_BASE_ADDR;

    /* Copy upower config data to target memory */
    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[DPD_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[DPD_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[DPD_RTD_PWR_MODE], &rtd_swt_cfgs[DPD_RTD_PWR_MODE], sizeof(swt_config_t));

    memcpy(&pwr_sys_cfg->ps_rtd_pwr_mode_cfg[ACT_RTD_PWR_MODE], &rtd_pwr_mode_cfgs[ACT_RTD_PWR_MODE],
           sizeof(struct ps_rtd_pwr_mode_cfg_t));
    memcpy(&pwr_sys_cfg->ps_rtd_swt_cfg[ACT_RTD_PWR_MODE], &rtd_swt_cfgs[ACT_RTD_PWR_MODE], sizeof(swt_config_t));

    memcpy(&pwr_sys_cfg->ps_rtd_pmic_reg_data_cfg, &rtd_pmic_reg_cfgs_dpd, sizeof(ps_rtd_pmic_reg_data_cfgs_t));

    for (uint32_t rtd_mode = 0; rtd_mode < NUM_RTD_PWR_MODES; rtd_mode++)
    {
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_board =
            (struct upwr_switch_board_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_board);
        pwr_sys_cfg->ps_rtd_pwr_mode_cfg[rtd_mode].swt_mem =
            (struct upwr_mem_switches_t *)(pwr_sys_cfg->ps_rtd_swt_cfg[rtd_mode].swt_mem);
    }

    /*
     * Switch clock source from PLL0/1 to FRO to save more power during Power Down,
     * then switch back after wakeup in function BOARD_ResumeClockInit()
     */
    BOARD_SwitchToFROClk(BOARD_GetRtdDriveMode());
    BOARD_DisablePlls();

    /* Program CMC0 to notify system (upower) to enter Deep Power Down */
    RTDCMC_SetClockMode(CMC_RTD, kRTDCMC_GateAllSystemClocks);
    RTDCMC_SetPowerModeProtection(CMC_RTD, CMC_RTD_PMPROT_ADPD_MASK);
    RTDCMC_EnableDebugOperation(CMC_RTD, false);

    /* Note: Pls enable flexspi pins before using flexspi0 to send command to nor flash */
    /*
     * Change function of pins(PTC0~10) of flexspi0 as flexspi when non-XiP
     * When XiP, skip changing function of pins of flexspi in APP_Suspend().
     */
    if (!BOARD_IS_XIP_FLEXSPI0())
    {
        IOMUXC_SetPinMux(IOMUXC_PTC0_FLEXSPI0_A_DQS, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC1_FLEXSPI0_A_DATA7, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC2_FLEXSPI0_A_DATA6, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC3_FLEXSPI0_A_DATA5, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC4_FLEXSPI0_A_DATA4, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC5_FLEXSPI0_A_SS0_B, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC6_FLEXSPI0_A_SCLK, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC7_FLEXSPI0_A_DATA3, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC8_FLEXSPI0_A_DATA2, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC9_FLEXSPI0_A_DATA1, 0U);
        IOMUXC_SetPinMux(IOMUXC_PTC10_FLEXSPI0_A_DATA0, 0U);

        /* Restore flexspi0's settings, it will reset NOR FLASH to default by flexspi pins when with non-XiP */
        LPM_RestoreFlexspi0();
    }

    /* Detect which mode flash is in(SPI, OPI), currently it's in SPI mode */
    /* Whether enabled 4-Byte address mode, for GD25LX256E, enabled 4-Byte address mode */
    /* Change from 4-byte address mode to 3-byte address mode in SDR SPI Mode for the nor flash GD25LX256E */
    memset((void *)tempLUT, 0, sizeof(tempLUT));
    /*
     * 0xE9: Disable 4-Byte Address Mode, exit 4-Byte address mode (from GigaDevice GD25LX256E), for ATXP032, no this
     * command 0xE9
     */
    tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINOR_OP_DISABLE_4_BYTE_ADDR_MODE_GIGADEVICE,
                                 kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x0);
    FLEXSPI_UpdateLUT(BOARD_FLEXSPI_CONNECT_TO_NOR_FLASH, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));
    status = flexspi_nor_exec_op(BOARD_FLEXSPI_CONNECT_TO_NOR_FLASH, 0, BOARD_FLEXSPI_NOR_FLASH_PORT, kFLEXSPI_Read,
                                 NOR_CMD_LUT_SEQ_IDX_CONFIG, 1, NULL, 0);
    (void)status; /* Fix warning: variable "status" was set but never used */
    /*
     * Change function of pins(PTC0~10) of flexspi0 to save power
     * Note: Currently make sure that the instruction is in TCM, then it's safe to do anything for flexspi0 and NOR
     * FLASH
     */
    for (i = 0; i <= 10; i++)
    {
        IOMUXC0->PCR0_IOMUXCARRAY2[i] = 0; /* Set to Analog/HiZ state */
    }

    /* assert is in flash for XiP case(flash_debug, flash_release target), so pls don't check status with assert */
    /* assert(status == kStatus_Success); */
    /* Cannot place the function RTDCMC_EnterLowPowerMode to TCM for Xip case(flash_debug, flash_release target), so
     * copy the code of the function to here directly */
    /* RTDCMC_EnterLowPowerMode(CMC_RTD, kRTDCMC_DeepPowerDown); */
    CMC_RTD->RTD_PMCTRL = CMC_RTD_PMCTRL_RTD_LPMODE(kRTDCMC_DeepPowerDown);

    /*
     * Before executing WFI instruction read back the last register to
     * ensure all registers writes have completed.
     */
    (void)CMC_RTD->RTD_PMCTRL;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __DSB();
    __WFI();
    __ISB();

    /* Successful Power Down exit will never come here, but do normal reset */
    return kStatus_Fail;
}

bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec)
{
    uint32_t irqMask;
    status_t status = kStatus_Success;

    irqMask = DisableGlobalIRQ();

    DisableIRQ(SYSTICK_IRQn);
    LPTMR_StopTimer(SYSTICK_BASE);

    switch (s_curMode)
    {
        case LPM_PowerModeWait:
            /* Clear the SLEEPDEEP bit to disable deep sleep mode (wait mode) */
            SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
            __DSB();
            __WFI();
            __ISB();
            status = kStatus_Success;
            break;
        case LPM_PowerModeStop:
            /* Set the SLEEPDEEP bit to enable deep sleep mode (stop mode) */
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            __DSB();
            __WFI();
            __ISB();
            status = kStatus_Success;
            break;
        case LPM_PowerModeSleep:
            if (!LPM_SystemSleep())
            {
                status = kStatus_Fail;
            }
            break;
        case LPM_PowerModeDeepSleep:
            if (!LPM_SystemDeepSleep())
            {
                status = kStatus_Fail;
            }
            break;
        case LPM_PowerModePowerDown:
            if (!LPM_SystemPowerDown())
            {
                status = kStatus_Fail;
            }
            break;
        case LPM_PowerModeDeepPowerDown:
            if (!LPM_SystemDeepPowerDown())
            {
                status = kStatus_Fail;
            }
            break;
        default:
            break;
    }

    LPTMR_StartTimer(SYSTICK_BASE);
    NVIC_EnableIRQ(SYSTICK_IRQn);
    EnableGlobalIRQ(irqMask);

    return status == kStatus_Success;
}

void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data)
{
    lpm_power_mode_listener_t *l = (lpm_power_mode_listener_t *)pvPortMalloc(sizeof(lpm_power_mode_listener_t));
    assert(l);

    l->callback = callback;
    l->data     = data;
    l->next     = NULL;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    if (s_listenerHead)
    {
        s_listenerTail->next = l;
        s_listenerTail       = l;
    }
    else
    {
        s_listenerHead = s_listenerTail = l;
    }

    xSemaphoreGive(s_mutex);
}

void LPM_UnregisterPowerListener(lpm_power_mode_callback_t callback, void *data)
{
    lpm_power_mode_listener_t *l, *p = NULL;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    for (l = s_listenerHead; l != NULL; l = l->next)
    {
        if (l->callback == callback && l->data == data)
        {
            if (p)
            {
                p->next = l->next;
            }
            else
            {
                s_listenerHead = l->next;
            }

            if (l->next == NULL)
            {
                s_listenerTail = p;
            }

            vPortFree(l);
            break;
        }
        p = l;
    }

    xSemaphoreGive(s_mutex);
}

/************ Internal public API start **************/
/* The systick interrupt handler. */
void SYSTICK_HANDLER(void)
{
    /* Clear the interrupt. */
    LPTMR_ClearStatusFlags(SYSTICK_BASE, kLPTMR_TimerCompareFlag);

    if (SYSTICK_BASE->CSR & LPTMR_CSR_TFC_MASK)
    {
        /* Freerun timer means this is the first tick after tickless exit. */
        LPTMR_StopTimer(SYSTICK_BASE);
        LPTMR_SetTimerPeriod(LPTMR0, SYSTICK_SOURCE_CLOCK / configTICK_RATE_HZ);
        SYSTICK_BASE->CSR &= ~LPTMR_CSR_TFC_MASK;
        LPTMR_StartTimer(SYSTICK_BASE);
    }
    /* Call FreeRTOS tick handler. */
    vPortSysTickHandler();
}

/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
 * defined in the FreeRTOS Cortex-M4F port layer with a version that configures LPTMR0
 * to generate the tick interrupt. */
void vPortSetupTimerInterrupt(void)
{
    lptmr_config_t lptmrConfig;

    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* Select SIRC as tick timer clock source */
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
    /* Initialize the LPTMR */
    LPTMR_Init(SYSTICK_BASE, &lptmrConfig);

    /* Set timer period */
    LPTMR_SetTimerPeriod(SYSTICK_BASE, SYSTICK_SOURCE_CLOCK / configTICK_RATE_HZ);

    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(SYSTICK_BASE, kLPTMR_TimerInterruptEnable);
    NVIC_SetPriority(SYSTICK_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(SYSTICK_IRQn);

    /* Start counting */
    LPTMR_StartTimer(SYSTICK_BASE);
}
