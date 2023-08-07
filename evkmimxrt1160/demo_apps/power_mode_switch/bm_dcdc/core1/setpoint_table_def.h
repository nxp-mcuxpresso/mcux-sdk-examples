/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SETPOINT_TABLE_DEF_H__
#define __SETPOINT_TABLE_DEF_H__

/* clang-format off */
#define SP0  0
#define SP1  1
#define SP2  2
#define SP3  3
#define SP4  4
#define SP5  5
#define SP6  6
#define SP7  7
#define SP8  8
#define SP9  9
#define SP10 10
#define SP11 11
#define SP12 12
#define SP13 13
#define SP14 14
#define SP15 15

// Set points ompatible table
#define CPU0_COMPATIBLE_SP_TABLE                                                              \
{{NA , SP0, SP1, SP2, SP3, SP4, SP5, SP6, SP7, SP8, SP9, SP10, SP11, SP12, SP13, SP14, SP15}, \
{SP0 ,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP1 ,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP2 ,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP3 ,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP4 ,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP5 ,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP6 ,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP7 ,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP8 ,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,    0,    0,    0,    0,    0,    0}, \
{SP9 ,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    0,    0,    0,    0,    0,    0}, \
{SP10,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    0,    0,    0,    0,    0}, \
{SP11,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    1}, \
{SP12,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    1}, \
{SP13,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    1}, \
{SP14,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    1}, \
{SP15,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    1}}

#define CPU1_COMPATIBLE_SP_TABLE                                                              \
{{NA , SP0, SP1, SP2, SP3, SP4, SP5, SP6, SP7, SP8, SP9, SP10, SP11, SP12, SP13, SP14, SP15}, \
{SP0 ,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP1 ,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP2 ,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,    0,    0,    0,    0,    0,    0}, \
{SP3 ,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,    0,    1,    0,    0,    0,    0}, \
{SP4 ,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,    0,    1,    0,    0,    0,    0}, \
{SP5 ,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,    0,    1,    0,    0,    0,    0}, \
{SP6 ,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,    0,    1,    0,    0,    0,    0}, \
{SP7 ,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,    0,    1,    0,    0,    0,    0}, \
{SP8 ,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,    0,    1,    0,    0,    0,    0}, \
{SP9 ,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    0,    1,    0,    0,    0,    0}, \
{SP10,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    0,    0,    0}, \
{SP11,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,    0,    1,    0,    0,    0,    0}, \
{SP12,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    0,    1,    1,    0,    0,    0}, \
{SP13,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    0,    0}, \
{SP14,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    0}, \
{SP15,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1,    1,    1,    1,    1,    1}}

// Power domain on/off set point value
#define PD_WKUP_SP_VAL  0xf800 // OFF at SP11~SP15
#define PD_MEGA_SP_VAL  0xf800 // OFF at SP11~SP15
#define PD_DISP_SP_VAL  0xf800 // OFF at SP11 ~SP15
#define PD_LPSR_SP_VAL  0x0000 // ON at all SP
#define PD_PMIC_SP_VAL  0x0000
#define PD_M4MEM_SP_VAL 0x0000
#define PD_M7MEM_SP_VAL 0x0000

// DCDC
#define DCDC_ONOFF_SP_VAL     (~PD_WKUP_SP_VAL)
#define DCDC_DIG_ONOFF_SP_VAL DCDC_ONOFF_SP_VAL
#define DCDC_LP_MODE_SP_VAL   0x0000
#define DCDC_ONOFF_STBY_VAL   DCDC_ONOFF_SP_VAL
#define DCDC_LP_MODE_STBY_VAL 0x0000
// DCDC 1P0 buck mode target voltage in sp0~15
#define DCDC_1P0_BUCK_MODE_CONFIGURATION_TABLE \
{                                              \
    kDCDC_1P0BuckTarget1P0V,                   \
    kDCDC_1P0BuckTarget1P0V,                   \
    kDCDC_1P0BuckTarget1P0V,                   \
    kDCDC_1P0BuckTarget1P0V,                   \
    kDCDC_1P0BuckTarget1P0V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P8V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
    kDCDC_1P0BuckTarget0P9V,                   \
}
// DCDC 1P0 standby mode target voltage in sp0~15
#define DCDC_1P0_STANDBY_MODE_CONFIGURATION_TABLE \
{                                                 \
    kDCDC_1P0StbyTarget1P0V,                      \
    kDCDC_1P0StbyTarget1P0V,                      \
    kDCDC_1P0StbyTarget1P0V,                      \
    kDCDC_1P0StbyTarget1P0V,                      \
    kDCDC_1P0StbyTarget1P0V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P8V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
    kDCDC_1P0StbyTarget0P9V,                      \
}

// PMU
#define PMU_BG_SP_VAL                  0xffff
#define PMU_RBB_SOC_SP_VAL             0x0500
#define PMU_RBB_LPSR_SP_VAL            0x8000
#define PMU_LDO_PLL_EN_SP_VAL          0x007f
#define PMU_LDO_ANA_EN_SP_VAL          0xf800
#define PMU_LDO_ANA_LP_MODE_SP_VAL     0xffff
#define PMU_LDO_ANA_TRACKING_EN_SP_VAL (~PMU_LDO_ANA_EN_SP_VAL)
#define PMU_LDO_ANA_BYPASS_EN_SP_VAL   (~PMU_LDO_ANA_EN_SP_VAL)
#define PMU_LDO_DIG_EN_SP_VAL          0xf81c
#define PMU_LDO_DIG_LP_MODE_SP_VAL     0xffff
#define PMU_LDO_DIG_TRACKING_EN_SP_VAL (~PMU_LDO_DIG_EN_SP_VAL)
#define PMU_LDO_DIG_BYPASS_EN_SP_VAL   (~PMU_LDO_DIG_EN_SP_VAL)

#define PMU_BG_STBY_VAL         0xffff
#define PMU_FBB_STBY_VAL        0xffff
#define PMU_RBB_SOC_STBY_VAL    0xffff
#define PMU_RBB_LPSR_STBY_VAL   0xffff
#define PMU_LDO_PLL_EN_STBY_VAL 0xffff
#define PMU_LDO_ANA_EN_STBY_VAL 0xffff
#define PMU_LDO_DIG_EN_STBY_VAL 0xffff

// PMU lpsr dig target voltage in sp0~15
#define PMU_LDO_CONDIGURATION_TABLE        \
{                                          \
    kPMU_LpsrDigTargetStableVoltage1P0V,   \
    kPMU_LpsrDigTargetStableVoltage1P0V,   \
    kPMU_LpsrDigTargetStableVoltage1P0V,   \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P806V, \
    kPMU_LpsrDigTargetStableVoltage1P0V,   \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P903V, \
    kPMU_LpsrDigTargetStableVoltage0P806V, \
    kPMU_LpsrDigTargetStableVoltage0P806V  \
}

#define OSC_RC_16M_SP_VAL      0xffff // NOT used
#define OSC_RC_48M_SP_VAL      0x0000
#define OSC_RC_48M_DIV2_SP_VAL 0x0000
#define OSC_RC_400M_SP_VAL     0x7fff
#define OSC_24M_SP_VAL         0x007f
#define OSC_24M_OUT_SP_VAL     0x007f
#define PLL_ARM_SP_VAL         0x001f
#define PLL_ARM_OUT_SP_VAL     0x001f
#define PLL_528_SP_VAL         0x007f
#define PLL_528_OUT_SP_VAL     0x007f
#define PLL_528_PFD0_SP_VAL    0x007f
#define PLL_528_PFD1_SP_VAL    0x007f
#define PLL_528_PFD2_SP_VAL    0x007f
#define PLL_528_PFD3_SP_VAL    0x007f
#define PLL_480_SP_VAL         0x007f
#define PLL_480_OUT_SP_VAL     0x007f
#define PLL_480_DIV2_SP_VAL    0x007f
#define PLL_480_PFD0_SP_VAL    0x007f
#define PLL_480_PFD1_SP_VAL    0x007f
#define PLL_480_PFD2_SP_VAL    0x007f
#define PLL_480_PFD3_SP_VAL    0x007f
#define PLL_1G_SP_VAL          0x000e
#define PLL_1G_OUT_SP_VAL      0x000e
#define PLL_1G_DIV2_SP_VAL     0x000e
#define PLL_1G_DIV5_SP_VAL     0x000e
#define PLL_AUDIO_SP_VAL       0x007f
#define PLL_AUDIO_OUT_SP_VAL   0x007f
#define PLL_VIDEO_SP_VAL       0x007f
#define PLL_VIDEO_OUT_SP_VAL   0x007f

#define OSC_RC_16M_STBY_VAL      0x0000 // Used in GPC since GPC control RC16M ON/OFF
#define OSC_RC_48M_STBY_VAL      0x0000
#define OSC_RC_48M_DIV2_STBY_VAL 0x0000
#define OSC_RC_400M_STBY_VAL     0x0000
#define OSC_24M_STBY_VAL         0x0000
#define OSC_24M_OUT_STBY_VAL     0x0000
#define PLL_ARM_STBY_VAL         0x0000
#define PLL_ARM_OUT_STBY_VAL     0x0000
#define PLL_528_STBY_VAL         0x0000
#define PLL_528_OUT_STBY_VAL     0x0000
#define PLL_528_PFD0_STBY_VAL    0x0000
#define PLL_528_PFD1_STBY_VAL    0x0000
#define PLL_528_PFD2_STBY_VAL    0x0000
#define PLL_528_PFD3_STBY_VAL    0x0000
#define PLL_480_STBY_VAL         0x0000
#define PLL_480_OUT_STBY_VAL     0x0000
#define PLL_480_DIV2_STBY_VAL    0x0000
#define PLL_480_PFD0_STBY_VAL    0x0000
#define PLL_480_PFD1_STBY_VAL    0x0000
#define PLL_480_PFD2_STBY_VAL    0x0000
#define PLL_480_PFD3_STBY_VAL    0x0000
#define PLL_1G_STBY_VAL          0x0000
#define PLL_1G_OUT_STBY_VAL      0x0000
#define PLL_1G_DIV2_STBY_VAL     0x0000
#define PLL_1G_DIV5_STBY_VAL     0x0000
#define PLL_AUDIO_STBY_VAL       0x0000
#define PLL_AUDIO_OUT_STBY_VAL   0x0000
#define PLL_VIDEO_STBY_VAL       0x0000
#define PLL_VIDEO_OUT_STBY_VAL   0x0000

#define CLK_ROOT_M7_SP_TABLE                                                                \
{/* grade, clockOff, mux,                                    div,    Freq(MHz), SetPoint */ \
{   0,     false,    kCLOCK_M7_ClockRoot_MuxArmPllOut,        1 }, /* 600         0   */    \
{   1,     false,    kCLOCK_M7_ClockRoot_MuxArmPllOut,        1 }, /* 600         1   */    \
{   2,     false,    kCLOCK_M7_ClockRoot_MuxSysPll3Out,       1 }, /* 480         2   */    \
{   3,     false,    kCLOCK_M7_ClockRoot_MuxSysPll3Out,       1 }, /* 480         3   */    \
{   4,     false,    kCLOCK_M7_ClockRoot_MuxArmPllOut,        2 }, /* 300         4   */    \
{   5,     false,    kCLOCK_M7_ClockRoot_MuxSysPll3Out,       2 }, /* 240         5   */    \
{   6,     false,    kCLOCK_M7_ClockRoot_MuxSysPll3Out,       2 }, /* 240         6   */    \
{   7,     false,    kCLOCK_M7_ClockRoot_MuxOscRc400M,        2 }, /* 200         7   */    \
{   8,     false,    kCLOCK_M7_ClockRoot_MuxOscRc400M,        2 }, /* 200         8   */    \
{   9,     false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          9   */    \
{   10,    false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          10  */    \
{   11,    false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          11  */    \
{   12,    false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          12  */    \
{   13,    false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          13  */    \
{   14,    false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          14  */    \
{   15,    false,    kCLOCK_M7_ClockRoot_MuxOscRc16M,         1 }, /* 16          15  */    \
}

#define CLK_ROOT_M4_SP_TABLE                                                                \
{/* grade, clockOff, mux,                                     div,   Freq(MHz), SetPoint */ \
{   0,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Out,       2 }, /* 240         0   */    \
{   1,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Pfd3,      2 }, /* 200         1   */    \
{   2,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Pfd3,      2 }, /* 200         2   */    \
{   4,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Out,       4 }, /* 120         3   */    \
{   5,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Out,       4 }, /* 120         4   */    \
{   6,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Out,       4 }, /* 120         5   */    \
{   7,     false,    kCLOCK_M4_ClockRoot_MuxSysPll3Out,       4 }, /* 120         6   */    \
{   8,     false,    kCLOCK_M4_ClockRoot_MuxOscRc400M,        4 }, /* 100         7   */    \
{   9,     false,    kCLOCK_M4_ClockRoot_MuxOscRc400M,        4 }, /* 100         8   */    \
{   10,    false,    kCLOCK_M4_ClockRoot_MuxOscRc400M,        4 }, /* 100         9   */    \
{   12,    false,    kCLOCK_M4_ClockRoot_MuxOscRc16M,         1 }, /* 16          10  */    \
{   3,     false,    kCLOCK_M4_ClockRoot_MuxOscRc400M,        2 }, /* 200         11  */    \
{   11,    false,    kCLOCK_M4_ClockRoot_MuxOscRc400M,        4 }, /* 100         12  */    \
{   13,    false,    kCLOCK_M4_ClockRoot_MuxOscRc16M,         1 }, /* 16          13  */    \
{   14,    false,    kCLOCK_M4_ClockRoot_MuxOscRc16M,         1 }, /* 16          14  */    \
{   15,    false,    kCLOCK_M4_ClockRoot_MuxOscRc16M,         1 }, /* 16          15  */    \
}

#define CLK_ROOT_BUS_SP_TABLE                                                               \
{/* grade, clockOff, mux,                                     div,   Freq(MHz), SetPoint */ \
{   0,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3,     2 }, /* 200         0   */    \
{   1,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3,     2 }, /* 200         1   */    \
{   2,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3,     2 }, /* 200         2   */    \
{   3,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3,     2 }, /* 200         3   */    \
{   4,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll3Out,      4 }, /* 120         4   */    \
{   5,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3,     4 }, /* 100         5   */    \
{   6,     false,    kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3,     4 }, /* 100         6   */    \
{   7,     false,    kCLOCK_BUS_ClockRoot_MuxOscRc400M,       4 }, /* 100         7   */    \
{   8,     false,    kCLOCK_BUS_ClockRoot_MuxOscRc400M,       4 }, /* 100         8   */    \
{   9,     false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          9   */    \
{   10,    false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          10  */    \
{   11,    false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          11  */    \
{   12,    false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          12  */    \
{   13,    false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          13  */    \
{   14,    false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          14  */    \
{   15,    false,    kCLOCK_BUS_ClockRoot_MuxOscRc16M,        1 }, /* 16          15  */    \
}

#define CLK_ROOT_BUS_LPSR_SP_TABLE                                                          \
{/* grade, clockOff, mux,                                     div,   Freq(MHz), SetPoint */ \
{   0,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out,  4 }, /* 120        0   */    \
{   1,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Pfd3, 4 }, /* 100        1   */    \
{   2,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Pfd3, 4 }, /* 100        2   */    \
{   4,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out,  8 }, /* 60         3   */    \
{   5,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out,  8 }, /* 60         4   */    \
{   6,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out,  8 }, /* 60         5   */    \
{   7,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out,  8 }, /* 60         6   */    \
{   8,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M,   8 }, /* 50         7   */    \
{   9,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M,   8 }, /* 50         8   */    \
{   10,    false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M,   8 }, /* 50         9   */    \
{   12,    false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M,    1 }, /* 16         10  */    \
{   3,     false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M,   4 }, /* 100        11  */    \
{   11,    false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M,   8 }, /* 50         12  */    \
{   13,    false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M,    1 }, /* 16         13  */    \
{   14,    false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M,    1 }, /* 16         14  */    \
{   15,    false,    kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M,    1 }, /* 16         15  */    \
}

/* clang-format on */
#endif
