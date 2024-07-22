/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CHIP_INIT_DEF_H__
#define __CHIP_INIT_DEF_H__

/* clang-format off */
#ifndef SINGLE_CORE_M33
#define CORE1_GET_INPUT_FROM_CORE0
#endif

#define PF5020_SWx_VOLT_0P8V (64U)
#define PF5020_SWx_VOLT_0P9V (80U)
#define PF5020_SWx_VOLT_1P0V (96U)
#define PF5020_SWx_VOLT_1P1V (112U)

#define NA                     31

// Domain assignment
#define UNASSIGNED             0
#define CM33_DOMAIN            2
#ifndef SINGLE_CORE_M33
#define CM7_DOMAIN             4
#else
#define CM7_DOMAIN             CM33_DOMAIN
#endif
#define CM33_CM7_DOMAIN        3

#define CM33_DOMAIN_MASK       (1UL << CM33_DOMAIN)
#define CM7_DOMAIN_MASK        (1UL << CM7_DOMAIN)

#define RUN_MODE_NUM       3
#define RUN_MODE_DEF_TABLE \
{/* Grade   DCDC                  CM7 FREQ                   CM33                                   EDGELOCK                                   BUS AON                                    BUS WAKEUP                                    WAKEUP AXI                                                    index*/ \
{   0     , PF5020_SWx_VOLT_1P1V, kDCDC_1P0Target1P1V ,800  ,kCLOCK_M33_ClockRoot_MuxSysPll3Out, 2 ,kCLOCK_EDGELOCK_ClockRoot_MuxOscRc400M, 2, kCLOCK_BUS_AON_ClockRoot_MuxSysPll2Out, 4, kCLOCK_BUS_WAKEUP_ClockRoot_MuxSysPll2Out, 4, kCLOCK_WAKEUP_AXI_ClockRoot_MuxSysPll3Out,  2}, /* Over Drive   0  */ \
{   1     , PF5020_SWx_VOLT_1P0V, kDCDC_1P0Target1P0V ,600  ,kCLOCK_M33_ClockRoot_MuxSysPll3Out, 2 ,kCLOCK_EDGELOCK_ClockRoot_MuxOscRc400M, 2, kCLOCK_BUS_AON_ClockRoot_MuxSysPll2Out, 4, kCLOCK_BUS_WAKEUP_ClockRoot_MuxSysPll2Out, 4, kCLOCK_WAKEUP_AXI_ClockRoot_MuxSysPll2Pfd1, 4}, /* Normal       1  */ \
{   2     , PF5020_SWx_VOLT_0P9V, kDCDC_1P0Target0P9V ,360  ,kCLOCK_M33_ClockRoot_MuxOscRc400M , 4 ,kCLOCK_EDGELOCK_ClockRoot_MuxOscRc400M, 6, kCLOCK_BUS_AON_ClockRoot_MuxOscRc400M,  8, kCLOCK_BUS_WAKEUP_ClockRoot_MuxOscRc400M,  8, kCLOCK_WAKEUP_AXI_ClockRoot_MuxSysPll2Pfd1, 8}} /* Under Drive  2  */ \

#define SRC_SLICE_NUM          6
#define SRC_CONFIGURATION_TABLE \
{/*sliceName,             ctrlMode,     power level,            name,           index*/\
{ AON_MIX_SLICE          ,CM33_DOMAIN   ,kSRC_PowerLevel4 }, /* AON_MIX           0  */\
{ CM7PLATFORM_MIX_SLICE  ,CM7_DOMAIN    ,kSRC_PowerLevel4 }, /* CM7PLATFORM_MIX   1  */\
{ CM33PLATFORM_MIX_SLICE ,CM33_DOMAIN   ,kSRC_PowerLevel4 }, /* CM33PLATFORM_MIX  2  */\
{ MEGA_MIX_SLICE         ,CM33_DOMAIN   ,kSRC_PowerLevel2 }, /* MEGA_MIX          3  */\
{ NETC_MIX_SLICE         ,CM33_DOMAIN   ,kSRC_PowerLevel2 }, /* NETC_MIX          4  */\
{ WAKEUP_MIX_SLICE       ,CM33_DOMAIN   ,kSRC_PowerLevel4 }} /* WAKEUP_MIX        5  */

#define CLOCK_OSCPLL_NUM           25
#define CLK_OSCPLL_CONFIGURATION_TABLE \
{/*ctrlMode,          clock_level,        name,            index*/ \
{  UNASSIGNED      , kCLOCK_Level3 }, /* OSC_RC_24M          0  */ \
{  CM33_DOMAIN     , kCLOCK_Level3 }, /* OSC_RC_400M         1  */ \
{  CM33_DOMAIN     , kCLOCK_Level3 }, /* OSC_24M             2  */ \
{  CM33_DOMAIN     , kCLOCK_Level3 }, /* OSC_24M_OUT         3  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* ARM_PLL             4  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* ARM_PLL_OUT         5  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL2            6  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL2_OUT        7  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL2_PFD0       8  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL2_PFD1       9  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL2_PFD2      10  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL2_PFD3      11  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3           12  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3_OUT       13  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3_DIV2      14  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3_PFD0      15  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3_PFD1      16  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3_PFD2      17  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL3_PFD3      18  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL1           19  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL1_OUT       20  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL1_DIV2      21  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* SYS_PLL1_DIV5      22  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }, /* AUDIO_PLL          23  */ \
{  CM33_DOMAIN     , kCLOCK_Level2 }} /* AUDIO_PLL_OUT      24  */

#ifndef SINGLE_CORE_M33
#define CM7_CLK_LEVEL       kCLOCK_Level3
#else
#define CM7_CLK_LEVEL       kCLOCK_Level4
#endif

#define CM33_CLK_LEVEL      kCLOCK_Level3
#define NETC_CLK_LEVEL      kCLOCK_Level2
#define EDGELOCK_CLK_LEVEL  kCLOCK_Level2
#define MEGA_CLK_LEVEL      kCLOCK_Level2
#define AON_CLK_LEVEL       kCLOCK_Level3
#define WAKEUP_CLK_LEVEL    kCLOCK_Level3
#define PUBLIC_CLK_LEVEL    kCLOCK_Level4

#define CLOCK_LPCG_NUM         149
#define CLK_LPCG_CONFIGURATION_TABLE \
{/*ctrlMode,    clock_level,            name,         index  */ \
{  CM7_DOMAIN   ,kCLOCK_Level1     }, /* M7             0    */ \
{  CM33_DOMAIN  ,kCLOCK_Level1     }, /* M33            1    */ \
{  CM33_DOMAIN  ,EDGELOCK_CLK_LEVEL}, /* Sentinel       2    */ \
{  CM33_DOMAIN  ,CM33_CLK_LEVEL    }, /* Sim_Aon        3    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Sim_Wakeup     4    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sim_Mega       5    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Sim_R          6    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Anadig         7    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Dcdc           8    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Src            9    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Ccm           10    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Gpc           11    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Adc1          12    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Adc2          13    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Dac           14    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Acmp1         15    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Acmp2         16    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Acmp3         17    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Acmp4         18    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Wdog1         19    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Wdog2         20    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Wdog3         21    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Wdog4         22    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Wdog5         23    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Ewm           24    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Sema1         25    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Sema2         26    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Mu_A          27    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Mu_B          28    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Edma1         29    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Edma2         30    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Romcp         31    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Ocram1        32    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Ocram2        33    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Flexspi1      34    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Flexspi2      35    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Flexspi_Slv   36    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Trdc          37    */ \
{  CM33_DOMAIN  ,EDGELOCK_CLK_LEVEL},/* Ocotp         38    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Semc          39    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Iee           40    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Cstrace       41    */ \
{  CM33_DOMAIN  ,PUBLIC_CLK_LEVEL  }, /* Csswo         42    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Iomuxc1       43    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Iomuxc2       44    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Gpio1         45    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Gpio2         46    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Gpio3         47    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Gpio4         48    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Gpio5         49    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Gpio6         50    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Flexio1       51    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Flexio2       52    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpit1         53    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpit2         54    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpit3         55    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lptmr1        56    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lptmr2        57    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lptmr3        58    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Tpm1          59    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Tpm2          60    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Tpm3          61    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Tpm4          62    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Tpm5          63    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Tpm6          64    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer1       65    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer2       66    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer3       67    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer4       68    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer5       69    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer6       70    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer7       71    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Qtimer8       72    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Gpt1          73    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Gpt2          74    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Syscount      75    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Can1          76    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Can2          77    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Can3          78    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpuart1       79    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpuart2       80    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart3       81    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart4       82    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart5       83    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart6       84    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpuart7       85    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart8       86    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart9       87    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart10      88    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpuart11      89    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpuart12      90    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpi2c1        91    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpi2c2        92    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpi2c3        93    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpi2c4        94    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpi2c5        95    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpi2c6        96    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpspi1        97    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Lpspi2        98    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpspi3        99    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpspi4       100    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpspi5       101    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Lpspi6       102    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* I3c1         103    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* I3c2         104    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Usdhc1       105    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Usdhc2       106    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Usb          107    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sinc1        108    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sinc2        109    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sinc3        110    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Xbar1        111    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Xbar2        112    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Xbar3        113    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Aoi1         114    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Aoi2         115    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Aoi3         116    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Aoi4         117    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Enc1         118    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Enc2         119    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Enc3         120    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Enc4         121    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Kpp          122    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Pwm1         123    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Pwm2         124    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Pwm3         125    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Pwm4         126    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Ecat         127    */ \
{  CM33_DOMAIN  ,NETC_CLK_LEVEL    }, /* Netc         128    */ \
{  CM33_DOMAIN  ,NETC_CLK_LEVEL    }, /* Serdes1      129    */ \
{  CM33_DOMAIN  ,NETC_CLK_LEVEL    }, /* Serdes2      130    */ \
{  CM33_DOMAIN  ,NETC_CLK_LEVEL    }, /* Serdes3      131    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Xcelbusx     132    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Xriocu4      133    */ \
{  CM33_DOMAIN  ,NETC_CLK_LEVEL    }, /* Sptp         134    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Mctrl        135    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Sai1         136    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sai2         137    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sai3         138    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Sai4         139    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Spdif        140    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Asrc         141    */ \
{  CM33_DOMAIN  ,MEGA_CLK_LEVEL    }, /* Mic          142    */ \
{  CM33_DOMAIN  ,WAKEUP_CLK_LEVEL  }, /* Vref         143    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }, /* Bist         144    */ \
{  CM7_DOMAIN   ,WAKEUP_CLK_LEVEL  }, /* Ssi_W2M7     145    */ \
{  CM7_DOMAIN   ,CM7_CLK_LEVEL     }, /* Ssi_M72W     146    */ \
{  CM7_DOMAIN   ,WAKEUP_CLK_LEVEL  }, /* Ssi_W2Ao     147    */ \
{  CM33_DOMAIN  ,AON_CLK_LEVEL     }} /* Ssi_Ao2W     148    */

/* clang-format on */
#endif
