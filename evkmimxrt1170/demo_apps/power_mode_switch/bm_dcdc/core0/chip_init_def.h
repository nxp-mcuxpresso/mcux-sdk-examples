/*
 * Copyright 2020, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CHIP_INIT_DEF_H__
#define __CHIP_INIT_DEF_H__

#include "setpoint_table_def.h"

/* clang-format off */
#ifndef SINGLE_CORE_M7
#define CORE1_GET_INPUT_FROM_CORE0
#endif

#define NA                     31

// Domain assignment
#define UNASSIGNED             0
#ifndef SINGLE_CORE_M7
#define CM7_DOMAIN             1
#define CM4_DOMAIN             2
#define CM7_CTRL               CM7_DOMAIN
#define CM4_CTRL               CM4_DOMAIN
#else
#define CM7_DOMAIN             1
#define CM4_DOMAIN             CM7_DOMAIN  //all domain controlled peripheral will follow CM7 domain
#define CM7_CTRL               CM7_DOMAIN
#define CM4_CTRL               UNASSIGNED
#endif
#define SP_CTRL                3

// Power control types
#define BPC                    0
#define CPC                    1
#define PPC                    2

#define BPC0                   0 // MEGAMIX
#define BPC1                   1 // DISPLAYMIX
#define BPC2                   2 // WAKEUPMIX
#define BPC3                   3 // LPSRMIX
#define BPC4                   4 // MIPIPHY
#define BPC5                   5 // NA
#define BPC6                   6 // NA
#define BPC7                   7 // NA
#define CPC0                   0 // M7
#define CPC1                   1 // M4
#define PPC0                   0 // PMIC

#define PD_TYPE_CORE           0
#define PD_TYPE_LMEM           1
#define PD_TYPE_PERIPH         2
#define PD_TYPE_PMIC           3

#define POWER_DOMAIN_NUM       10
#define PGMC_CONFIGURATION_TABLE \
{/*ctrlType,  domainType,     sliceID,   ctrlMode,    spConfig,              name,       index*/ \
{  BPC       ,PD_TYPE_PERIPH ,BPC0      ,SP_CTRL     ,PD_MEGA_SP_VAL   }, /* MEGAMIX       0  */ \
{  BPC       ,PD_TYPE_PERIPH ,BPC1      ,SP_CTRL     ,PD_DISP_SP_VAL   }, /* DISPLAYMIX    1  */ \
{  BPC       ,PD_TYPE_PERIPH ,BPC2      ,SP_CTRL     ,PD_WKUP_SP_VAL   }, /* WAKEUPMIX     2  */ \
{  BPC       ,PD_TYPE_PERIPH ,BPC3      ,SP_CTRL     ,PD_LPSR_SP_VAL   }, /* LPSRMIX       3  */ \
{  BPC       ,PD_TYPE_PERIPH ,BPC4      ,CM7_DOMAIN  ,NA               }, /* MIPIPHY       4  */ \
{  CPC       ,PD_TYPE_CORE   ,CPC0      ,CM7_CTRL    ,NA               }, /* M7CORE        5  */ \
{  CPC       ,PD_TYPE_LMEM   ,CPC0      ,SP_CTRL     ,PD_M7MEM_SP_VAL  }, /* M7MEM         6  */ \
{  CPC       ,PD_TYPE_CORE   ,CPC1      ,CM4_CTRL    ,NA               }, /* M4CORE        7  */ \
{  CPC       ,PD_TYPE_LMEM   ,CPC1      ,SP_CTRL     ,PD_M4MEM_SP_VAL  }, /* M4MEM         8  */ \
{  PPC       ,PD_TYPE_PMIC   ,PPC0      ,SP_CTRL     ,PD_PMIC_SP_VAL   }} /* PMIC          9  */

#define SRC_SLICE_NUM          12
#define SRC_CONFIGURATION_TABLE \
{/*sliceName,           ctrlMode,   spConfig,            name,        index*/\
{  kSRC_MegaSlice      ,SP_CTRL    ,PD_MEGA_SP_VAL }, /* MEGAMIX        0  */\
{  kSRC_DisplaySlice   ,SP_CTRL    ,PD_DISP_SP_VAL }, /* DISPLAYMIX     1  */\
{  kSRC_WakeUpSlice    ,SP_CTRL    ,PD_WKUP_SP_VAL }, /* WAKEUPMIX      2  */\
{  kSRC_LpsrSlice      ,SP_CTRL    ,PD_LPSR_SP_VAL }, /* LPSRMIX        3  */\
{  kSRC_M4CoreSlice    ,CM4_CTRL   ,NA             }, /* M4CORE         4  */\
{  kSRC_M7CoreSlice    ,CM7_CTRL   ,NA             }, /* M7CORE         5  */\
{  kSRC_M4DebugSlice   ,CM4_CTRL   ,NA             }, /* M4DBG          6  */\
{  kSRC_M7DebugSlice   ,CM7_CTRL   ,NA             }, /* M7DBG          7  */\
{  kSRC_Usbphy1Slice   ,SP_CTRL    ,PD_MEGA_SP_VAL }, /* USBPHY1        8  */\
{  kSRC_Usbphy2Slice   ,SP_CTRL    ,PD_MEGA_SP_VAL }} /* USBPHY2        9  */

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
#define CM4_DOMAIN_CLOCK_MODE  kCLOCK_Level4
#else
#define CM4_DOMAIN_CLOCK_MODE  kCLOCK_Level2
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */

#define CLOCK_LPCG_NUM         138
#define CLK_LPCG_CONFIGURATION_TABLE \
{/*ctrlMode,    stbyValue,   spValue,     clock_level,       name,            index  */ \
{  CM7_CTRL    ,NA          ,NA          ,kCLOCK_Level1 }, /* M7                0    */ \
{  CM4_CTRL    ,NA          ,NA          ,kCLOCK_Level1 }, /* M4                1    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* SIM_M7            2    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* SIM_M             3    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* SIM_DISP          4    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* SIM_PER           5    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* SIM_LPSR          6    */ \
{  SP_CTRL     ,0xffff      ,0xffff      ,kCLOCK_Level2 }, /* ANADIG            7    */ \
{  SP_CTRL     ,0xffff      ,0xffff      ,kCLOCK_Level2 }, /* DCDC              8    */ \
{  SP_CTRL     ,0xffff      ,0xffff      ,kCLOCK_Level2 }, /* SRC               9    */ \
{  SP_CTRL     ,0xffff      ,0xffff      ,kCLOCK_Level2 }, /* CCM              10    */ \
{  SP_CTRL     ,0xffff      ,0xffff      ,kCLOCK_Level2 }, /* GPC              11    */ \
{  SP_CTRL     ,0xffff      ,0xffff      ,kCLOCK_Level2 }, /* SSARC            12    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SIM_R            13    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* WDOG1            14    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* WDOG2            15    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* WDOG3            16    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* WDOG4            17    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* EWM              18    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* SEMA             19    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level4 }, /* MU_A             20    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level4 }, /* MU_B             21    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* EDMA             22    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* EDMA_LPSR        23    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* ROMCP            24    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* OCRAM            25    */ \
{  CM7_DOMAIN  ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* FLEXRAM          26    */ \
{  SP_CTRL     ,0x0000      ,0xffff      ,kCLOCK_Level2 }, /* LMEM             27    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* FLEXSPI1         28    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* FLEXSPI2         29    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* RDC              30    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* M7_XRDC          31    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* M4_XRDC          32    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* SEMC             33    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* XECC             34    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* IEE              35    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* KEY_MANAGER      36    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* OCOTP            37    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* SNVS_HP          38    */ \
{  CM4_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level4 }, /* SNVS             39    for GPIO13 access*/ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* CAAM             40    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* JTAG_MUX         41    */ \
{  UNASSIGNED  ,NA          ,NA          ,kCLOCK_Level2 }, /* CSTRACE          42    for debug*/ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* XBAR1            43    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* XBAR2            44    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* XBAR3            45    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* AOI1             46    */ \
{  CM7_DOMAIN  ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* AOI2             47    */ \
{  SP_CTRL     ,0x0000      ,0x07ff      ,kCLOCK_Level2 }, /* ADC_ETC          48    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level4 }, /* IOMUXC           49    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level4 }, /* IOMUXC_LPSR      50    */ \
{  UNASSIGNED  ,NA          ,NA          ,kCLOCK_Level4 }, /* GPIO             51    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* KPP              52    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* FLEXIO1          53    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* FLEXIO2          54    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ADC1             55    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ADC2             56    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* DAC              57    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ACMP1            58    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ACMP2            59    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ACMP3            60    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ACMP4            61    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* PIT1             62    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* PIT2             63    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPT1             64    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPT2             65    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPT3             66    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPT4             67    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPT5             68    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPT6             69    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* QTIMER1          70    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* QTIMER2          71    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* QTIMER3          72    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* QTIMER4          73    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENC1             74    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENC2             75    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENC3             76    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENC4             77    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* HRTIMER          78    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* FLEXPWM1         79    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* FLEXPWM2         80    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* FLEXPWM3         81    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* FLEXPWM4         82    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* CAN1             83    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* CAN2             84    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* CAN3             85    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART1          86    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART2          87    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART3          88    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART4          89    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART5          90    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART6          91    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART7          92    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART8          93    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART9          94    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART10         95    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART11         96    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPUART12         97    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPI2C1           98    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPI2C2           99    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPI2C3          100    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPI2C4          101    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPI2C5          102    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,CM4_DOMAIN_CLOCK_MODE }, /* LPI2C6          103    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPSPI1          104    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPSPI2          105    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPSPI3          106    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPSPI4          107    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPSPI5          108    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LPSPI6          109    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SIM1            110    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SIM2            111    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENET            112    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENET_1G         113    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ENET_QOS        114    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* USB             115    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SDIO            116    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* USDHC1          117    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* USDHC2          118    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* ASRC            119    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* MQS             120    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* MIC             121    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SPDIF           122    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SAI1            123    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SAI2            124    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SAI3            125    */ \
{  CM4_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* SAI4            126    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* PXP             127    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* GPU2D           128    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LCDIF           129    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* LCDIFV2         130    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* MIPI_DSI        131    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* MIPI_CSI        132    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* CSI             133    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* DCIC_MIPI       134    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* DCIC_LCD        135    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }, /* VIDEO_MUX       136    */ \
{  CM7_DOMAIN  ,NA          ,NA          ,kCLOCK_Level2 }} /* UNIQ_EDT_I      137    */

#define CM7_CLK_CTRL   SP_CTRL
#ifndef SINGLE_CORE_M7
#define CM4_CLK_CTRL   SP_CTRL
#else
#define CM4_CLK_CTRL   UNASSIGNED
#endif

#define CLOCK_ROOT_NUM             79
#define CLK_ROOT_CONFIGURATION_TABLE \
{/*ctrl_mode,      name           index  */ \
   CM7_CLK_CTRL,/* M7               0    */ \
   CM4_CLK_CTRL,/* M4               1    */ \
   SP_CTRL   ,  /* BUS              2    */ \
   SP_CTRL   ,  /* BUS_LPSR         3    */ \
   CM7_DOMAIN,  /* SEMC             4    */ \
   UNASSIGNED,  /* CSSYS            5    for debug*/ \
   UNASSIGNED,  /* CSTRACE          6    for debug*/ \
   CM4_DOMAIN,  /* M4_SYSTICK       7    */ \
   CM7_DOMAIN,  /* M7_SYSTICK       8    */ \
   CM7_DOMAIN,  /* ADC1             9    */ \
   CM7_DOMAIN,  /* ADC2            10    */ \
   CM7_DOMAIN,  /* ACMP            11    */ \
   CM7_DOMAIN,  /* FLEXIO1         12    */ \
   CM7_DOMAIN,  /* FLEXIO2         13    */ \
   CM7_DOMAIN,  /* GPT1            14    */ \
   CM7_DOMAIN,  /* GPT2            15    */ \
   CM7_DOMAIN,  /* GPT3            16    */ \
   CM7_DOMAIN,  /* GPT4            17    */ \
   CM7_DOMAIN,  /* GPT5            18    */ \
   CM7_DOMAIN,  /* GPT6            19    */ \
   CM7_DOMAIN,  /* FLEXSPI1        20    */ \
   CM7_DOMAIN,  /* FLEXSPI2        21    */ \
   CM7_DOMAIN,  /* CAN1            22    */ \
   CM7_DOMAIN,  /* CAN2            23    */ \
   CM4_DOMAIN,  /* CAN3            24    */ \
   CM7_DOMAIN,  /* LPUART1         25    */ \
   CM7_DOMAIN,  /* LPUART2         26    */ \
   CM7_DOMAIN,  /* LPUART3         27    */ \
   CM7_DOMAIN,  /* LPUART4         28    */ \
   CM7_DOMAIN,  /* LPUART5         29    */ \
   CM7_DOMAIN,  /* LPUART6         30    */ \
   CM7_DOMAIN,  /* LPUART7         31    */ \
   CM7_DOMAIN,  /* LPUART8         32    */ \
   CM7_DOMAIN,  /* LPUART9         33    */ \
   CM7_DOMAIN,  /* LPUART10        34    */ \
   CM4_DOMAIN,  /* LPUART11        35    */ \
   CM4_DOMAIN,  /* LPUART12        36    */ \
   CM7_DOMAIN,  /* LPI2C1          37    */ \
   CM7_DOMAIN,  /* LPI2C2          38    */ \
   CM7_DOMAIN,  /* LPI2C3          39    */ \
   CM7_DOMAIN,  /* LPI2C4          40    */ \
   CM4_DOMAIN,  /* LPI2C5          41    */ \
   CM4_DOMAIN,  /* LPI2C6          42    */ \
   CM7_DOMAIN,  /* LPSPI1          43    */ \
   CM7_DOMAIN,  /* LPSPI2          44    */ \
   CM7_DOMAIN,  /* LPSPI3          45    */ \
   CM7_DOMAIN,  /* LPSPI4          46    */ \
   CM4_DOMAIN,  /* LPSPI5          47    */ \
   CM4_DOMAIN,  /* LPSPI6          48    */ \
   CM7_DOMAIN,  /* EMV1            49    */ \
   CM7_DOMAIN,  /* EMV2            50    */ \
   CM7_DOMAIN,  /* ENET1           51    */ \
   CM7_DOMAIN,  /* ENET2           52    */ \
   CM7_DOMAIN,  /* ENET_QOS        53    */ \
   CM7_DOMAIN,  /* ENET_25M        54    */ \
   CM7_DOMAIN,  /* ENET_TIMER1     55    */ \
   CM7_DOMAIN,  /* ENET_TIMER2     56    */ \
   CM7_DOMAIN,  /* ENET_TIMER3     57    */ \
   CM7_DOMAIN,  /* USDHC1          58    */ \
   CM7_DOMAIN,  /* USDHC2          59    */ \
   CM7_DOMAIN,  /* ASRC            60    */ \
   CM7_DOMAIN,  /* MQS             61    */ \
   CM7_DOMAIN,  /* MIC             62    */ \
   CM7_DOMAIN,  /* SPDIF           63    */ \
   CM7_DOMAIN,  /* SAI1            64    */ \
   CM7_DOMAIN,  /* SAI2            65    */ \
   CM7_DOMAIN,  /* SAI3            66    */ \
   CM4_DOMAIN,  /* SAI4            67    */ \
   CM7_DOMAIN,  /* GC355           68    */ \
   CM7_DOMAIN,  /* LCDIF           69    */ \
   CM7_DOMAIN,  /* LCDIFV2         70    */ \
   CM7_DOMAIN,  /* MIPI_REF        71    */ \
   CM7_DOMAIN,  /* MIPI_ESC        72    */ \
   CM7_DOMAIN,  /* CSI2            73    */ \
   CM7_DOMAIN,  /* CSI2_ESC        74    */ \
   CM7_DOMAIN,  /* CSI2_UI         75    */ \
   CM7_DOMAIN,  /* CSI             76    */ \
   CM4_DOMAIN,  /* CKO1            77    */ \
   CM7_DOMAIN}  /* CKO2            78    */

#define CLOCK_OSCPLL_NUM           29
// NOTE: RC16M is not controlled by CCM. Just use a configuration to let LP sequence go ahead.
#define CLK_OSCPLL_CONFIGURATION_TABLE \
{/*ctrlMode,    stbyValue               , spValue               , clock_level,        name,               index*/ \
{  UNASSIGNED  ,OSC_RC_16M_STBY_VAL     , OSC_RC_16M_SP_VAL     , kCLOCK_Level1 }, /* OSC_RC_16M            0  */ \
{  SP_CTRL     ,OSC_RC_48M_STBY_VAL     , OSC_RC_48M_SP_VAL     , kCLOCK_Level1 }, /* OSC_RC_48M            1  */ \
{  SP_CTRL     ,OSC_RC_48M_DIV2_STBY_VAL, OSC_RC_48M_DIV2_SP_VAL, kCLOCK_Level1 }, /* OSC_RC_48M_DIV2       2  */ \
{  SP_CTRL     ,OSC_RC_400M_STBY_VAL    , OSC_RC_400M_SP_VAL    , kCLOCK_Level1 }, /* OSC_RC_400M           3  */ \
{  SP_CTRL     ,OSC_24M_STBY_VAL        , OSC_24M_SP_VAL        , kCLOCK_Level1 }, /* OSC_24M               4  */ \
{  SP_CTRL     ,OSC_24M_OUT_STBY_VAL    , OSC_24M_OUT_SP_VAL    , kCLOCK_Level1 }, /* OSC_24M_OUT           5  */ \
{  SP_CTRL     ,PLL_ARM_STBY_VAL        , PLL_ARM_SP_VAL        , kCLOCK_Level1 }, /* PLL_ARM               6  */ \
{  SP_CTRL     ,PLL_ARM_OUT_STBY_VAL    , PLL_ARM_OUT_SP_VAL    , kCLOCK_Level1 }, /* PLL_ARM_OUT           7  */ \
{  SP_CTRL     ,PLL_528_STBY_VAL        , PLL_528_SP_VAL        , kCLOCK_Level1 }, /* PLL_528               8  */ \
{  SP_CTRL     ,PLL_528_OUT_STBY_VAL    , PLL_528_OUT_SP_VAL    , kCLOCK_Level1 }, /* PLL_528_OUT           9  */ \
{  SP_CTRL     ,PLL_528_PFD0_STBY_VAL   , PLL_528_PFD0_SP_VAL   , kCLOCK_Level1 }, /* PLL_528_PFD0         10  */ \
{  SP_CTRL     ,PLL_528_PFD1_STBY_VAL   , PLL_528_PFD1_SP_VAL   , kCLOCK_Level1 }, /* PLL_528_PFD1         11  */ \
{  SP_CTRL     ,PLL_528_PFD2_STBY_VAL   , PLL_528_PFD2_SP_VAL   , kCLOCK_Level1 }, /* PLL_528_PFD2         12  */ \
{  SP_CTRL     ,PLL_528_PFD3_STBY_VAL   , PLL_528_PFD3_SP_VAL   , kCLOCK_Level1 }, /* PLL_528_PFD3         13  */ \
{  SP_CTRL     ,PLL_480_STBY_VAL        , PLL_480_SP_VAL        , kCLOCK_Level1 }, /* PLL_480              14  */ \
{  SP_CTRL     ,PLL_480_OUT_STBY_VAL    , PLL_480_OUT_SP_VAL    , kCLOCK_Level1 }, /* PLL_480_OUT          15  */ \
{  SP_CTRL     ,PLL_480_DIV2_STBY_VAL   , PLL_480_DIV2_SP_VAL   , kCLOCK_Level1 }, /* PLL_480_DIV2         16  */ \
{  SP_CTRL     ,PLL_480_PFD0_STBY_VAL   , PLL_480_PFD0_SP_VAL   , kCLOCK_Level1 }, /* PLL_480_PFD0         17  */ \
{  SP_CTRL     ,PLL_480_PFD1_STBY_VAL   , PLL_480_PFD1_SP_VAL   , kCLOCK_Level1 }, /* PLL_480_PFD1         18  */ \
{  SP_CTRL     ,PLL_480_PFD2_STBY_VAL   , PLL_480_PFD2_SP_VAL   , kCLOCK_Level1 }, /* PLL_480_PFD2         19  */ \
{  SP_CTRL     ,PLL_480_PFD3_STBY_VAL   , PLL_480_PFD3_SP_VAL   , kCLOCK_Level1 }, /* PLL_480_PFD3         20  */ \
{  SP_CTRL     ,PLL_1G_STBY_VAL         , PLL_1G_SP_VAL         , kCLOCK_Level1 }, /* PLL_1G               21  */ \
{  SP_CTRL     ,PLL_1G_OUT_STBY_VAL     , PLL_1G_OUT_SP_VAL     , kCLOCK_Level1 }, /* PLL_1G_OUT           22  */ \
{  SP_CTRL     ,PLL_1G_DIV2_STBY_VAL    , PLL_1G_DIV2_SP_VAL    , kCLOCK_Level1 }, /* PLL_1G_DIV2          23  */ \
{  SP_CTRL     ,PLL_1G_DIV5_STBY_VAL    , PLL_1G_DIV5_SP_VAL    , kCLOCK_Level1 }, /* PLL_1G_DIV5          24  */ \
{  SP_CTRL     ,PLL_AUDIO_STBY_VAL      , PLL_AUDIO_SP_VAL      , kCLOCK_Level1 }, /* PLL_AUDIO            25  */ \
{  SP_CTRL     ,PLL_AUDIO_OUT_STBY_VAL  , PLL_AUDIO_OUT_SP_VAL  , kCLOCK_Level1 }, /* PLL_AUDIO_OUT        26  */ \
{  SP_CTRL     ,PLL_VIDEO_STBY_VAL      , PLL_VIDEO_SP_VAL      , kCLOCK_Level1 }, /* PLL_VIDEO            27  */ \
{  SP_CTRL     ,PLL_VIDEO_OUT_STBY_VAL  , PLL_VIDEO_OUT_SP_VAL  , kCLOCK_Level1 }} /* PLL_VIDEO_OUT        28  */

/* clang-format on */
#endif
