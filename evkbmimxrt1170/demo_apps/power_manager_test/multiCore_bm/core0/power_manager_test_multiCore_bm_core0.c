/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_mu.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_gpc.h"

#include "fsl_clock.h"
#include "fsl_dcdc.h"
#include "fsl_ssarc.h"
#include "fsl_pmu.h"
#include "fsl_lpuart.h"
#include "fsl_pgmc.h"
#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NA 31
// Domain assignment
#define UNASSIGNED 0
#define CM7_DOMAIN 1
#define CM4_DOMAIN 2
#define CM7_CTRL   CM7_DOMAIN
#define CM4_CTRL   CM4_DOMAIN
#define SP_CTRL    3

#define CM7_CLK_CTRL SP_CTRL
#define CM4_CLK_CTRL SP_CTRL

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

#define CLOCK_ROOT_NUM 79
#define CLK_ROOT_CONFIGURATION_TABLE                                 \
    {                     /*ctrl_mode,      name           index  */ \
        CM7_CLK_CTRL,     /* M7               0    */                \
            CM4_CLK_CTRL, /* M4               1    */                \
            SP_CTRL,      /* BUS              2    */                \
            SP_CTRL,      /* BUS_LPSR         3    */                \
            CM7_DOMAIN,   /* SEMC             4    */                \
            UNASSIGNED,   /* CSSYS            5    for debug*/       \
            UNASSIGNED,   /* CSTRACE          6    for debug*/       \
            CM4_DOMAIN,   /* M4_SYSTICK       7    */                \
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
            CM4_DOMAIN,   /* CAN3            24    */                \
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
            CM4_DOMAIN,   /* LPUART11        35    */                \
            CM4_DOMAIN,   /* LPUART12        36    */                \
            CM7_DOMAIN,   /* LPI2C1          37    */                \
            CM7_DOMAIN,   /* LPI2C2          38    */                \
            CM7_DOMAIN,   /* LPI2C3          39    */                \
            CM7_DOMAIN,   /* LPI2C4          40    */                \
            CM4_DOMAIN,   /* LPI2C5          41    */                \
            CM4_DOMAIN,   /* LPI2C6          42    */                \
            CM7_DOMAIN,   /* LPSPI1          43    */                \
            CM7_DOMAIN,   /* LPSPI2          44    */                \
            CM7_DOMAIN,   /* LPSPI3          45    */                \
            CM7_DOMAIN,   /* LPSPI4          46    */                \
            CM4_DOMAIN,   /* LPSPI5          47    */                \
            CM4_DOMAIN,   /* LPSPI6          48    */                \
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
            CM4_DOMAIN,   /* SAI4            67    */                \
            CM7_DOMAIN,   /* GC355           68    */                \
            CM7_DOMAIN,   /* LCDIF           69    */                \
            CM7_DOMAIN,   /* LCDIFV2         70    */                \
            CM7_DOMAIN,   /* MIPI_REF        71    */                \
            CM7_DOMAIN,   /* MIPI_ESC        72    */                \
            CM7_DOMAIN,   /* CSI2            73    */                \
            CM7_DOMAIN,   /* CSI2_ESC        74    */                \
            CM7_DOMAIN,   /* CSI2_UI         75    */                \
            CM7_DOMAIN,   /* CSI             76    */                \
            CM4_DOMAIN,   /* CKO1            77    */                \
            CM7_DOMAIN                                               \
    }                     /* CKO2            78    */

#define CLOCK_LPCG_NUM 138
#define CLK_LPCG_CONFIGURATION_TABLE                                                                   \
    { /*ctrlMode,    stbyValue,   spValue,     clock_level,       name,            index  */           \
        {CM7_CTRL, NA, NA, kCLOCK_Level1},               /* M7                0    */                  \
            {CM4_CTRL, NA, NA, kCLOCK_Level1},           /* M4                1    */                  \
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
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* WDOG4            17    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* EWM              18    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* SEMA             19    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level4},         /* MU_A             20    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level4},         /* MU_B             21    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* EDMA             22    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* EDMA_LPSR        23    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* ROMCP            24    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* OCRAM            25    */                  \
            {CM7_DOMAIN, 0x0000, 0xffff, kCLOCK_Level2}, /* FLEXRAM          26    */                  \
            {SP_CTRL, 0x0000, 0xffff, kCLOCK_Level2},    /* LMEM             27    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* FLEXSPI1         28    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* FLEXSPI2         29    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* RDC              30    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* M7_XRDC          31    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* M4_XRDC          32    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* SEMC             33    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* XECC             34    */                  \
            {CM7_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* IEE              35    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* KEY_MANAGER      36    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level2}, /* OCOTP            37    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* SNVS_HP          38    */                  \
            {CM4_DOMAIN, 0x0000, 0x07ff, kCLOCK_Level4}, /* SNVS             39    for GPIO13 access*/ \
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
            {CM4_DOMAIN, NA, NA, kCLOCK_Level4},         /* IOMUXC_LPSR      50    */                  \
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
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* CAN3             85    */                  \
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
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART11         96    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPUART12         97    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C1           98    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C2           99    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C3          100    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C4          101    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C5          102    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPI2C6          103    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI1          104    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI2          105    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI3          106    */                  \
            {CM7_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI4          107    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI5          108    */                  \
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* LPSPI6          109    */                  \
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
            {CM4_DOMAIN, NA, NA, kCLOCK_Level2},         /* SAI4            126    */                  \
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

typedef struct _lpcg_config
{
    uint8_t ctrlMode;
    uint16_t stbyValue;
    uint16_t spValue;
    clock_level_t level;
} lpcg_config_t;
#define APP_CORE1_BOOT_ADDRESS (0x20200000)

#define APP_TARGET_POWER_NUM (4U)
#define APP_CHN_MU_REG_NUM   0U
#define APP_POWER_NAME                                                                                \
    {                                                                                                 \
        "Setpoint0, CM7 domain WAIT, CM4 domain WAIT", "Setpoint1, CM7 domain STOP, CM4 domain WAIT", \
            "Setpoint10, CM7 domain SUSPEND, CM4 domain SUSPEND",                                     \
            "Setpoint15, CM7 domain SUSPEND, CM4 domain SUSPEND",                                     \
    }

#define APP_SETPOINT0_CONSTRAINTS  1U, PM_RESC_CORE_DOMAIN_WAIT
#define APP_SETPOINT1_CONSTRAINTS  3U, PM_RESC_CORE_DOMAIN_STOP, PM_RESC_SYS_PLL1_ON, PM_RESC_LPSR_DIG_LDO_OFF
#define APP_SETPOINT10_CONSTRAINTS 2U, PM_RESC_CORE_DOMAIN_SUSPEND, PM_RESC_WAKEUP_MIX_ON
#define APP_SETPOINT15_CONSTRAINTS 2U, PM_RESC_CORE_DOMAIN_SUSPEND, PM_RESC_OSC_RC_16M_ON

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME
#define APP_WAKEUP_BUTTON_WSID        PM_WSID_GPIO13_COMBINED_0_31_IRQ

#define APP_CHN_MU_REG_NUM 0U
#define BOOT_FLAG              0x01U
#define REV_TARGET_MODE_FLAG   0x02U
#define REV_TIMEOUT_VALUE_FLAG 0x03U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_GetPreviousState(void);
void APP_InitWakeupSource(void);
void APP_RegisterNotify(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);
uint32_t APP_GetWakeupTimeout(void);
void APP_BootSecondCore(void);
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

AT_QUICKACCESS_SECTION_CODE(void CLOCK_SetClockRoot(clock_root_t root, const clock_root_config_t *config));
AT_QUICKACCESS_SECTION_CODE(void SwitchFlexspiRootClock(bool pllLdoDisabled));

status_t APP_SwitchFlexspiRootClock(pm_event_type_t eventType, uint8_t powerState, void *data);
pm_notify_element_t g_notify4 = {
    .notifyCallback = APP_SwitchFlexspiRootClock,
    .data           = NULL,
};

pm_wakeup_source_t g_buttonWakeup;

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif defined(__GNUC__)
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t __Vectors[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__Vectors)
#elif defined(__MCUXPRESSO)
extern uint32_t __Vectors[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__Vectors)
#elif defined(__ICCARM__)
extern uint32_t __VECTOR_TABLE[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__VECTOR_TABLE)
#elif defined(__GNUC__)
extern uint32_t __VECTOR_TABLE[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)__VECTOR_TABLE)
#endif

AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);
static const char *const g_targetPowerNameArray[APP_TARGET_POWER_NUM] = APP_POWER_NAME;
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief ISR for wakeup button.
 */
void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* Disable interrupt. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        MU_TriggerInterrupts(MU_BASE, kMU_GenInt0InterruptTrigger);
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Callback function for g_notify1 to de-init uart before entering low power state and re-init uart after
 * exiting.
 */
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

/*!
 * @brief Callback function for g_notify2 to enable gpio interrupt.
 */

status_t APP_SetWakeupButton(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);

        // PM_InitWakeupSource(&g_buttonWakeup, APP_WAKEUP_BUTTON_WSID, NULL, true);
        PRINTF("Press button %s to wake up system.\r\n", APP_WAKEUP_BUTTON_NAME);
    }

    return kStatus_Success;
}

/*!
 * @brief Callback function for g_notify3 to print entry/exit of selected low power states.
 */
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

void CLOCK_SetClockRoot(clock_root_t root, const clock_root_config_t *config)
{
    assert(config);
    CCM->CLOCK_ROOT[root].CONTROL = CCM_CLOCK_ROOT_CONTROL_MUX(config->mux) |
                                    CCM_CLOCK_ROOT_CONTROL_DIV(config->div - 1) |
                                    (config->clockOff ? CCM_CLOCK_ROOT_CONTROL_OFF(config->clockOff) : 0);
    __DSB();
    __ISB();
#if __CORTEX_M == 4
    (void)CCM->CLOCK_ROOT[root].CONTROL;
#endif
}

void SwitchFlexspiRootClock(bool pllLdoDisabled)
{
    clock_root_config_t rootCfg = {0};

    if (pllLdoDisabled)
    {
        if (CLOCK_GetRootClockMux(kCLOCK_Root_Flexspi1) == kCLOCK_FLEXSPI1_ClockRoot_MuxOscRc16M)
        {
            /* Clock root already mux to OscRc16M, no need to config it again. */
            return;
        }
    }
    else
    {
        if (CLOCK_GetRootClockMux(kCLOCK_Root_Flexspi1) == kCLOCK_FLEXSPI1_ClockRoot_MuxSysPll2Out)
        {
            /* Clock root already mux to SysPll2Out, no need to config it again. */
            return;
        }
    }

    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
    }

#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))

    /* Enable FLEXSPI module */
    FLEXSPI1->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;
    while (!((FLEXSPI1->STS0 & FLEXSPI_STS0_ARBIDLE_MASK) && (FLEXSPI1->STS0 & FLEXSPI_STS0_SEQIDLE_MASK)))
    {
    }
    FLEXSPI1->MCR0 |= FLEXSPI_MCR0_MDIS_MASK;

    CCM->LPCG[kCLOCK_Flexspi1].DOMAINr = 0UL;
    /* Disable clock gate of flexspi. */
    CCM->LPCG[kCLOCK_Flexspi1].DIRECT &= ~CCM_LPCG_DIRECT_ON_MASK;

    __DSB();
    __ISB();

    while (CCM->LPCG[kCLOCK_Flexspi1].STATUS0 & CCM_LPCG_STATUS0_ON_MASK)
    {
    }

#endif

    if (pllLdoDisabled)
    {
        rootCfg.mux = kCLOCK_FLEXSPI1_ClockRoot_MuxOscRc16M;
        rootCfg.div = 1;
        CLOCK_SetClockRoot(kCLOCK_Root_Flexspi1, &rootCfg);
    }
    else
    {
        rootCfg.mux = kCLOCK_FLEXSPI1_ClockRoot_MuxSysPll2Out;
        rootCfg.div = 4;
        CLOCK_SetClockRoot(kCLOCK_Root_Flexspi1, &rootCfg);
    }

#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Enable clock gate of flexspi. */
    CCM->LPCG[kCLOCK_Flexspi1].DIRECT |= CCM_LPCG_DIRECT_ON_MASK;
    CCM->LPCG[kCLOCK_Flexspi1].DOMAINr |= 0x4UL;
    __DSB();
    __ISB();

    while (!(CCM->LPCG[kCLOCK_Flexspi1].STATUS0 & CCM_LPCG_STATUS0_ON_MASK))
    {
    }

    uint32_t status;
    uint32_t lastStatus;
    uint32_t retry;

    /* If serial root clock is >= 100 MHz, DLLEN set to 1, OVRDEN set to 0, then SLVDLYTARGET setting of 0x0 is
     * recommended. */
    FLEXSPI1->DLLCR[0] = 0x1U;

    /* Enable FLEXSPI module */
    FLEXSPI1->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;

    FLEXSPI1->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
    while (FLEXSPI1->MCR0 & FLEXSPI_MCR0_SWRESET_MASK)
    {
    }

    /* Need to wait DLL locked if DLL enabled */
    if (0U != (FLEXSPI1->DLLCR[0] & FLEXSPI_DLLCR_DLLEN_MASK))
    {
        lastStatus = FLEXSPI1->STS2;
        retry      = 10;
        /* Wait slave delay line locked and slave reference delay line locked. */
        do
        {
            status = FLEXSPI1->STS2;
            if ((status & (FLEXSPI_STS2_AREFLOCK_MASK | FLEXSPI_STS2_ASLVLOCK_MASK)) ==
                (FLEXSPI_STS2_AREFLOCK_MASK | FLEXSPI_STS2_ASLVLOCK_MASK))
            {
                /* Locked */
                retry = 100;
                break;
            }
            else if (status == lastStatus)
            {
                /* Same delay cell number in calibration */
                retry--;
            }
            else
            {
                retry      = 10;
                lastStatus = status;
            }
        } while (retry > 0);
        /* According to ERR011377, need to delay at least 100 NOPs to ensure the DLL is locked. */
        for (; retry > 0U; retry--)
        {
            __NOP();
        }
    }

    SCB_EnableICache();
#endif
}

/*!
 * @brief Callback function to switch flexspi root clock if current clock root is disabled in selected power state.
 */
status_t APP_SwitchFlexspiRootClock(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if ((powerState > PM_LP_STATE_SP10) && (eventType == kPM_EventEnteringSleep))
    {
        SwitchFlexspiRootClock(true);
    }
    else if ((powerState > PM_LP_STATE_SP10) && (eventType == kPM_EventExitingSleep))
    {
        SwitchFlexspiRootClock(false);
    }

    return kStatus_Success;
}

void APP_RegisterNotify(void)
{
    PM_RegisterNotify(kPM_NotifyGroup2, &g_notify1);
    PM_RegisterNotify(kPM_NotifyGroup1, &g_notify2);
    PM_RegisterNotify(kPM_NotifyGroup0, &g_notify3);
    PM_RegisterNotify(kPM_NotifyGroup2, &g_notify4);
}

void APP_GetPreviousState(void)
{
    PRINTF("Previous setpoint is %d.\r\n", GPC_SP_GetPreviousSetPoint(GPC_SET_POINT_CTRL));
    PRINTF("Current setpoint is %d.\r\n", GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL));
}

void APP_InitWakeupSource(void)
{
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);

    /* Set Wakeup Botton as the wakeup source. */
    PM_InitWakeupSource(&g_buttonWakeup, APP_WAKEUP_BUTTON_WSID, NULL, true);
}

uint32_t APP_GetWakeupTimeout(void)
{
    return 0UL;
}

void APP_SetConstraints(uint8_t powerState)
{
    switch (powerState)
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
        // setpoint 10.
        case 2:
        {
            PM_SetConstraints(PM_LP_STATE_SP10, APP_SETPOINT10_CONSTRAINTS);
            break;
        }
        // setpoint 15.
        case 3:
        {
            PM_SetConstraints(PM_LP_STATE_SP15, APP_SETPOINT15_CONSTRAINTS);
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

void APP_ReleaseConstraints(uint8_t powerState)
{
    switch (powerState)
    {
        case 0:
        {
            // Release setpoint 0 constraints.
            PM_ReleaseConstraints(PM_LP_STATE_SP0, APP_SETPOINT0_CONSTRAINTS);
            break;
        }
        case 1:
        {
            // Release setpoint 1 constraints.
            PM_ReleaseConstraints(PM_LP_STATE_SP1, APP_SETPOINT1_CONSTRAINTS);
            break;
        }
        case 2:
        {
            // Release setpoint 10 constraints.
            PM_ReleaseConstraints(PM_LP_STATE_SP10, APP_SETPOINT10_CONSTRAINTS);
            break;
        }
        case 3:
        {
            // Release setpoint 15 constraints.
            PM_ReleaseConstraints(PM_LP_STATE_SP15, APP_SETPOINT15_CONSTRAINTS);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
}

static uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}

void APP_BootSecondCore(void)
{
    if (GPC_CM_GetPreviousCpuMode(GPC_CPU_MODE_CTRL_0) == kGPC_RunMode)
    {
#ifdef CORE1_IMAGE_COPY_TO_RAM
        /* Calculate size of the image  - not required on MCUXpresso IDE. MCUXpresso copies the secondary core
           image to the target memory during startup automatically */
        uint32_t core1_image_size = get_core1_image_size();

        PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", APP_CORE1_BOOT_ADDRESS, core1_image_size);

        /* Copy Secondary core application from FLASH to the target memory. */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanInvalidateDCache_by_Addr((void *)APP_CORE1_BOOT_ADDRESS, core1_image_size);
#endif
        memcpy((void *)APP_CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanInvalidateDCache_by_Addr((void *)APP_CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif /* CORE1_IMAGE_COPY_TO_RAM */
        IOMUXC_LPSR_GPR->GPR0 = IOMUXC_LPSR_GPR_GPR0_CM4_INIT_VTOR_LOW(APP_CORE1_BOOT_ADDRESS >> 3);
        IOMUXC_LPSR_GPR->GPR1 = IOMUXC_LPSR_GPR_GPR1_CM4_INIT_VTOR_HIGH(APP_CORE1_BOOT_ADDRESS >> 16);

        /* Read back to make sure write takes effect. */
        (void)IOMUXC_LPSR_GPR->GPR0;
        (void)IOMUXC_LPSR_GPR->GPR1;

        /* If CM4 is already running (released by debugger), then reset it.
           If CM4 is not running, release it. */
        if ((SRC->SCR & SRC_SCR_BT_RELEASE_M4_MASK) == 0)
        {
            SRC_ReleaseCoreReset(SRC, kSRC_CM4Core);
        }
        else
        {
            SRC_AssertSliceSoftwareReset(SRC, kSRC_M4CoreSlice);
        }
    }
    else
    {
        MU_TriggerInterrupts(MU_BASE, kMU_GenInt0InterruptTrigger);
    }
}

int main(void)
{
    /* Init board hardware. */
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

    for (uint8_t i = 0; i < CLOCK_ROOT_NUM; i++)
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
        else if (clkrootCfg[index] == CM4_DOMAIN)
        {
            CLOCK_ROOT_ControlByDomainMode((clock_root_t)index, CM4_DOMAIN);
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
        else if (lpcgCfg[index].ctrlMode == CM4_DOMAIN)
        {
            CLOCK_LPCG_ControlByCpuLowPowerMode((clock_lpcg_t)index, CM4_DOMAIN, lpcgCfg[index].level,
                                                lpcgCfg[index].level);
        }
    }

    GPC_SET_POINT_CTRL->SP_ROSC_CTRL = 0xFFFFU;

    EnableIRQ(BOARD_USER_BUTTON_IRQ);
    PRINTF("\r\nPower Manager Multi-Core Bare-metal Demo.\r\n");
    PRINTF("\r\nPrimary Core Boot.\r\n");

    APP_GetPreviousState();

    /* Init MU. */
    MU_Init(MU_BASE);

    PRINTF("Starting Second Core...\r\n");
    APP_BootSecondCore();
    /* Wait for Second Core is booted. */
    while (BOOT_FLAG != MU_GetFlags(MU_BASE))
    {
    }
    PRINTF("Second Core application has been started.\r\n");

    /* Init Power Manager. */
    PM_CreateHandle(&g_pmHandle);
    APP_InitWakeupSource();
    APP_RegisterNotify();

    while (1)
    {
        g_targetPowerMode = APP_GetTargetPowerMode();
        if (g_targetPowerMode >= APP_TARGET_POWER_NUM)
        {
            PRINTF("\r\nWrong Input! Please reselect.\r\n");
            continue;
        }
        /* Send target power mode to second core. */
        MU_SendMsg(MU_BASE, APP_CHN_MU_REG_NUM, g_targetPowerMode);

        /* Waiting conform from second core. */
        while (REV_TARGET_MODE_FLAG != MU_GetFlags(MU_BASE))
        {
        }

        PRINTF("Selected to enter %s.\r\n", g_targetPowerNameArray[(uint8_t)g_targetPowerMode]);
        uint32_t timeoutUs = APP_GetWakeupTimeout();
        /* Send timeout value to second core. */
        MU_SendMsg(MU_BASE, APP_CHN_MU_REG_NUM, timeoutUs);
        /* Waiting conform from second core. */
        while (REV_TIMEOUT_VALUE_FLAG != MU_GetFlags(MU_BASE))
        {
        }
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
