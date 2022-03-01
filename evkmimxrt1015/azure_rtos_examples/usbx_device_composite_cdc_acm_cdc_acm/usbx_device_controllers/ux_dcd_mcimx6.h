/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   MCIMX6 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_dcd_mcimx6.h                                     PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the equivalences for the MCIMX6 FREESCALE UDC     */
/*    controller.                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/

#ifndef UX_DCD_MCIMX6_H
#define UX_DCD_MCIMX6_H


/* Define MCIMX6 generic equivalences.  */

#define UX_DCD_MCIMX6_SLAVE_CONTROLLER                                 0x80
#define UX_DCD_MCIMX6_8BIT_REG                                         8
#define UX_DCD_MCIMX6_16BIT_REG                                        16
#define UX_DCD_MCIMX6_32BIT_REG                                        32
#define UX_DCD_MCIMX6_MAX_ED                                           16
#define UX_DCD_MCIMX6_MAX_ED_VALUE                                     8
#define UX_DCD_MCIMX6_MAX_QTD                                          32
#define UX_DCD_MCIMX6_MAX_QTD_TRANSFER                                 (16*1024)
#define UX_DCD_MCIMX6_RESET_RETRY                                      1000
#define UX_DCD_MCIMX6_PAGE_SIZE                                        4096
#define UX_DCD_MCIMX6_PAGE_ALIGN                                       0xfffff000

/* Define MCIMX6 device controller memory map.  */

#define UX_DCD_MCIMX6_IDENTIFICATION_REGISTER                          0x00000000
#define UX_DCD_MCIMX6_HW_GENERAL                                       0x00000004
#define UX_DCD_MCIMX6_HW_HOST                                          0x00000008
#define UX_DCD_MCIMX6_HW_DEVICE                                        0x0000000C
#define UX_DCD_MCIMX6_HW_TX_BUF                                        0x00000010
#define UX_DCD_MCIMX6_HW_RX_BUF                                        0x00000004
#define UX_DCD_MCIMX6_DCI_VERSION                                      0x00000120
#define UX_DCD_MCIMX6_DCC_PARAMS                                       0x00000124
#define UX_DCD_MCIMX6_USBCMD                                           0x00000140
#define UX_DCD_MCIMX6_USBSTS                                           0x00000144
#define UX_DCD_MCIMX6_USBINTR                                          0x00000148
#define UX_DCD_MCIMX6_FRINDEX                                          0x0000014C
#define UX_DCD_MCIMX6_DEVICE_ADDRESS                                   0x00000154
#define UX_DCD_MCIMX6_EP_LIST_ADDR                                     0x00000158
#define UX_DCD_MCIMX6_BURST_SIZE                                       0x00000160
#define UX_DCD_MCIMX6_ULPI_VIEWPORT                                    0x00000170
#define UX_DCD_MCIMX6_PORTSC                                           0x00000184
#define UX_DCD_MCIMX6_OTGSC                                            0x000001A4
#define UX_DCD_MCIMX6_USB_MODE                                         0x000001A8
#define UX_DCD_MCIMX6_EPSETUPSR                                        0x000001AC
#define UX_DCD_MCIMX6_EPPRIME                                          0x000001B0
#define UX_DCD_MCIMX6_EPFLUSH                                          0x000001B4
#define UX_DCD_MCIMX6_EPSR                                             0x000001B8
#define UX_DCD_MCIMX6_EPCOMPLETE                                       0x000001BC
#define UX_DCD_MCIMX6_EPCR                                             0x000001C0

/* Define MCIMX6 device controller hardware general parameters.  */

#define UX_DCD_MCIMX6_HW_GENERAL_PHY_WIDTH_MASK                        0x00000030
#define UX_DCD_MCIMX6_HW_GENERAL_PHY_MODE_MASK                         0x000000C0

/* Define MCIMX6 device controller transmit buffer hardware parameters.  */

#define UX_DCD_MCIMX6_HW_TX_BUF_TXBURTS_MASK                           0x000000FF
#define UX_DCD_MCIMX6_HW_TX_BUF_TXADD_MASK                             0x0000FF00
#define UX_DCD_MCIMX6_HW_TX_BUF_TXCHANNADD_MASK                        0x00FF0000
#define UX_DCD_MCIMX6_HW_TX_BUF_TXLC_FIFO                              0x00000000
#define UX_DCD_MCIMX6_HW_TX_BUF_TXLC_REGISTER                          0x80000000


/* Define MCIMX6 device controller Command register.  */

#define UX_DCD_MCIMX6_USBMD_RS                                         0x00000001
#define UX_DCD_MCIMX6_USBCMD_RST                                       0x00000002

/* Define MCIMX6 device controller command register.  */

#define UX_DCD_MCIMX6_USBCMD_RS                                        0x00000001
#define UX_DCD_MCIMX6_USBCMD_RST                                       0x00000002
#define UX_DCD_MCIMX6_USBCMD_FS0                                       0x00000004
#define UX_DCD_MCIMX6_USBCMD_FS1                                       0x00000008
#define UX_DCD_MCIMX6_USBCMD_PSE                                       0x00000010
#define UX_DCD_MCIMX6_USBCMD_ASE                                       0x00000020
#define UX_DCD_MCIMX6_USBCMD_IAA                                       0x00000040
#define UX_DCD_MCIMX6_USBCMD_LR                                        0x00000080
#define UX_DCD_MCIMX6_USBCMD_ASP_MASK                                  0x00000300
#define UX_DCD_MCIMX6_USBCMD_ASP_SHIFT                                 7
#define UX_DCD_MCIMX6_USBCMD_ASPE                                      0x00000800
#define UX_DCD_MCIMX6_USBCMD_SUTW                                      0x00002000
#define UX_DCD_MCIMX6_USBCMD_ATDTW                                     0x00004000
#define UX_DCD_MCIMX6_USBCMD_FS2                                       0x00008000
#define UX_DCD_MCIMX6_USBCMD_ITC_MASK                                  0x00FF0000
#define UX_DCD_MCIMX6_USBCMD_ITC_SHIFT                                 15

/* Define MCIMX6 device controller FRINDEX register.  */

#define UX_DCD_MCIMX6_FRINDEX_SHIFT                                    3

/* Define MCIMX6 device controller status register.  */

#define UX_DCD_MCIMX6_USBSTS_UI                                        0x00000001
#define UX_DCD_MCIMX6_USBSTS_UEI                                       0x00000002
#define UX_DCD_MCIMX6_USBSTS_PCI                                       0x00000004
#define UX_DCD_MCIMX6_USBSTS_FRI                                       0x00000008
#define UX_DCD_MCIMX6_USBSTS_SEI                                       0x00000010
#define UX_DCD_MCIMX6_USBSTS_AAI                                       0x00000020
#define UX_DCD_MCIMX6_USBSTS_URI                                       0x00000040
#define UX_DCD_MCIMX6_USBSTS_SRI                                       0x00000080
#define UX_DCD_MCIMX6_USBSTS_SLI                                       0x00000100
#define UX_DCD_MCIMX6_USBSTS_HCH                                       0x00001000
#define UX_DCD_MCIMX6_USBSTS_RCL                                       0x00002000
#define UX_DCD_MCIMX6_USBSTS_PS                                        0x00004000
#define UX_DCD_MCIMX6_USBSTS_AS                                        0x00008000
#define UX_DCD_MCIMX6_USBSTS_NAK                                       0x00010000

#define UX_DCD_MCIMX6_USBSTS_MASK                                      (UX_DCD_MCIMX6_USBSTS_PCI |       \
                                                                         UX_DCD_MCIMX6_USBSTS_URI  |      \
                                                                         UX_DCD_MCIMX6_USBSTS_SRI  |      \
                                                                         UX_DCD_MCIMX6_USBSTS_SLI  |      \
                                                                         UX_DCD_MCIMX6_USBSTS_UI   |      \
                                                                         UX_DCD_MCIMX6_USBSTS_UEI)




/* Define MCIMX6 device controller interrupt enable register.  */

#define UX_DCD_MCIMX6_USBINTR_UE                                       0x00000001
#define UX_DCD_MCIMX6_USBINTR_UEE                                      0x00000002
#define UX_DCD_MCIMX6_USBINTR_PCE                                      0x00000004
#define UX_DCD_MCIMX6_USBINTR_FRE                                      0x00000008
#define UX_DCD_MCIMX6_USBINTR_SEE                                      0x00000010
#define UX_DCD_MCIMX6_USBINTR_AAE                                      0x00000020
#define UX_DCD_MCIMX6_USBINTR_URE                                      0x00000040
#define UX_DCD_MCIMX6_USBINTR_SRE                                      0x00000080
#define UX_DCD_MCIMX6_USBINTR_SLE                                      0x00000100
#define UX_DCD_MCIMX6_USBINTR_ULPIE                                    0x00000400
#define UX_DCD_MCIMX6_USBINTR_NAKE                                     0x00004000
#define UX_DCD_MCIMX6_USBINTR_MASK                                     (UX_DCD_MCIMX6_USBINTR_PCE |       \
                                                                         UX_DCD_MCIMX6_USBINTR_URE  |      \
                                                                         UX_DCD_MCIMX6_USBINTR_UE   |      \
                                                                         UX_DCD_MCIMX6_USBINTR_UEE)

/* Define MCIMX6 device controller Device Address register.  */

#define UX_DCD_MCIMX6_DEVICEADDR_MASK                                  0xF7000000
#define UX_DCD_MCIMX6_DEVICEADDR_SHIFT                                 25

/* Define MCIMX6 device controller Endpoint List Address register.  */

#define UX_DCD_MCIMX6_EPLISTADDR_MASK                                  0xFFFFF800
#define UX_DCD_MCIMX6_EPLISTADDR_SHIFT                                 10
#define UX_DCD_MCIMX6_EPLISTADDR_ALIGN                                 2048

/* Define MCIMX6 device controller OTGSC registe value.  */

#define UX_DCD_MCIMX6_OTGSC_VD                                         0x00000001
#define UX_DCD_MCIMX6_OTGSC_VC                                         0x00000002
#define UX_DCD_MCIMX6_OTGSC_OT                                         0x00000008
#define UX_DCD_MCIMX6_OTGSC_DP                                         0x00000010
#define UX_DCD_MCIMX6_OTGSC_IDPU                                       0x00000020
#define UX_DCD_MCIMX6_OTGSC_ID                                         0x00000100
#define UX_DCD_MCIMX6_OTGSC_AVV                                        0x00000200
#define UX_DCD_MCIMX6_OTGSC_ASV                                        0x00000400
#define UX_DCD_MCIMX6_OTGSC_BSV                                        0x00000800
#define UX_DCD_MCIMX6_OTGSC_BSE                                        0x00001000
#define UX_DCD_MCIMX6_OTGSC_1MST                                       0x00002000
#define UX_DCD_MCIMX6_OTGSC_DPS                                        0x00004000
#define UX_DCD_MCIMX6_OTGSC_IDIS                                       0x00010000
#define UX_DCD_MCIMX6_OTGSC_AVVIS                                      0x00020000
#define UX_DCD_MCIMX6_OTGSC_ASVIS                                      0x00040000
#define UX_DCD_MCIMX6_OTGSC_BSVIS                                      0x00080000
#define UX_DCD_MCIMX6_OTGSC_BSEIS                                      0x00100000
#define UX_DCD_MCIMX6_OTGSC_1MSS                                       0x00200000
#define UX_DCD_MCIMX6_OTGSC_DPIS                                       0x00400000
#define UX_DCD_MCIMX6_OTGSC_IDIE                                       0x01000000
#define UX_DCD_MCIMX6_OTGSC_AVVIE                                      0x02000000
#define UX_DCD_MCIMX6_OTGSC_ASVIE                                      0x04000000
#define UX_DCD_MCIMX6_OTGSC_BSVIE                                      0x08000000
#define UX_DCD_MCIMX6_OTGSC_BSEIE                                      0x10000000
#define UX_DCD_MCIMX6_OTGSC_1MSE                                       0x20000000
#define UX_DCD_MCIMX6_OTGSC_DPIE                                       0x40000000

/* Define MCIMX6 device controller USBMODE registe value.  */

#define UX_DCD_MCIMX6_PORTSC_CCS                                       0x00000001
#define UX_DCD_MCIMX6_PORTSC_CSC                                       0x00000002
#define UX_DCD_MCIMX6_PORTSC_PE                                        0x00000004
#define UX_DCD_MCIMX6_PORTSC_PEC                                       0x00000008
#define UX_DCD_MCIMX6_PORTSC_OCA                                       0x00000010
#define UX_DCD_MCIMX6_PORTSC_OCC                                       0x00000020
#define UX_DCD_MCIMX6_PORTSC_FPR                                       0x00000040
#define UX_DCD_MCIMX6_PORTSC_SUSP                                      0x00000080
#define UX_DCD_MCIMX6_PORTSC_PR                                        0x00000100
#define UX_DCD_MCIMX6_PORTSC_HSP                                       0x00000200
#define UX_DCD_MCIMX6_PORTSC_LSJ                                       0x00000400
#define UX_DCD_MCIMX6_PORTSC_LSK                                       0x00000800
#define UX_DCD_MCIMX6_PORTSC_PP                                        0x00001000
#define UX_DCD_MCIMX6_PORTSC_PO                                        0x00002000
#define UX_DCD_MCIMX6_PORTSC_WLCN                                      0x00100000
#define UX_DCD_MCIMX6_PORTSC_WKDS                                      0x00200000
#define UX_DCD_MCIMX6_PORTSC_WKOK                                      0x00400000
#define UX_DCD_MCIMX6_PORTSC_PHCD                                      0x00800000
#define UX_DCD_MCIMX6_PORTSC_PFSC                                      0x01000000
#define UX_DCD_MCIMX6_PORTSC_PSPD_MASK                                 0x0CC00000
#define UX_DCD_MCIMX6_PORTSC_PSPD_FULL                                 0x00000000
#define UX_DCD_MCIMX6_PORTSC_PSPD_LOW                                  0x04000000
#define UX_DCD_MCIMX6_PORTSC_PSPD_HIGH                                 0x08000000
#define UX_DCD_MCIMX6_PORTSC_PTS_MASK                                  0xC0000000
#define UX_DCD_MCIMX6_PORTSC_PTS_ULPI                                  0x80000000
#define UX_DCD_MCIMX6_PORTSC_PTS_ONT                                   0xC0000000


/* Define MCIMX6 device controller USBMODE registe value.  */

#define UX_DCD_MCIMX6_USBMODE_CM_DEVICE                                0x00000002
#define UX_DCD_MCIMX6_USBMODE_CM_HOST                                  0x00000003
#define UX_DCD_MCIMX6_USBMODE_ES                                       0x00000004
#define UX_DCD_MCIMX6_USBMODE_SLOM                                     0x00000008
#define UX_DCD_MCIMX6_USBMODE_SDIS                                     0x00000010

/* Define MCIMX6 device controller EPSETUPSR field descriptor.  */

#define UX_DCD_MCIMX6_EPSETUPSR_EPSETPSTAT_MASK                        0x0000000F
#define UX_DCD_MCIMX6_EPSETUPSR_PERB_SHIFT                             0

/* Define MCIMX6 device controller EPPRIME field descriptor.  */

#define UX_DCD_MCIMX6_EPPRIME_PERB_MASK                                0x0000000F
#define UX_DCD_MCIMX6_EPPRIME_PERB_SHIFT                               0
#define UX_DCD_MCIMX6_EPPRIME_PETB_MASK                                0x000F0000
#define UX_DCD_MCIMX6_EPPRIME_PETB_SHIFT                               16

/* Define MCIMX6 device controller EPFLUSH field descriptor.  */

#define UX_DCD_MCIMX6_EPFLUSH_FERB_MASK                                0x0000000F
#define UX_DCD_MCIMX6_EPFLUSH_FERB_SHIFT                               0
#define UX_DCD_MCIMX6_EPFLUSH_FETB_MASK                                0x000F0000
#define UX_DCD_MCIMX6_EPFLUSH_FETB_SHIFT                               16

/* Define MCIMX6 device controller EPSR field descriptor.  */

#define UX_DCD_MCIMX6_EPSR_ERBR_MASK                                   0x0000000F
#define UX_DCD_MCIMX6_EPSR_ERBR_SHIFT                                  0
#define UX_DCD_MCIMX6_EPSR_ETBR_MASK                                   0x000F0000
#define UX_DCD_MCIMX6_EPSR_ETBR_SHIFT                                  16

/* Define MCIMX6 device controller EPCOMPLETE field descriptor.  */

#define UX_DCD_MCIMX6_EPCOMPLETE_ERCE_MASK                             0x0000000F
#define UX_DCD_MCIMX6_EPCOMPLETE_ERCE_SHIFT                            0
#define UX_DCD_MCIMX6_EPCOMPLETE_ETCE_MASK                             0x000F0000
#define UX_DCD_MCIMX6_EPCOMPLETE_ETCE_SHIFT                            16

/* Define MCIMX6 device controller Endpoint 0 control register.  */

#define UX_DCD_MCIMX6_EPCR0_RXS                                        0x00000001
#define UX_DCD_MCIMX6_EPCR0_TXS                                        0x00000100

/* Define MCIMX6 device controller Endpoint x control register.  */

#define UX_DCD_MCIMX6_EPCRN_RXS                                        0x00000001
#define UX_DCD_MCIMX6_EPCRN_RXTCTRL                                    0x00000000
#define UX_DCD_MCIMX6_EPCRN_RXTISO                                     0x00000004
#define UX_DCD_MCIMX6_EPCRN_RXTBULK                                    0x00000008
#define UX_DCD_MCIMX6_EPCRN_RXTINT                                     0x0000000C
#define UX_DCD_MCIMX6_EPCRN_RXI                                        0x00000020
#define UX_DCD_MCIMX6_EPCRN_RXR                                        0x00000040
#define UX_DCD_MCIMX6_EPCRN_RXE                                        0x00000080
#define UX_DCD_MCIMX6_EPCRN_TXS                                        0x00010000
#define UX_DCD_MCIMX6_EPCRN_TXTCTRL                                    0x00000000
#define UX_DCD_MCIMX6_EPCRN_TXTISO                                     0x00040000
#define UX_DCD_MCIMX6_EPCRN_TXTBULK                                    0x00080000
#define UX_DCD_MCIMX6_EPCRN_TXTINT                                     0x000C0000
#define UX_DCD_MCIMX6_EPCRN_TXI                                        0x00200000
#define UX_DCD_MCIMX6_EPCRN_TXR                                        0x00400000
#define UX_DCD_MCIMX6_EPCRN_TXE                                        0x00800000

/* Define USB MCIMX6 physical endpoint status definition.  */

#define UX_DCD_MCIMX6_ED_STATUS_UNUSED                                 0
#define UX_DCD_MCIMX6_ED_STATUS_USED                                   1
#define UX_DCD_MCIMX6_ED_STATUS_TRANSFER                               2
#define UX_DCD_MCIMX6_ED_STATUS_STALLED                                4

/* Define USB MCIMX6 physical endpoint types.  */

#define UX_DCD_MCIMX6_ED_TYPE_CONTROL                                  0
#define UX_DCD_MCIMX6_ED_TYPE_ISO                                      1
#define UX_DCD_MCIMX6_ED_TYPE_BULK                                     2
#define UX_DCD_MCIMX6_ED_TYPE_INT                                      3

/* Define USB MCIMX6 physical QED bits definition.  */

#define UX_DCD_MCIMX6_QED_MULT_N                                       0x00000000
#define UX_DCD_MCIMX6_QED_MULT_1                                       0x40000000
#define UX_DCD_MCIMX6_QED_MULT_2                                       0x80000000
#define UX_DCD_MCIMX6_QED_MULT_3                                       0xC0000000
#define UX_DCD_MCIMX6_QED_ZLT_ENABLE                                   0x00000000
#define UX_DCD_MCIMX6_QED_ZLT_DISABLE                                  0x20000000
#define UX_DCD_MCIMX6_QED_MPL_MASK                                     0x07F00000
#define UX_DCD_MCIMX6_QED_MPL_SHIFT                                    16
#define UX_DCD_MCIMX6_QED_IOS                                          0x00008000
#define UX_DCD_MCIMX6_QED_QTD_MASK                                     0xFFFFFFE0
#define UX_DCD_MCIMX6_QED_TERMINATE                                    0x00000001

/* Define USB MCIMX6 physical QED structure.  */

typedef struct UX_DCD_MCIMX6_QED_STRUCT
{

    ULONG           ux_dcd_mcimx6_qed_control;
    struct UX_DCD_MCIMX6_QTD_STRUCT           *ux_dcd_mcimx6_qed_current_qtd;
    struct UX_DCD_MCIMX6_QTD_STRUCT           *ux_dcd_mcimx6_qed_next_qtd;
    ULONG           ux_dcd_mcimx6_qed_status;
    ULONG           ux_dcd_mcimx6_qed_bp[5];
    ULONG           ux_dcd_mcimx6_qed_reserved;
    ULONG           ux_dcd_mcimx6_qed_setup_buffer_03;
    ULONG           ux_dcd_mcimx6_qed_setup_buffer_47;
    ULONG           ux_dcd_mcimx6_qed_padding[4];
} UX_DCD_MCIMX6_QED;


/* Define USB MCIMX6 physical QTD bits definition.  */

#define UX_DCD_MCIMX6_QTD_TERMINATE                                    0x00000001
#define UX_DCD_MCIMX6_QTD_TOTAL_BYTES_MASK                             0x3FFF0000
#define UX_DCD_MCIMX6_QTD_TOTAL_BYTES_SHIFT                            16
#define UX_DCD_MCIMX6_QTD_IOC                                          0x00008000
#define UX_DCD_MCIMX6_QTD_STATUS_MASK                                  0x00000FFF
#define UX_DCD_MCIMX6_QTD_STATUS_SHIFT                                 0
#define UX_DCD_MCIMX6_QTD_FRAME_NUMBER_MASK                            0x000007FF
#define UX_DCD_MCIMX6_QTD_FRAME_NUMBER_SHIFT                           0


/* Define USB MCIMX6 QED status bit definition.  */

#define UX_DCD_MCIMX6_QTD_STATUS_ACTIVE                                0x00000080
#define UX_DCD_MCIMX6_QTD_STATUS_HALTED                                0x00000040
#define UX_DCD_MCIMX6_QTD_STATUS_DATA_BUFFER_ERROR                     0x00000020
#define UX_DCD_MCIMX6_QTD_STATUS_TRANSACTION_ERROR                     0x00000008
#define UX_DCD_MCIMX6_QTD_STATUS_FREE                                  0x00000000
#define UX_DCD_MCIMX6_QTD_STATUS_USED                                  0x00000001
#define UX_DCD_MCIMX6_QTD_STATUS_PHASE                                 0x00000002

/* Define USB MCIMX6 physical QTD structure.  */

typedef struct UX_DCD_MCIMX6_QTD_STRUCT
{
    struct UX_DCD_MCIMX6_QTD_STRUCT
                    *ux_dcd_mcimx6_qtd_next_qtd;
    ULONG           ux_dcd_mcimx6_qtd_status;
    UCHAR           *ux_dcd_mcimx6_qtd_bp[5];
    USHORT          ux_dcd_mcimx6_qtd_transfer_length;
    USHORT          ux_dcd_mcimx6_qtd_control;
} UX_DCD_MCIMX6_QTD;

/* Define control state  definition.  */

#define UX_DCD_MCIMX6_CONTROL_STATE_IDLE                              0
#define UX_DCD_MCIMX6_CONTROL_STATE_DATA_TX                           1
#define UX_DCD_MCIMX6_CONTROL_STATE_DATA_RX                           2
#define UX_DCD_MCIMX6_CONTROL_STATE_STATUS_TX                         3
#define UX_DCD_MCIMX6_CONTROL_STATE_STATUS_RX                         4

/* Define USB MCIMX6 logical endpoint structure.  */

typedef struct UX_DCD_MCIMX6_ED_STRUCT
{
    ULONG           ux_dcd_mcimx6_ed_status;
    ULONG           ux_dcd_mcimx6_ed_type;
    ULONG           ux_dcd_mcimx6_ed_address;
    ULONG           ux_dcd_mcimx6_ed_direction;
    ULONG           ux_dcd_mcimx6_ed_payload_length;
    struct UX_SLAVE_ENDPOINT_STRUCT
                    *ux_dcd_mcimx6_ed_endpoint;
    struct UX_DCD_MCIMX6_QED_STRUCT
                    *ux_dcd_mcimx6_ed_qed;
    struct UX_DCD_MCIMX6_QTD_STRUCT
                    *ux_dcd_mcimx6_ed_qtd_head;
    struct UX_DCD_MCIMX6_QTD_STRUCT
                    *ux_dcd_mcimx6_ed_qtd_tail;

} UX_DCD_MCIMX6_ED;


/* Define USB MCIMX6 DCD structure definition.  */

typedef struct UX_DCD_MCIMX6_STRUCT
{

    struct UX_SLAVE_DCD_STRUCT
                    *ux_dcd_mcimx6_dcd_owner;
    struct UX_DCD_MCIMX6_ED_STRUCT
                    ux_dcd_mcimx6_ed[UX_DCD_MCIMX6_MAX_ED];
    struct UX_DCD_MCIMX6_QED_STRUCT
                    *ux_dcd_mcimx6_qed_head;
    struct UX_DCD_MCIMX6_QTD_STRUCT
                    *ux_dcd_mcimx6_qtd_head;
    ULONG           ux_dcd_mcimx6_qtd_mask;
    ULONG           ux_dcd_mcimx6_base;
    ULONG           ux_dcd_mcimx6_debug;
    VOID            *ux_dcd_mcimx6_thread_stack;
    TX_THREAD       ux_dcd_mcimx6_thread;
    TX_SEMAPHORE    ux_dcd_mcimx6_semaphore;
    ULONG           ux_dcd_mcimx6_interrupt;
    ULONG           ux_dcd_mcimx6_control_state;

} UX_DCD_MCIMX6;


/* Define USB MCIMX6 DCD prototypes.  */

UINT    _ux_dcd_mcimx6_address_set(UX_DCD_MCIMX6 *dcd_mcimx6,ULONG address);
UINT    _ux_dcd_mcimx6_endpoint_create(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_mcimx6_endpoint_destroy(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_mcimx6_endpoint_reset(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_mcimx6_endpoint_stall(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint);
UINT    _ux_dcd_mcimx6_endpoint_status(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG endpoint_index);
UINT    _ux_dcd_mcimx6_frame_number_get(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG *frame_number);
UINT    _ux_dcd_mcimx6_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter);
UINT    _ux_dcd_mcimx6_initialize(ULONG dcd_io);
UINT    _ux_dcd_mcimx6_initialize_complete(VOID);
VOID    _ux_dcd_mcimx6_interrupt_handler(VOID);
UINT    _ux_dcd_mcimx6_transfer_abort(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_TRANSFER *transfer_request);
VOID    _ux_dcd_mcimx6_register_clear(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG width, ULONG mcimx6_register, ULONG value);
ULONG   _ux_dcd_mcimx6_register_read(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG width, ULONG mcimx6_register);
VOID    _ux_dcd_mcimx6_register_set(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG width, ULONG mcimx6_register, ULONG value);
VOID    _ux_dcd_mcimx6_register_write(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG width, ULONG mcimx6_register, ULONG value);
UINT    _ux_dcd_mcimx6_state_change(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG state);
UINT    _ux_dcd_mcimx6_transfer_callback(UX_DCD_MCIMX6 *dcd_mcimx6, UX_DCD_MCIMX6_ED *ed, ULONG callback_phase);
UINT    _ux_dcd_mcimx6_transfer_request(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_TRANSFER *transfer_request);
UINT    _ux_dcd_mcimx6_endpoint_flush(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint, ULONG endpoint_direction);
UINT    _ux_dcd_mcimx6_endpoint_stall_clear(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint);
UX_DCD_MCIMX6_ED *  _ux_dcd_mcimx6_endpoint_address_get(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint, ULONG endpoint_direction);
VOID    _ux_dcd_mcimx6_interrupt_thread(ULONG dcd_pointer);
UX_DCD_MCIMX6_QTD *  _ux_dcd_mcimx6_qtd_get(UX_DCD_MCIMX6 *dcd_mcimx6);
UINT    _ux_dcd_mcimx6_status_phase_hook(UX_DCD_MCIMX6 *dcd_mcimx6,
                                        UX_SLAVE_ENDPOINT *endpoint,
                                        ULONG direction);

#endif
