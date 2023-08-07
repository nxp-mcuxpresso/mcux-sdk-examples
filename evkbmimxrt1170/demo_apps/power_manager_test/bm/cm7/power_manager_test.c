/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_dcdc.h"
#include "fsl_ssarc.h"
#include "fsl_pmu.h"
#include "fsl_lpuart.h"
#include "fsl_gpc.h"
#include "fsl_pgmc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CLK_ROOT_M7_SP_TABLE                                                                     \
    { /* grade, clockOff, mux,                                    div,    Freq(MHz), SetPoint */ \
        {3, false, kCLOCK_M7_ClockRoot_MuxArmPllOut, 1},      /* 700         0   */              \
            {0, false, kCLOCK_M7_ClockRoot_MuxSysPll1Out, 1}, /* 1000        1   */              \
            {1, false, kCLOCK_M7_ClockRoot_MuxSysPll1Out, 1}, /* 1000        2   */              \
            {2, false, kCLOCK_M7_ClockRoot_MuxSysPll1Out, 1}, /* 1000        3   */              \
            {4, false, kCLOCK_M7_ClockRoot_MuxArmPllOut, 1},  /* 700         4   */              \
            {5, false, kCLOCK_M7_ClockRoot_MuxSysPll3Out, 2}, /* 240         5   */              \
            {6, false, kCLOCK_M7_ClockRoot_MuxSysPll3Out, 2}, /* 240         6   */              \
            {7, false, kCLOCK_M7_ClockRoot_MuxOscRc400M, 2},  /* 200         7   */              \
            {8, false, kCLOCK_M7_ClockRoot_MuxOscRc400M, 2},  /* 200         8   */              \
            {9, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},   /* 16          9   */              \
            {10, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},  /* 16          10  */              \
            {11, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},  /* 16          11  */              \
            {12, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},  /* 16          12  */              \
            {13, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},  /* 16          13  */              \
            {14, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},  /* 16          14  */              \
            {15, false, kCLOCK_M7_ClockRoot_MuxOscRc16M, 1},  /* 16          15  */              \
    }

#define CLK_ROOT_M4_SP_TABLE                                                                     \
    { /* grade, clockOff, mux,                                     div,   Freq(MHz), SetPoint */ \
        {2, false, kCLOCK_M4_ClockRoot_MuxSysPll3Out, 2},      /* 240         0   */             \
            {0, false, kCLOCK_M4_ClockRoot_MuxSysPll3Pfd3, 1}, /* 400         1   */             \
            {1, false, kCLOCK_M4_ClockRoot_MuxSysPll3Out, 2},  /* 240         2   */             \
            {4, false, kCLOCK_M4_ClockRoot_MuxSysPll3Out, 4},  /* 120         3   */             \
            {5, false, kCLOCK_M4_ClockRoot_MuxSysPll3Out, 4},  /* 120         4   */             \
            {6, false, kCLOCK_M4_ClockRoot_MuxSysPll3Out, 4},  /* 120         5   */             \
            {7, false, kCLOCK_M4_ClockRoot_MuxSysPll3Out, 4},  /* 120         6   */             \
            {8, false, kCLOCK_M4_ClockRoot_MuxOscRc400M, 4},   /* 100         7   */             \
            {9, false, kCLOCK_M4_ClockRoot_MuxOscRc400M, 4},   /* 100         8   */             \
            {10, false, kCLOCK_M4_ClockRoot_MuxOscRc400M, 4},  /* 100         9   */             \
            {12, false, kCLOCK_M4_ClockRoot_MuxOscRc16M, 1},   /* 16          10  */             \
            {3, false, kCLOCK_M4_ClockRoot_MuxOscRc400M, 2},   /* 200         11  */             \
            {11, false, kCLOCK_M4_ClockRoot_MuxOscRc400M, 4},  /* 100         12  */             \
            {13, false, kCLOCK_M4_ClockRoot_MuxOscRc16M, 1},   /* 16          13  */             \
            {14, false, kCLOCK_M4_ClockRoot_MuxOscRc16M, 1},   /* 16          14  */             \
            {15, false, kCLOCK_M4_ClockRoot_MuxOscRc16M, 1},   /* 16          15  */             \
    }

#define CLK_ROOT_BUS_SP_TABLE                                                                    \
    { /* grade, clockOff, mux,                                     div,   Freq(MHz), SetPoint */ \
        {3, false, kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3, 2},     /* 200         0   */            \
            {0, false, kCLOCK_BUS_ClockRoot_MuxSysPll3Out, 2},  /* 240         1   */            \
            {1, false, kCLOCK_BUS_ClockRoot_MuxSysPll3Out, 2},  /* 240         2   */            \
            {2, false, kCLOCK_BUS_ClockRoot_MuxSysPll3Out, 2},  /* 240         3   */            \
            {4, false, kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3, 2}, /* 200         4   */            \
            {5, false, kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3, 4}, /* 100         5   */            \
            {6, false, kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3, 4}, /* 100         6   */            \
            {7, false, kCLOCK_BUS_ClockRoot_MuxOscRc400M, 4},   /* 100         7   */            \
            {8, false, kCLOCK_BUS_ClockRoot_MuxOscRc400M, 4},   /* 100         8   */            \
            {9, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},    /* 16          9   */            \
            {10, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},   /* 16          10  */            \
            {11, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},   /* 16          11  */            \
            {12, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},   /* 16          12  */            \
            {13, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},   /* 16          13  */            \
            {14, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},   /* 16          14  */            \
            {15, false, kCLOCK_BUS_ClockRoot_MuxOscRc16M, 1},   /* 16          15  */            \
    }

#define CLK_ROOT_BUS_LPSR_SP_TABLE                                                               \
    { /* grade, clockOff, mux,                                     div,   Freq(MHz), SetPoint */ \
        {2, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 4},     /* 120         0   */        \
            {0, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 3}, /* 160         1   */        \
            {1, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 4}, /* 120         2   */        \
            {4, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 8}, /* 60          3   */        \
            {5, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 8}, /* 60          4   */        \
            {6, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 8}, /* 60          5   */        \
            {7, false, kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out, 8}, /* 60          6   */        \
            {8, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M, 8},  /* 50          7   */        \
            {9, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M, 8},  /* 50          8   */        \
            {10, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M, 8}, /* 50          9   */        \
            {12, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M, 1},  /* 16          10  */        \
            {3, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M, 4},  /* 100         11  */        \
            {11, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc400M, 8}, /* 50          12  */        \
            {13, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M, 1},  /* 16          13  */        \
            {14, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M, 1},  /* 16          14  */        \
            {15, false, kCLOCK_BUS_LPSR_ClockRoot_MuxOscRc16M, 1},  /* 16          15  */        \
    }

#define UNASSIGNED 0
#define CM7_DOMAIN 1
#define CM7_CTRL   CM7_DOMAIN
#define SP_CTRL    3

#define NA 31
#define CLK_LPCG_CONFIGURATION_TABLE                                                                   \
    { /*ctrlMode,    stbyValue,   spValue,     clock_level,       name,            index  */           \
        {CM7_CTRL, NA, NA, kCLOCK_Level1},               /* M7                0    */                  \
            {UNASSIGNED, NA, NA, kCLOCK_Level1},         /* M4                1    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* SIM_M7            2    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* SIM_M             3    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* SIM_DISP          4    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* SIM_PER           5    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* SIM_LPSR          6    */                  \
            {SP_CTRL, 0xffff, 0xffff, kCLOCK_Level2},    /* ANADIG            7    */                  \
            {SP_CTRL, 0xffff, 0xffff, kCLOCK_Level2},    /* DCDC              8    */                  \
            {SP_CTRL, 0xffff, 0xffff, kCLOCK_Level2},    /* SRC               9    */                  \
            {SP_CTRL, 0xffff, 0xffff, kCLOCK_Level2},    /* CCM              10    */                  \
            {SP_CTRL, 0xffff, 0xffff, kCLOCK_Level2},    /* GPC              11    */                  \
            {SP_CTRL, 0xffff, 0xffff, kCLOCK_Level2},    /* SSARC            12    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SIM_R            13    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* WDOG1            14    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* WDOG2            15    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* WDOG3            16    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* WDOG4            17    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* EWM              18    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* SEMA             19    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level4},         /* MU_A             20    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level4},         /* MU_B             21    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* EDMA             22    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* EDMA_LPSR        23    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* ROMCP            24    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* OCRAM            25    */                  \
            {CM7_DOMAIN, 0x0000, 0xffff, kCLOCK_Level2}, /* FLEXRAM          26    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* LMEM             27    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* FLEXSPI1         28    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* FLEXSPI2         29    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* RDC              30    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* M7_XRDC          31    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* M4_XRDC          32    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* SEMC             33    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* XECC             34    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* IEE              35    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* KEY_MANAGER      36    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* OCOTP            37    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* SNVS_HP          38    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* SNVS             39    for GPIO13 access*/ \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* CAAM             40    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* JTAG_MUX         41    */                  \
            {UNASSIGNED, NA, NA, kCLOCK_Level2},         /* CSTRACE          42    for debug*/         \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* XBAR1            43    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* XBAR2            44    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* XBAR3            45    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* AOI1             46    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* AOI2             47    */                  \
            {SP_CTRL, 0x0000, 0x07ff, kCLOCK_Level2},    /* ADC_ETC          48    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level4},         /* IOMUXC           49    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level4},         /* IOMUXC_LPSR      50    */                  \
            {UNASSIGNED, NA, NA, kCLOCK_Level4},         /* GPIO             51    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* KPP              52    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* FLEXIO1          53    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* FLEXIO2          54    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ADC1             55    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ADC2             56    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* DAC              57    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ACMP1            58    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ACMP2            59    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ACMP3            60    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ACMP4            61    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* PIT1             62    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* PIT2             63    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPT1             64    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPT2             65    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPT3             66    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPT4             67    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPT5             68    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPT6             69    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* QTIMER1          70    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* QTIMER2          71    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* QTIMER3          72    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* QTIMER4          73    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENC1             74    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENC2             75    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENC3             76    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENC4             77    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* HRTIMER          78    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* FLEXPWM1         79    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* FLEXPWM2         80    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* FLEXPWM3         81    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* FLEXPWM4         82    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* CAN1             83    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* CAN2             84    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* CAN3             85    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART1          86    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART2          87    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART3          88    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART4          89    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART5          90    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART6          91    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART7          92    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART8          93    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART9          94    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART10         95    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART11         96    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART12         97    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C1           98    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C2           99    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C3          100    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C4          101    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C5          102    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C6          103    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI1          104    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI2          105    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI3          106    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI4          107    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI5          108    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI6          109    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SIM1            110    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SIM2            111    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENET            112    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENET_1G         113    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ENET_QOS        114    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* USB             115    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SDIO            116    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* USDHC1          117    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* USDHC2          118    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* ASRC            119    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* MQS             120    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* MIC             121    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SPDIF           122    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SAI1            123    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SAI2            124    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SAI3            125    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* SAI4            126    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* PXP             127    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* GPU2D           128    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LCDIF           129    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LCDIFV2         130    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* MIPI_DSI        131    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* MIPI_CSI        132    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* CSI             133    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* DCIC_MIPI       134    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* DCIC_LCD        135    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* VIDEO_MUX       136    */                  \
        {                                                                                              \
            CM7_DOMAIN, NA, NA, kCLOCK_Level2                                                          \
        }                                                                                              \
    } /* UNIQ_EDT_I      137    */

#define CM7_CLK_CTRL SP_CTRL
#define CM4_CLK_CTRL UNASSIGNED

#define CLK_ROOT_CONFIGURATION_TABLE                                 \
    {                     /*ctrl_mode,      name           index  */ \
        CM7_CLK_CTRL,     /* M7               0    */                \
            CM4_CLK_CTRL, /* M4               1    */                \
            SP_CTRL,      /* BUS              2    */                \
            SP_CTRL,      /* BUS_LPSR         3    */                \
            CM7_DOMAIN,   /* SEMC             4    */                \
            UNASSIGNED,   /* CSSYS            5    for debug*/       \
            UNASSIGNED,   /* CSTRACE          6    for debug*/       \
            CM7_DOMAIN,   /* M4_SYSTICK       7    */                \
            CM7_DOMAIN,   /* M7_SYSTICK       8    */                \
            CM7_DOMAIN,   /* ADC1             9    */                \
            CM7_DOMAIN,   /* ADC2            10    */                \
            CM7_DOMAIN,   /* ACMP            11    */                \
            CM7_DOMAIN,   /* FLEXIO1         12    */                \
            CM7_DOMAIN,   /* FLEXIO2         13    */                \
            CM7_DOMAIN,   /* GPT1            14    */                \
            CM7_DOMAIN,   /* GPT2            15    */                \
            CM7_DOMAIN,   /* GPT3            16    */                \
            CM7_DOMAIN,   /* GPT4            17    */                \
            CM7_DOMAIN,   /* GPT5            18    */                \
            CM7_DOMAIN,   /* GPT6            19    */                \
            CM7_DOMAIN,   /* FLEXSPI1        20    */                \
            CM7_DOMAIN,   /* FLEXSPI2        21    */                \
            CM7_DOMAIN,   /* CAN1            22    */                \
            CM7_DOMAIN,   /* CAN2            23    */                \
            CM7_DOMAIN,   /* CAN3            24    */                \
            CM7_DOMAIN,   /* LPUART1         25    */                \
            CM7_DOMAIN,   /* LPUART2         26    */                \
            CM7_DOMAIN,   /* LPUART3         27    */                \
            CM7_DOMAIN,   /* LPUART4         28    */                \
            CM7_DOMAIN,   /* LPUART5         29    */                \
            CM7_DOMAIN,   /* LPUART6         30    */                \
            CM7_DOMAIN,   /* LPUART7         31    */                \
            CM7_DOMAIN,   /* LPUART8         32    */                \
            CM7_DOMAIN,   /* LPUART9         33    */                \
            CM7_DOMAIN,   /* LPUART10        34    */                \
            CM7_DOMAIN,   /* LPUART11        35    */                \
            CM7_DOMAIN,   /* LPUART12        36    */                \
            CM7_DOMAIN,   /* LPI2C1          37    */                \
            CM7_DOMAIN,   /* LPI2C2          38    */                \
            CM7_DOMAIN,   /* LPI2C3          39    */                \
            CM7_DOMAIN,   /* LPI2C4          40    */                \
            CM7_DOMAIN,   /* LPI2C5          41    */                \
            CM7_DOMAIN,   /* LPI2C6          42    */                \
            CM7_DOMAIN,   /* LPSPI1          43    */                \
            CM7_DOMAIN,   /* LPSPI2          44    */                \
            CM7_DOMAIN,   /* LPSPI3          45    */                \
            CM7_DOMAIN,   /* LPSPI4          46    */                \
            CM7_DOMAIN,   /* LPSPI5          47    */                \
            CM7_DOMAIN,   /* LPSPI6          48    */                \
            CM7_DOMAIN,   /* EMV1            49    */                \
            CM7_DOMAIN,   /* EMV2            50    */                \
            CM7_DOMAIN,   /* ENET1           51    */                \
            CM7_DOMAIN,   /* ENET2           52    */                \
            CM7_DOMAIN,   /* ENET_QOS        53    */                \
            CM7_DOMAIN,   /* ENET_25M        54    */                \
            CM7_DOMAIN,   /* ENET_TIMER1     55    */                \
            CM7_DOMAIN,   /* ENET_TIMER2     56    */                \
            CM7_DOMAIN,   /* ENET_TIMER3     57    */                \
            CM7_DOMAIN,   /* USDHC1          58    */                \
            CM7_DOMAIN,   /* USDHC2          59    */                \
            CM7_DOMAIN,   /* ASRC            60    */                \
            CM7_DOMAIN,   /* MQS             61    */                \
            CM7_DOMAIN,   /* MIC             62    */                \
            CM7_DOMAIN,   /* SPDIF           63    */                \
            CM7_DOMAIN,   /* SAI1            64    */                \
            CM7_DOMAIN,   /* SAI2            65    */                \
            CM7_DOMAIN,   /* SAI3            66    */                \
            CM7_DOMAIN,   /* SAI4            67    */                \
            CM7_DOMAIN,   /* GC355           68    */                \
            CM7_DOMAIN,   /* LCDIF           69    */                \
            CM7_DOMAIN,   /* LCDIFV2         70    */                \
            CM7_DOMAIN,   /* MIPI_REF        71    */                \
            CM7_DOMAIN,   /* MIPI_ESC        72    */                \
            CM7_DOMAIN,   /* CSI2            73    */                \
            CM7_DOMAIN,   /* CSI2_ESC        74    */                \
            CM7_DOMAIN,   /* CSI2_UI         75    */                \
            CM7_DOMAIN,   /* CSI             76    */                \
            CM7_DOMAIN,   /* CKO1            77    */                \
            CM7_DOMAIN                                               \
    } /* CKO2            78    */

#define CLOCK_ROOT_NUM (79U)
#define CLOCK_LPCG_NUM (138U)

typedef struct _lpcg_config
{
    uint8_t ctrlMode;
    uint16_t stbyValue;
    uint16_t spValue;
    clock_level_t level;
} lpcg_config_t;

#define PINMUX_DESCRIPTOR_TABLE                                                                                      \
    { /*address                                                              ,data ,size ,opreation ,type ,index                                                                                                              \
       */                                                                                                            \
        {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_05], 0,                                 \
         kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable, kSSARC_ReadValueWriteBack}, /*  0   */ \
            {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_06], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  1   */                                                                  \
            {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_07], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  2   */                                                                  \
            {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_08], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  3   */                                                                  \
            {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_09], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  4   */                                                                  \
            {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_10], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  5   */                                                                  \
            {(uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_11], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  6   */                                                                  \
            {(uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_DQS_FA_SELECT_INPUT], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  7   */                                                                  \
            {(uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_0], 0,                            \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  8   */                                                                  \
            {(uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_1], 0,                            \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  9   */                                                                  \
            {(uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_2], 0,                            \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  10  */                                                                  \
            {(uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_3], 0,                            \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  11  */                                                                  \
            {(uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_SCK_FA_SELECT_INPUT], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  12  */                                                                  \
            {(uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_05], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  13  */                                                                  \
            {(uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_06], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  14  */                                                                  \
            {(uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_07], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  15  */                                                                  \
            {(uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_08], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  16  */                                                                  \
            {(uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_09], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  17  */                                                                  \
            {(uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_10], 0,                             \
             kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable,                                    \
             kSSARC_ReadValueWriteBack}, /*  18  */                                                                  \
        {                                                                                                            \
            (uint32_t) & IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_11], 0,                            \
                kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveEnableRestoreEnable, kSSARC_ReadValueWriteBack       \
        }                                                                                                            \
    } /*  19  */

#define FLEXSPI_DESCRIPTOR_TABLE                                                                                      \
    { /*address                            ,data       ,size                                ,opreation ,type                                                                                                                \
         ,index */                                                                                                    \
        {(uint32_t)&FLEXSPI1->MCR0, 0xFFFF1010, kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveDisableRestoreEnable, \
         kSSARC_WriteFixedValue}, /*  0   */                                                                          \
            {(uint32_t)&FLEXSPI1->MCR0, 0x1, kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveDisableRestoreEnable,    \
             kSSARC_RMWOr}, /*  1   */                                                                                \
            {(uint32_t)&FLEXSPI1->MCR0, 0x1, kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveDisableRestoreEnable,    \
             kSSARC_Polling0}, /*  2   */                                                                             \
            {(uint32_t)&FLEXSPI1->MCR0, 0xFFFF1012, kSSARC_DescriptorRegister32bitWidth,                              \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  3   */                                     \
            {(uint32_t)&FLEXSPI1->MCR1, 0xFFFFFFFF, kSSARC_DescriptorRegister32bitWidth,                              \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  4   */                                     \
            {(uint32_t)&FLEXSPI1->MCR2, 0x200001F7, kSSARC_DescriptorRegister32bitWidth,                              \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  5   */                                     \
            {(uint32_t)&FLEXSPI1->AHBCR, 0x00000078, kSSARC_DescriptorRegister32bitWidth,                             \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  6   */                                     \
            {(uint32_t)&FLEXSPI1->AHBRXBUFCR0[0], 0x800F0000, kSSARC_DescriptorRegister32bitWidth,                    \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  7   */                                     \
            {(uint32_t)&FLEXSPI1->AHBRXBUFCR0[1], 0x800F0000, kSSARC_DescriptorRegister32bitWidth,                    \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  8   */                                     \
            {(uint32_t)&FLEXSPI1->AHBRXBUFCR0[2], 0x80000020, kSSARC_DescriptorRegister32bitWidth,                    \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  9   */                                     \
            {(uint32_t)&FLEXSPI1->AHBRXBUFCR0[3], 0x80000020, kSSARC_DescriptorRegister32bitWidth,                    \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  10  */                                     \
            {(uint32_t)&FLEXSPI1->IPRXFCR, 0x00000000, kSSARC_DescriptorRegister32bitWidth,                           \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  11  */                                     \
            {(uint32_t)&FLEXSPI1->IPTXFCR, 0x00000000, kSSARC_DescriptorRegister32bitWidth,                           \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  12  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR0[0], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  13  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR0[1], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  14  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR0[2], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  15  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR0[3], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  16  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR0[0], 0x00004000, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  17  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR1[0], 0x00020063, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  18  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR2[0], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                        \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  19  */                                     \
            {(uint32_t)&FLEXSPI1->DLLCR[0], 0x00000100, kSSARC_DescriptorRegister32bitWidth,                          \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  20  */                                     \
            {(uint32_t)&FLEXSPI1->MCR0, 0xFFFF1010, kSSARC_DescriptorRegister32bitWidth,                              \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  21  */                                     \
            {0, 200, kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveDisableRestoreEnable,                            \
             kSSARC_DelayCycles}, /*  22  */                                                                          \
            {(uint32_t)&FLEXSPI1->MCR0, 0xFFFF1012, kSSARC_DescriptorRegister32bitWidth,                              \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  23  */                                     \
            {(uint32_t)&FLEXSPI1->FLSHCR4, 0x00000003, kSSARC_DescriptorRegister32bitWidth,                           \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  24  */                                     \
            {(uint32_t)&FLEXSPI1->MCR0, 0xFFFF1010, kSSARC_DescriptorRegister32bitWidth,                              \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  25  */                                     \
            {(uint32_t)&FLEXSPI1->LUTKEY, 0x5AF05AF0, kSSARC_DescriptorRegister32bitWidth,                            \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  26  */                                     \
            {(uint32_t)&FLEXSPI1->LUTCR, 0x00000002, kSSARC_DescriptorRegister32bitWidth,                             \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  27  */                                     \
            {(uint32_t)&FLEXSPI1->LUT[0], 0x0a1804eb, kSSARC_DescriptorRegister32bitWidth,                            \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  28  */                                     \
            {(uint32_t)&FLEXSPI1->LUT[1], 0x26043206, kSSARC_DescriptorRegister32bitWidth,                            \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  29  */                                     \
            {(uint32_t)&FLEXSPI1->LUT[2], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                            \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  30  */                                     \
            {(uint32_t)&FLEXSPI1->LUT[3], 0x00000000, kSSARC_DescriptorRegister32bitWidth,                            \
             kSSARC_SaveDisableRestoreEnable, kSSARC_WriteFixedValue}, /*  31  */                                     \
            {(uint32_t)&FLEXSPI1->MCR0, 0x1, kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveDisableRestoreEnable,    \
             kSSARC_RMWOr}, /*  32  */                                                                                \
        {                                                                                                             \
            (uint32_t) & FLEXSPI1->MCR0, 0x1, kSSARC_DescriptorRegister32bitWidth, kSSARC_SaveDisableRestoreEnable,   \
                kSSARC_Polling0                                                                                       \
        }                                                                                                             \
    } /*  33  */

#define APP_TARGET_POWER_NUM (11U)
#define APP_POWER_NAME                                                                                \
    {                                                                                                 \
        "Setpoint0, CM7 domain WAIT", "Setpoint1, CM7 domain STOP", "Setpoint2, CM7 domain STOP",     \
            "Setpoint3, CM7 domain STOP", "Setpoint4, CM7 domain STOP", "Setpoint5, CM7 domain STOP", \
            "Setpoint6, CM7 domain STOP", "Setpoint7, CM7 domain STOP", "Setpoint8, CM7 domain STOP", \
            "Setpoint9, CM7 domain SUSPEND", "Setpoint10, CM7 domain SUSPEND",                        \
    }

/* To test all resources' constraints, user application no need to cover all constraints. */
#define APP_SETPOINT0_CONSTRAINTS                                                                                    \
    43U, PM_RESC_CORE_DOMAIN_WAIT, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,       \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_ON,                \
        PM_RESC_ARM_PLL_CLK_ON, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,              \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,           \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,       \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,          \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,        \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,                 \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                       \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_ON

#define APP_SETPOINT1_CONSTRAINTS                                                                                   \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,      \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_ON,               \
        PM_RESC_ARM_PLL_CLK_ON, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,             \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,          \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,      \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_ON, PM_RESC_SYS_PLL1_CLK_ON,           \
        PM_RESC_SYS_PLL1_DIV2_ON, PM_RESC_SYS_PLL1_DIV5_ON, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,         \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,                \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                      \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_ON, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_ON
#define APP_SETPOINT2_CONSTRAINTS                                                                                  \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,     \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_ON,              \
        PM_RESC_ARM_PLL_CLK_ON, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,            \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,         \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,     \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_ON, PM_RESC_SYS_PLL1_CLK_ON,          \
        PM_RESC_SYS_PLL1_DIV2_ON, PM_RESC_SYS_PLL1_DIV5_ON, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,        \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,               \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                     \
        PM_RESC_LPSR_DIG_LDO_ON, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_ON, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_ON
#define APP_SETPOINT3_CONSTRAINTS                                                                                  \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,     \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_ON,              \
        PM_RESC_ARM_PLL_CLK_ON, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,            \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,         \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,     \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_ON, PM_RESC_SYS_PLL1_CLK_ON,          \
        PM_RESC_SYS_PLL1_DIV2_ON, PM_RESC_SYS_PLL1_DIV5_ON, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,        \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,               \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                     \
        PM_RESC_LPSR_DIG_LDO_ON, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_ON, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_ON
#define APP_SETPOINT4_CONSTRAINTS                                                                                   \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,      \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_ON,               \
        PM_RESC_ARM_PLL_CLK_ON, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,             \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,          \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,      \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,         \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,       \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,                \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                      \
        PM_RESC_LPSR_DIG_LDO_ON, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_ON
#define APP_SETPOINT5_CONSTRAINTS                                                                                    \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,       \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_OFF,               \
        PM_RESC_ARM_PLL_CLK_OFF, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,             \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,           \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,       \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,          \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,        \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,                 \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                       \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_ON
#define APP_SETPOINT6_CONSTRAINTS                                                                                    \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,       \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_ON, PM_RESC_OSC_RC_24M_CLK_ON, PM_RESC_ARM_PLL_OFF,               \
        PM_RESC_ARM_PLL_CLK_OFF, PM_RESC_SYS_PLL2_ON, PM_RESC_SYS_PLL2_CLK_ON, PM_RESC_SYS_PLL2_PFD0_ON,             \
        PM_RESC_SYS_PLL2_PFD1_ON, PM_RESC_SYS_PLL2_PFD2_ON, PM_RESC_SYS_PLL2_PFD3_ON, PM_RESC_SYS_PLL3_ON,           \
        PM_RESC_SYS_PLL3_CLK_ON, PM_RESC_SYS_PLL3_DVI2_ON, PM_RESC_SYS_PLL3_PFD0_ON, PM_RESC_SYS_PLL3_PFD1_ON,       \
        PM_RESC_SYS_PLL3_PFD2_ON, PM_RESC_SYS_PLL3_PFD3_ON, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,          \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_ON, PM_RESC_AUDIO_PLL_CLK_ON,        \
        PM_RESC_VIDEO_PLL_ON, PM_RESC_VIDEO_PLL_CLK_ON, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,                 \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                       \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_OFF
#define APP_SETPOINT7_CONSTRAINTS                                                                                    \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,       \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_OFF, PM_RESC_OSC_RC_24M_CLK_OFF, PM_RESC_ARM_PLL_OFF,             \
        PM_RESC_ARM_PLL_CLK_OFF, PM_RESC_SYS_PLL2_OFF, PM_RESC_SYS_PLL2_CLK_OFF, PM_RESC_SYS_PLL2_PFD0_OFF,          \
        PM_RESC_SYS_PLL2_PFD1_OFF, PM_RESC_SYS_PLL2_PFD2_OFF, PM_RESC_SYS_PLL2_PFD3_OFF, PM_RESC_SYS_PLL3_OFF,       \
        PM_RESC_SYS_PLL3_CLK_OFF, PM_RESC_SYS_PLL3_DVI2_OFF, PM_RESC_SYS_PLL3_PFD0_OFF, PM_RESC_SYS_PLL3_PFD1_OFF,   \
        PM_RESC_SYS_PLL3_PFD2_OFF, PM_RESC_SYS_PLL3_PFD3_OFF, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,        \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_OFF, PM_RESC_AUDIO_PLL_CLK_OFF,      \
        PM_RESC_VIDEO_PLL_OFF, PM_RESC_VIDEO_PLL_CLK_OFF, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,               \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                       \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_OFF
#define APP_SETPOINT8_CONSTRAINTS                                                                                  \
    43U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,     \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_OFF, PM_RESC_OSC_RC_24M_CLK_OFF, PM_RESC_ARM_PLL_OFF,           \
        PM_RESC_ARM_PLL_CLK_OFF, PM_RESC_SYS_PLL2_OFF, PM_RESC_SYS_PLL2_CLK_OFF, PM_RESC_SYS_PLL2_PFD0_OFF,        \
        PM_RESC_SYS_PLL2_PFD1_OFF, PM_RESC_SYS_PLL2_PFD2_OFF, PM_RESC_SYS_PLL2_PFD3_OFF, PM_RESC_SYS_PLL3_OFF,     \
        PM_RESC_SYS_PLL3_CLK_OFF, PM_RESC_SYS_PLL3_DVI2_OFF, PM_RESC_SYS_PLL3_PFD0_OFF, PM_RESC_SYS_PLL3_PFD1_OFF, \
        PM_RESC_SYS_PLL3_PFD2_OFF, PM_RESC_SYS_PLL3_PFD3_OFF, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,      \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_OFF, PM_RESC_AUDIO_PLL_CLK_OFF,    \
        PM_RESC_VIDEO_PLL_OFF, PM_RESC_VIDEO_PLL_CLK_OFF, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,             \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                     \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_ON, PM_RESC_RBB_LPSR_ON, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_OFF
#define APP_SETPOINT9_CONSTRAINTS                                                                                    \
    43U, PM_RESC_CORE_DOMAIN_SUSPEND, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,    \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_OFF, PM_RESC_OSC_RC_24M_CLK_OFF, PM_RESC_ARM_PLL_OFF,             \
        PM_RESC_ARM_PLL_CLK_OFF, PM_RESC_SYS_PLL2_OFF, PM_RESC_SYS_PLL2_CLK_OFF, PM_RESC_SYS_PLL2_PFD0_OFF,          \
        PM_RESC_SYS_PLL2_PFD1_OFF, PM_RESC_SYS_PLL2_PFD2_OFF, PM_RESC_SYS_PLL2_PFD3_OFF, PM_RESC_SYS_PLL3_OFF,       \
        PM_RESC_SYS_PLL3_CLK_OFF, PM_RESC_SYS_PLL3_DVI2_OFF, PM_RESC_SYS_PLL3_PFD0_OFF, PM_RESC_SYS_PLL3_PFD1_OFF,   \
        PM_RESC_SYS_PLL3_PFD2_OFF, PM_RESC_SYS_PLL3_PFD3_OFF, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,        \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_OFF, PM_RESC_AUDIO_PLL_CLK_OFF,      \
        PM_RESC_VIDEO_PLL_OFF, PM_RESC_VIDEO_PLL_CLK_OFF, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,               \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                       \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_OFF, PM_RESC_RBB_LPSR_OFF, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_OFF
#define APP_SETPOINT10_CONSTRAINTS                                                                                 \
    43U, PM_RESC_CORE_DOMAIN_SUSPEND, PM_RESC_OSC_RC_16M_ON, PM_RESC_OSC_RC_48M_OFF, PM_RESC_OSC_RC_48M_DIV2_OFF,  \
        PM_RESC_OSC_RC_400M_ON, PM_RESC_OSC_RC_24M_OFF, PM_RESC_OSC_RC_24M_CLK_OFF, PM_RESC_ARM_PLL_OFF,           \
        PM_RESC_ARM_PLL_CLK_OFF, PM_RESC_SYS_PLL2_OFF, PM_RESC_SYS_PLL2_CLK_OFF, PM_RESC_SYS_PLL2_PFD0_OFF,        \
        PM_RESC_SYS_PLL2_PFD1_OFF, PM_RESC_SYS_PLL2_PFD2_OFF, PM_RESC_SYS_PLL2_PFD3_OFF, PM_RESC_SYS_PLL3_OFF,     \
        PM_RESC_SYS_PLL3_CLK_OFF, PM_RESC_SYS_PLL3_DVI2_OFF, PM_RESC_SYS_PLL3_PFD0_OFF, PM_RESC_SYS_PLL3_PFD1_OFF, \
        PM_RESC_SYS_PLL3_PFD2_OFF, PM_RESC_SYS_PLL3_PFD3_OFF, PM_RESC_SYS_PLL1_OFF, PM_RESC_SYS_PLL1_CLK_OFF,      \
        PM_RESC_SYS_PLL1_DIV2_OFF, PM_RESC_SYS_PLL1_DIV5_OFF, PM_RESC_AUDIO_PLL_OFF, PM_RESC_AUDIO_PLL_CLK_OFF,    \
        PM_RESC_VIDEO_PLL_OFF, PM_RESC_VIDEO_PLL_CLK_OFF, PM_RESC_MEGA_MIX_ON, PM_RESC_DISPLAY_MIX_ON,             \
        PM_RESC_WAKEUP_MIX_ON, PM_RESC_LPSR_MIX_ON, PM_RESC_DCDC_ON, PM_RESC_LPSR_ANA_LDO_OFF,                     \
        PM_RESC_LPSR_DIG_LDO_OFF, PM_RESC_BANDGAP_ON, PM_RESC_FBB_M7_OFF, PM_RESC_RBB_SOC_ON, PM_RESC_RBB_LPSR_ON, \
        PM_RESC_STANDBY_REQ_ON, PM_RESC_PLL_LDO_OFF

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME
#define APP_WAKEUP_BUTTON_WSID        PM_WSID_GPIO13_COMBINED_0_31_IRQ


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitWakeupSource(void);
void APP_RegisterNotify(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);
uint32_t APP_GetWakeupTimeout(void);

static uint8_t APP_GetTargetPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data);
pm_notify_element_t g_notify1 = {
    .notifyCallback = APP_UartControlCallback,
    .data           = NULL,
};

status_t APP_SetWakeupButton(pm_event_type_t eventType, uint8_t powerState, void *data);
pm_notify_element_t g_notify2 = {
    .notifyCallback = APP_SetWakeupButton,
    .data           = NULL,
};

status_t APP_PrintSetpointInfo(pm_event_type_t eventType, uint8_t powerState, void *data);
pm_notify_element_t g_notify3 = {
    .notifyCallback = APP_PrintSetpointInfo,
    .data           = NULL,
};

pm_wakeup_source_t g_buttonWakeup;
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);
static const char *const g_targetPowerNameArray[APP_TARGET_POWER_NUM] = APP_POWER_NAME;

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* Disable interrupt. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    }
    SDK_ISR_EXIT_BARRIER;
}

void APP_InitWakeupSource(void)
{
    PRINTF("Previous setpoint is %d.\r\n", GPC_SP_GetPreviousSetPoint(GPC_SET_POINT_CTRL));
    PRINTF("Current setpoint is %d.\r\n", GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL));
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);

    PM_InitWakeupSource(&g_buttonWakeup, APP_WAKEUP_BUTTON_WSID, NULL, true);
}

status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        /* De-init uart */
        PRINTF("De-init UART.\r\n");
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();
    }
    else
    {
        BOARD_InitPins();
        BOARD_InitDebugConsole();
        PRINTF("Re-init UART.\r\n");
    }

    return kStatus_Success;
}

status_t APP_SetWakeupButton(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);

        PM_InitWakeupSource(&g_buttonWakeup, APP_WAKEUP_BUTTON_WSID, NULL, true);
        PRINTF("Press button %s to wake up system.\r\n", APP_WAKEUP_BUTTON_NAME);
    }

    return kStatus_Success;
}

status_t APP_PrintSetpointInfo(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        PRINTF("Entering setpoint %d.\r\n", powerState);
    }
    else
    {
        switch (GPC_CM_GetPreviousCpuMode(GPC_CPU_MODE_CTRL_0))
        {
            case kGPC_WaitMode:
                PRINTF("Wakeup from wait mode.\r\n");
                break;
            case kGPC_StopMode:
                PRINTF("Wakeup from stop mode.\r\n");
                break;
            case kGPC_SuspendMode:
                PRINTF("Wakeup from suspend mode.\r\n");
                break;
            default:
                PRINTF("Wakeup from unknown mode.\r\n");
                break;
        }
        PRINTF("Previous setpoint is %d.\r\n", GPC_SP_GetPreviousSetPoint(GPC_SET_POINT_CTRL));
        PRINTF("Current setpoint is %d.\r\n", GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL));
    }

    return kStatus_Success;
}

void APP_RegisterNotify(void)
{
    PM_RegisterNotify(kPM_NotifyGroup2, &g_notify1);
    PM_RegisterNotify(kPM_NotifyGroup1, &g_notify2);
    PM_RegisterNotify(kPM_NotifyGroup0, &g_notify3);
}

uint32_t APP_GetWakeupTimeout(void)
{
    return 0UL;
}

void APP_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        // setpoint 0.
        case 0:
        {
            PM_SetConstraints(PM_LP_STATE_SP0, APP_SETPOINT0_CONSTRAINTS);
            break;
        }
        // setpoint 1.
        case 1:
        {
            PM_SetConstraints(PM_LP_STATE_SP1, APP_SETPOINT1_CONSTRAINTS);
            break;
        }
        // setpoint 2.
        case 2:
        {
            PM_SetConstraints(PM_LP_STATE_SP2, APP_SETPOINT2_CONSTRAINTS);
            break;
        }
        // setpoint 3.
        case 3:
        {
            PM_SetConstraints(PM_LP_STATE_SP3, APP_SETPOINT3_CONSTRAINTS);
            break;
        }
        // setpoint 4.
        case 4:
        {
            PM_SetConstraints(PM_LP_STATE_SP4, APP_SETPOINT4_CONSTRAINTS);
            break;
        }
        // setpoint 5.
        case 5:
        {
            PM_SetConstraints(PM_LP_STATE_SP5, APP_SETPOINT5_CONSTRAINTS);
            break;
        }
        // setpoint 6.
        case 6:
        {
            PM_SetConstraints(PM_LP_STATE_SP6, APP_SETPOINT6_CONSTRAINTS);
            break;
        }
        // setpoint 7.
        case 7:
        {
            PM_SetConstraints(PM_LP_STATE_SP7, APP_SETPOINT7_CONSTRAINTS);
            break;
        }
        // setpoint 8.
        case 8:
        {
            PM_SetConstraints(PM_LP_STATE_SP8, APP_SETPOINT8_CONSTRAINTS);
            break;
        }
        // setpoint 9.
        case 9:
        {
            PM_SetConstraints(PM_LP_STATE_SP9, APP_SETPOINT9_CONSTRAINTS);
            break;
        }
        // setpoint 10.
        case 10:
        {
            PM_SetConstraints(PM_LP_STATE_SP10, APP_SETPOINT10_CONSTRAINTS);
            break;
        }
        // default.
        default:
        {
            assert(false);
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case 0:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP0, APP_SETPOINT0_CONSTRAINTS);
            break;
        }
        case 1:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP1, APP_SETPOINT1_CONSTRAINTS);
            break;
        }
        case 2:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP2, APP_SETPOINT2_CONSTRAINTS);
            break;
        }
        case 3:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP3, APP_SETPOINT3_CONSTRAINTS);
            break;
        }
        case 4:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP4, APP_SETPOINT4_CONSTRAINTS);
            break;
        }
        case 5:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP5, APP_SETPOINT5_CONSTRAINTS);
            break;
        }
        case 6:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP6, APP_SETPOINT6_CONSTRAINTS);
            break;
        }
        case 7:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP7, APP_SETPOINT7_CONSTRAINTS);
            break;
        }
        case 8:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP8, APP_SETPOINT8_CONSTRAINTS);
            break;
        }
        case 9:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP9, APP_SETPOINT9_CONSTRAINTS);
            break;
        }
        case 10:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP10, APP_SETPOINT10_CONSTRAINTS);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
}

int main(void)
{
    uint32_t timeoutUs = 0UL;

    clock_root_config_t rootCfg = {0};

    BOARD_ConfigMPU();

    /* Workaround: Disable interrupt which might be enabled by ROM. */
    GPIO_DisableInterrupts(GPIO3, 1U << 24);
    GPIO_ClearPinsInterruptFlags(GPIO3, 1U << 24);

    /* Interrupt flags can't be cleared when wake up from SNVS mode, clear srtc and gpio interrupts here. */
    SNVS->LPSR |= SNVS_LPSR_LPTA_MASK;
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);

    BOARD_InitPins();

    rootCfg.mux = kCLOCK_LPUART1_ClockRoot_MuxOscRc16M;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart1, &rootCfg);

    BOARD_InitDebugConsole();

    DCDC_SetVDD1P0BuckModeTargetVoltage(DCDC, kDCDC_1P0BuckTarget1P0V);

    PMU_StaticEnableLpsrAnaLdoBypassMode(ANADIG_LDO_SNVS, true);
    PMU_StaticEnableLpsrDigLdoBypassMode(ANADIG_LDO_SNVS, true);

    const clock_sys_pll2_config_t sysPll2Config = {
        .ssEnable = false,
    };

    for (uint8_t i = 0; i < 79; i++)
    {
        /* Set clock root to unassigned mode so that software configuration can take effetcs. */
        CLOCK_ROOT_ControlByUnassignedMode((clock_root_t)i);
        /* For clock roots which mux to OscRc48MDiv2, mux to Rc16M so that application could turn off OscRc48MDiv2 to
         * save power. */
        if (CLOCK_GetRootClockMux((clock_root_t)i) == 0)
        {
            CLOCK_SetRootClockMux((clock_root_t)i, 3);
        }
    }
    ANADIG_OSC->OSC_48M_CTRL &= ~ANADIG_OSC_OSC_48M_CTRL_TEN_MASK;

    CLOCK_OSC_EnableOsc24M();
    rootCfg.mux = 1;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);

    CLOCK_OSC_EnableOscRc400M();
    CLOCK_InitArmPllWithFreq(700);
    CLOCK_InitSysPll2(&sysPll2Config);
    /* Init System Pll2 pfd3. */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    CLOCK_InitSysPll3();
    /* Init System Pll3 pfd3. */
    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd3, 22);

    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
    rootCfg.mux = kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg);
    rootCfg.mux = kCLOCK_M4_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);
    rootCfg.mux = kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);

    DCDC_BootIntoDCM(DCDC);

    uint32_t index;
    const clock_root_setpoint_config_t *spTable;
    static const clock_root_setpoint_config_t m7SpCfg[16]      = CLK_ROOT_M7_SP_TABLE;
    static const clock_root_setpoint_config_t m4SpCfg[16]      = CLK_ROOT_M4_SP_TABLE;
    static const clock_root_setpoint_config_t busSpCfg[16]     = CLK_ROOT_BUS_SP_TABLE;
    static const clock_root_setpoint_config_t busLpsrSpCfg[16] = CLK_ROOT_BUS_LPSR_SP_TABLE;
    static const uint8_t clkrootCfg[CLOCK_ROOT_NUM]            = CLK_ROOT_CONFIGURATION_TABLE;
    static const lpcg_config_t lpcgCfg[CLOCK_LPCG_NUM]         = CLK_LPCG_CONFIGURATION_TABLE;

    for (index = 0; index < CLOCK_ROOT_NUM; index++)
    {
        if (clkrootCfg[index] == SP_CTRL)
        {
            if (!CLOCK_ROOT_IsSetPointImplemented((clock_root_t)index))
            {
                assert(0);
            }
            switch (index)
            {
                case kCLOCK_Root_M7:
                    spTable = m7SpCfg;
                    break;
                case kCLOCK_Root_M4:
                    spTable = m4SpCfg;
                    break;
                case kCLOCK_Root_Bus:
                    spTable = busSpCfg;
                    break;
                case kCLOCK_Root_Bus_Lpsr:
                    spTable = busLpsrSpCfg;
                    break;
                default:
                    /* please prepare setpoint table for other clock roots. */
                    assert(0);
                    break;
            }
            CLOCK_ROOT_ControlBySetPointMode((clock_root_t)index, spTable);
        }
        else if (clkrootCfg[index] == CM7_DOMAIN)
        {
            CLOCK_ROOT_ControlByDomainMode((clock_root_t)index, CM7_DOMAIN);
        }
    }

    for (index = 0; index < CLOCK_LPCG_NUM; index++)
    {
        if (lpcgCfg[index].ctrlMode == SP_CTRL)
        {
            if (!CLOCK_LPCG_IsSetPointImplemented((clock_lpcg_t)index))
            {
                assert(0);
            }
            CLOCK_LPCG_ControlBySetPointMode((clock_lpcg_t)index, lpcgCfg[index].spValue, lpcgCfg[index].stbyValue);
        }
        else if (lpcgCfg[index].ctrlMode == CM7_DOMAIN)
        {
            CLOCK_LPCG_ControlByCpuLowPowerMode((clock_lpcg_t)index, CM7_DOMAIN, lpcgCfg[index].level,
                                                lpcgCfg[index].level);
        }
    }

    GPC_SET_POINT_CTRL->SP_ROSC_CTRL = 0xFFFFU;
    PRINTF("\r\nPower Manager Test.\r\n");
    PRINTF("\r\nNormal Boot.\r\n");
    PM_CreateHandle(&g_pmHandle);

    APP_RegisterNotify();
    APP_InitWakeupSource();
    while (1)
    {
        g_targetPowerMode = APP_GetTargetPowerMode();
        if (g_targetPowerMode >= APP_TARGET_POWER_NUM)
        {
            PRINTF("\r\nWrong Input! Please reselect.\r\n");
            continue;
        }
        PRINTF("Selected to enter %s.\r\n", g_targetPowerNameArray[(uint8_t)g_targetPowerMode]);
        timeoutUs = APP_GetWakeupTimeout();
        APP_SetConstraints(g_targetPowerMode);
        g_irqMask = DisableGlobalIRQ();
        PM_EnablePowerManager(true);
        PM_EnterLowPower(timeoutUs);
        PM_EnablePowerManager(false);
        EnableGlobalIRQ(g_irqMask);
        APP_ReleaseConstraints(g_targetPowerMode);
        PRINTF("\r\nNext Loop\r\n");
    }
}

static uint8_t APP_GetTargetPowerMode(void)
{
    uint32_t i;
    uint8_t ch;
    uint8_t g_targetPowerModeIndex;

    PRINTF("\r\nPlease select the desired power mode:\r\n");
    for (i = 0UL; i < APP_TARGET_POWER_NUM; i++)
    {
        PRINTF("\tPress %c to enter: %s\r\n", ('A' + i), g_targetPowerNameArray[i]);
    }

    PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

    ch = GETCHAR();

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    g_targetPowerModeIndex = ch - 'A';

    return g_targetPowerModeIndex;
}
