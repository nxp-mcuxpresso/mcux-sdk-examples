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
/**   IP3516 Controller                                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    ux_hcd_ip3516.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This file contains all the header and extern functions used by the  */
/*    USBX host IP3516 Controller.                                        */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef UX_HCD_IP3516_H
#define UX_HCD_IP3516_H


/* Possible defined IP3516 HCD extentions.  */

/* Extension for peripheral host mode select (function like).  */
/* #define UX_HCD_IP3516_EXT_USB_HOST_MODE_ENABLE(hcd_ip3516) */

/* Extension for embedded TT (UX_TRUE/UX_FALSE).  */
/* #define UX_HCD_IP3516_EXT_EMBEDDED_TT_SUPPORT */

/* Extension for phy high speed mode select (function like).  */
/* #define UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET(hcd_ip3516, on_off) */

/* IP3516 HCD extention for host mode select.  */
#ifndef UX_HCD_IP3516_EXT_USB_HOST_MODE_ENABLE
#define UX_HCD_IP3516_EXT_USB_HOST_MODE_ENABLE(hcd_ip3516)
#endif /* ifndef UX_HCD_IP3516_EXT_USB_HOST_MODE_ENABLE */

#ifndef UX_HCD_IP3516_EXT_EMBEDDED_TT_SUPPORT
#define UX_HCD_IP3516_EXT_EMBEDDED_TT_SUPPORT UX_TRUE
#endif /* ifndef UX_HCD_IP3516_EXT_EMBEDDED_TT_SUPPORT */

/* IP3516 HCD extention for host mode select.  */
#ifndef UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET

#define UX_IP3516_USBPHY_CTRL_LPC55S69             0x40038000
#define UX_IP3516_USBPHY_CTRL_SET_BIT1        ((*(volatile ULONG *)(UX_IP3516_USBPHY_CTRL_LPC55S69 + 0x34)) = 0x02)
#define UX_IP3516_USBPHY_CTRL_CLEAR_BIT1      ((*(volatile ULONG *)(UX_IP3516_USBPHY_CTRL_LPC55S69 + 0x38)) = 0x02)

#define UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET(hcd_ip3516, on_off) do          \
{                                                                               \
    if (on_off)                                                                 \
        UX_IP3516_USBPHY_CTRL_SET_BIT1;                                           \
    else                                                                        \
        UX_IP3516_USBPHY_CTRL_CLEAR_BIT1;                                         \
} while(0)


#define UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET(hcd_ip3516, on_off) do          \
{                                                                               \
    if (on_off)                                                                 \
        UX_IP3516_USBPHY_CTRL_SET_BIT1;                                           \
    else                                                                        \
        UX_IP3516_USBPHY_CTRL_CLEAR_BIT1;                                         \
} while(0)

#endif /* ifndef UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET */

/* Define IP3516 generic definitions.  */

#define UX_IP3516_CONTROLLER                                  2
#define UX_IP3516_MAX_PAYLOAD                                 16384
#define UX_IP3516_FRAME_DELAY                                 4 
#define UX_IP3516_PAGE_SIZE                                   4096
#define UX_IP3516_PAGE_ALIGN                                  0xfffff000

#ifndef UX_IP3516_BULK_ENDPOINT_BUFFER_SIZE
#define UX_IP3516_BULK_ENDPOINT_BUFFER_SIZE                   ((endpoint -> ux_endpoint_descriptor.wMaxPacketSize) * 4)
#endif

#ifndef UX_IP3516_BULK_ENDPOINT_BUFFER_SIZE_FS
#define UX_IP3516_BULK_ENDPOINT_BUFFER_SIZE_FS                ((endpoint -> ux_endpoint_descriptor.wMaxPacketSize) * 8)
#endif

#ifndef UX_IP3516_CONTROL_ENDPOINT_BUFFER_SIZE
#define UX_IP3516_CONTROL_ENDPOINT_BUFFER_SIZE                (256)
#endif

#ifndef UX_IP3516_PERIODIC_ENDPOINT_BUFFER_SIZE
#define UX_IP3516_PERIODIC_ENDPOINT_BUFFER_SIZE               (endpoint -> ux_endpoint_descriptor.wMaxPacketSize & UX_MAX_PACKET_SIZE_MASK)
#endif

/* Define IP3516 host controller capability registers.  */

#define IP3516_HCCR_CAP_LENGTH                                0x00
#define IP3516_HCCR_HCS_PARAMS                                0x01
#define IP3516_HCCR_HCC_PARAMS                                0x02
#define IP3516_HCCR_HCSP_PORT_ROUTE                           0x03


/* Define IP3516 host controller registers.  */

#define IP3516_HCOR_ATLPTD                                    (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x00)
#define IP3516_HCOR_ISOPTD                                    (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x01)
#define IP3516_HCOR_INTPTD                                    (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x02)
#define IP3516_HCOR_DATAPAYLOAD                               (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x03)

#define IP3516_HCOR_FRAME_INDEX                               (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x03)
#define IP3516_HCOR_PORT_SC                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x07)

#define IP3516_HCOR_USB_COMMAND                               (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x04)
#define IP3516_HCOR_USB_STATUS                                (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x05)
#define IP3516_HCOR_USB_INTERRUPT                             (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x06)
#define IP3516_HCOR_PORTSC1                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x07)
#define IP3516_HCOR_ATLPTDD                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x08)
#define IP3516_HCOR_ATLPTDS                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x09)
#define IP3516_HCOR_ISOPTDD                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x0A)
#define IP3516_HCOR_ISOPTDS                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x0B)
#define IP3516_HCOR_INTPTDD                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x0C)
#define IP3516_HCOR_INTPTDS                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x0D)
#define IP3516_HCOR_LASTPTD                                   (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x0E)
#define IP3516_HCOR_PORTMODE                                  (hcd_ip3516 -> ux_hcd_ip3516_hcor + 0x10)

/* Define IP3516 IO control register values.  */

#define IP3516_HC_IO_RS                                       0x00000001u

#define IP3516_HC_IO_ATL_EN                                   0x00000100u
#define IP3516_HC_IO_ISO_EN                                   0x00000200u
#define IP3516_HC_IO_INT_EN                                   0x00000400u


#define IP3516_HC_IO_HCRESET                                  0x00000002u
#define IP3516_HC_IO_PSE                                      0x00000010u
#define IP3516_HC_IO_ASE                                      0x00000020u
#define IP3516_HC_IO_IAAD                                     0x00000040u
#define IP3516_HC_IO_ITC                                      0x00010000u
#define IP3516_HC_IO_FRAME_SIZE_1024                          0x00000000u
#define IP3516_HC_IO_FRAME_SIZE_512                           0x00000004u
#define IP3516_HC_IO_FRAME_SIZE_256                           0x00000008u
#define IP3516_HC_IO_FRAME_SIZE_128                           0x0000000Cu
#define IP3516_HC_IO_FRAME_SIZE_64                            0x00008000u
#define IP3516_HC_IO_FRAME_SIZE_32                            0x00008004u

/* The number if entries in the periodic tree can be changed to save space IF and only IF the PFLF flag in the HCCPARAMS register
   allows it. Setting values less than 1024 in controllers without the ability to change the Frame List Size leads to a IP3516 crash.  */
   
#ifndef UX_IP3516_FRAME_LIST_ENTRIES
#define UX_IP3516_FRAME_LIST_ENTRIES                          1024
#endif
#define UX_IP3516_FRAME_LIST_MASK                             IP3516_HC_IO_FRAME_SIZE_1024

/* Define IP3516 HCOR status register.  */

#define IP3516_HC_STS_USB_INT                                 0x00000001u
#define IP3516_HC_STS_USB_ERR_INT                             0x00000002u
#define IP3516_HC_STS_PCD                                     0x00000004u
#define IP3516_HC_STS_FLR                                     0x00000008u
#define IP3516_HC_STS_HSE                                     0x00000010u
#define IP3516_HC_STS_IAA                                     0x00000020u
#define IP3516_HC_STS_HC_HALTED                               0x00001000u
#define IP3516_HC_STS_RECLAMATION                             0x00002000u
#define IP3516_HC_STS_PSS                                     0x00004000u
#define IP3516_HC_STS_ASS                                     0x00008000u

#define IP3516_HC_STS_ATL_IRQ                                 0x00010000u
#define IP3516_HC_STS_ISO_IRQ                                 0x00020000u
#define IP3516_HC_STS_INT_IRQ                                 0x00040000u
#define IP3516_HC_STS_SOF_IRQ                                 0x00080000u

#define IP3516_HC_INTERRUPT_ENABLE_NORMAL                     (IP3516_HC_STS_PCD | IP3516_HC_STS_ATL_IRQ | IP3516_HC_STS_INT_IRQ)


/* Define IP3516 HCOR root HUB command/status.  */

#define IP3516_HC_RH_PPC                                      0x00000010u
#define IP3516_HC_RH_PSM                                      0x00000100u
#define IP3516_HC_RH_NPS                                      0x00000200u
#define IP3516_HC_RH_DT                                       0x00000400u
#define IP3516_HC_RH_OCPM                                     0x00000800u
#define IP3516_HC_RH_NOCP                                     0x00001000u

#define IP3516_HC_PS_CCS                                      0x00000001u
#define IP3516_HC_PS_CSC                                      0x00000002u
#define IP3516_HC_PS_PE                                       0x00000004u
#define IP3516_HC_PS_PEC                                      0x00000008u
#define IP3516_HC_PS_OCA                                      0x00000010u
#define IP3516_HC_PS_OCC                                      0x00000020u
#define IP3516_HC_PS_FPR                                      0x00000040u
#define IP3516_HC_PS_SUSPEND                                  0x00000080u
#define IP3516_HC_PS_PR                                       0x00000100u
#define IP3516_HC_PS_PP                                       0x00001000u
#define IP3516_HC_PS_SPEED_MASK                               0x00000c00u
#define IP3516_HC_PS_SPEED_LOW                                0x00000400u
#define IP3516_HC_PS_PO                                       0x00002000u
#define IP3516_HC_PS_EMBEDDED_TT_SPEED_MASK                   0x00300000u
#define IP3516_HC_PS_EMBEDDED_TT_SPEED_FULL                   0x00100000u
#define IP3516_HC_PS_EMBEDDED_TT_SPEED_LOW                    0x00000000u
#define IP3516_HC_PS_EMBEDDED_TT_SPEED_HIGH                   0x00200000u

#define IP3516_HC_RH_POWER_STABLE_DELAY                       25
#define IP3516_HC_RH_RESET_DELAY                              50
#define IP3516_HC_RH_RESET_SETTLE_DELAY                       5


/* Define IP3516 interrupt status register definitions.  */

#define IP3516_HC_INT_IE                                      0x00000001u
#define IP3516_HC_INT_EIE                                     0x00000002u
#define IP3516_HC_INT_PCIE                                    0x00000004u
#define IP3516_HC_INT_FLRE                                    0x00000008u
#define IP3516_HC_INT_HSER                                    0x00000010u
#define IP3516_HC_INT_IAAE                                    0x00000020u


/* Define IP3516 frame interval definition.  */

#define IP3516_HC_FM_INTERVAL_CLEAR                           0x8000ffff
#define IP3516_HC_FM_INTERVAL_SET                             0x27780000


/* Define IP3516 static definition.  */

#define UX_IP3516_AVAILABLE_BANDWIDTH                         6000
#define UX_IP3516_STOP                                        0
#define UX_IP3516_START                                       1
#define UX_IP3516_ROUTE_TO_LOCAL_HC                           1
#define UX_IP3516_INIT_DELAY                                  1000
#define UX_IP3516_RESET_RETRY                                 1000
#define UX_IP3516_RESET_DELAY                                 100
#define UX_IP3516_PORT_RESET_RETRY                            10
#define UX_IP3516_PORT_RESET_DELAY                            50


/* Define IP3516 initialization values.  */

#define UX_IP3516_COMMAND_STATUS_RESET                        0
#define UX_IP3516_INIT_RESET_DELAY                            10


/* Define IP3516 completion code errors.  */

#define UX_IP3516_NO_ERROR                                    0x00
#define UX_IP3516_ERROR_CRC                                   0x01
#define UX_IP3516_ERROR_BIT_STUFFING                          0x02
#define UX_IP3516_ERROR_DATA_TOGGLE                           0x03
#define UX_IP3516_ERROR_STALL                                 0x04
#define UX_IP3516_ERROR_DEVICE_NOT_RESPONDING                 0x05
#define UX_IP3516_ERROR_PID_FAILURE                           0x06
#define UX_IP3516_ERROR_DATA_OVERRUN                          0x08
#define UX_IP3516_ERROR_DATA_UNDERRUN                         0x09
#define UX_IP3516_ERROR_BUFFER_OVERRUN                        0x0c
#define UX_IP3516_ERROR_BUFFER_UNDERRUN                       0x0d
#define UX_IP3516_ERROR_NOT_ACCESSED                          0x0f
#define UX_IP3516_ERROR_NAK                                   0x10
#define UX_IP3516_ERROR_BABBLE                                0x11


/* Define IP3516 pointers.  */

typedef union UX_IP3516_POINTER_UNION {
    ULONG                               value;
    VOID                                *void_ptr;
    UCHAR                               *u8_ptr;
    USHORT                              *u16_ptr;
    ULONG                               *u32_ptr;
} UX_IP3516_POINTER;

typedef union UX_IP3516_LINK_POINTER_UNION {
    ULONG                               value;
    VOID                                *void_ptr;
    UCHAR                               *u8_ptr;
    USHORT                              *u16_ptr;
    ULONG                               *u32_ptr;
    struct UX_IP3516_ED_STRUCT            *qh_ptr;
    struct UX_IP3516_ED_STRUCT            *ed_ptr;
    struct UX_IP3516_TD_STRUCT            *td_ptr;
    struct UX_IP3516_HSISO_TD_STRUCT      *itd_ptr;
    struct UX_IP3516_FSISO_TD_STRUCT      *sitd_ptr;
} UX_IP3516_LINK_POINTER;

typedef union UX_IP3516_PERIODIC_LINK_POINTER_UNION {
    ULONG                               value;
    VOID                                *void_ptr;
    UCHAR                               *u8_ptr;
    USHORT                              *u16_ptr;
    ULONG                               *u32_ptr;
    struct UX_IP3516_ED_STRUCT            *qh_ptr;
    struct UX_IP3516_ED_STRUCT            *ed_ptr;
    struct UX_IP3516_HSISO_TD_STRUCT      *itd_ptr;
    struct UX_IP3516_FSISO_TD_STRUCT      *sitd_ptr;
} UX_IP3516_PERIODIC_LINK_POINTER;

typedef struct _usb_host_ip3516hs_atl_struct
{
    union
    {
        ULONG controlState;
        struct
        {
            volatile ULONG V : 1U;              /*!< Valid */
            volatile ULONG NextPTDPointer : 5U; /*!< NextPTDPointer */
            ULONG R1 : 1U;                      /*!< Reserved */
            volatile ULONG J : 1U; /*!< Jump:  0: increment the PTD pointer. 1: enable the next PTD branching. */
            volatile ULONG
                uFrame : 8U; /*!< This field is only applicable for interrupt and isochronous endpoints. */
            volatile ULONG MaxPacketLength : 11U; /*!< Maximum Packet Length */
            ULONG R2 : 1U;                        /*!< Reserved */
            volatile ULONG Mult : 2U;             /*!< EndpointNumber */
            ULONG R3 : 2U;                        /*!< Reserved */
        } stateBitField;
    } control1Union;
    union
    {
        ULONG controlState;
        struct
        {
            volatile ULONG EP : 4U;            /*!< Endpoint number */
            volatile ULONG DeviceAddress : 7U; /*!< Device address */
            volatile ULONG S : 1U;  /*!< This bit indicates whether a split transaction has to be executed. */
            volatile ULONG RL : 4U; /*!< Reload: If RL is set to 0h, hardware ignores the NakCnt value. RL and
                                       NakCnt are set to the same value before a transaction. */
            volatile ULONG SE : 2U; /*!< This specifies the speed for a Control or Interrupt transaction to a device
                                       that is not high-speed: 00-Full-speed, 10-Low-speed */
            volatile ULONG PortNumber : 7U; /*!< Port number */
            volatile ULONG HubAddress : 7U; /*!< Hub Address */
        } stateBitField;
    } control2Union;
    union
    {
        ULONG data;
        struct
        {
            volatile ULONG NrBytesToTransfer : 15U; /*!< Number of Bytes to Transfer. */
            volatile ULONG I : 1U;                  /*!< Interrupt on Complete */
            volatile ULONG DataStartAddress : 16U;  /*!< Data buffer address */
        } dataBitField;
    } dataUnion;
    union
    {
        ULONG state;
        struct
        {
            volatile ULONG NrBytesToTransfer : 15U; /*!< Number of Bytes Transferred. */
            volatile ULONG
                Token : 2U; /*!< Token: Identifies the token Packet Identifier (PID) for this transaction. */
            volatile ULONG EpType : 2U; /*!< Endpoint type */
            volatile ULONG NakCnt : 4U; /*!< Nak count */
            volatile ULONG Cerr : 2U;   /*!< Error count */
            volatile ULONG DT : 1U;     /*!< Data Toggle */
            volatile ULONG P : 1U;      /*!< Ping */
            volatile ULONG SC : 1U;     /*!< Start/Complete */
            volatile ULONG X : 1U;      /*!< Error */
            volatile ULONG B : 1U;      /*!< Babble */
            volatile ULONG H : 1U;      /*!< Halt */
            volatile ULONG A : 1U;      /*!< Active */
        } stateBitField;
    } stateUnion;
} usb_host_ip3516hs_atl_struct_t;


typedef struct _usb_host_ip3516hs_ptl_struct
{
    union
    {
        ULONG controlState;
        struct
        {
            volatile ULONG V : 1U; /*!< valid */
            volatile ULONG
                NextPTDPointer : 5U;  /*!< Next PTD Counter: Next PTD branching assigned by the PTDpointer. */
            ULONG R1 : 1U;         /*!< Reserved */
            volatile ULONG J : 1U; /*!< Jump:  0: increment the PTD pointer. 1: enable the next PTD branching. */
            volatile ULONG
                uFrame : 8U; /*!< This field is only applicable for interrupt and isochronous endpoints. */
            volatile ULONG MaxPacketLength : 11U; /*!< Maximum Packet Length */
            ULONG R2 : 1U;                        /*!< Reserved */
            volatile ULONG Mult : 2U;             /*!< Multiplier */
            ULONG R3 : 2U;                        /*!< Reserved */
        } stateBitField;
    } control1Union;
    union
    {
        ULONG controlState;
        struct
        {
            volatile ULONG EP : 4U;            /*!< Endpoint number */
            volatile ULONG DeviceAddress : 7U; /*!< Device address */
            volatile ULONG S : 1U;  /*!< This bit indicates whether a split transaction has to be executed. */
            volatile ULONG RL : 4U; /*!< Reload: If RL is set to 0h, hardware ignores the NakCnt value. RL and
                                       NakCnt are set to the same value before a transaction. */
            volatile ULONG SE : 2U; /*!< This specifies the speed for a Control or Interrupt transaction to a device
                                       that is not high-speed: 00-Full-speed, 10-Low-speed */
            ULONG R1 : 14U;         /*!< Reserved */
        } stateBitField;
    } control2Union;
    union
    {
        ULONG data;
        struct
        {
            volatile ULONG NrBytesToTransfer : 15U; /*!< Number of Bytes to Transfer. */
            volatile ULONG I : 1U;                  /*!< Interrupt on Complete. */
            volatile ULONG DataStartAddress : 16U;  /*!< Data buffer address */
        } dataBitField;
    } dataUnion;
    union
    {
        ULONG state;
        struct
        {
            volatile ULONG NrBytesToTransfer : 15U; /*!< Number of Bytes Transferred. */
            volatile ULONG
                Token : 2U; /*!< Token: Identifies the token Packet Identifier (PID) for this transaction. */
            volatile ULONG EpType : 2U; /*!< Endpoint type */
            volatile ULONG NakCnt : 4U; /*!< Nak count */
            volatile ULONG Cerr : 2U;   /*!< Error count */
            volatile ULONG DT : 1U;     /*!< Data Toggle */
            volatile ULONG P : 1U;      /*!< Ping */
            volatile ULONG SC : 1U;     /*!< Start/Complete */
            volatile ULONG X : 1U;      /*!< Error */
            volatile ULONG B : 1U;      /*!< Babble */
            volatile ULONG H : 1U;      /*!< Halt */
            volatile ULONG A : 1U;      /*!< Active */
        } stateBitField;
    } stateUnion;
    union
    {
        ULONG status;
        struct
        {
            volatile ULONG uSA : 8U; /*!< This field is only used for periodic split transactions or if the port is
                                           enabled in HS mode. */
            volatile ULONG Status0 : 3U; /*!< Isochronous IN or OUT status at uSOF0 */
            volatile ULONG Status1 : 3U; /*!< Isochronous IN or OUT status at uSOF1 */
            volatile ULONG Status2 : 3U; /*!< Isochronous IN or OUT status at uSOF2 */
            volatile ULONG Status3 : 3U; /*!< Isochronous IN or OUT status at uSOF3 */
            volatile ULONG Status4 : 3U; /*!< Isochronous IN or OUT status at uSOF4 */
            volatile ULONG Status5 : 3U; /*!< Isochronous IN or OUT status at uSOF5 */
            volatile ULONG Status6 : 3U; /*!< Isochronous IN or OUT status at uSOF6 */
            volatile ULONG Status7 : 3U; /*!< Isochronous IN or OUT status at uSOF7 */
        } statusBitField;
    } statusUnion;
    union
    {
        ULONG isoIn;
        struct
        {
            volatile ULONG isoIn0 : 12U;   /*!< Data length */
            volatile ULONG isoIn1 : 12U;   /*!< Data length */
            volatile ULONG isoIn2low : 8U; /*!< Data length */
        } bitField;
    } isoInUnion1;
    union
    {
        ULONG isoIn;
        struct
        {
            volatile ULONG isoIn2High : 4U; /*!< Data length */
            volatile ULONG isoIn3 : 12U;    /*!< Data length */
            volatile ULONG isoIn4 : 12U;    /*!< Data length */
            volatile ULONG isoIn5Low : 4U;  /*!< Data length */
        } bitField;
    } isoInUnion2;
    union
    {
        ULONG isoIn;
        struct
        {
            volatile ULONG isoIn5High : 8U; /*!< Data length */
            volatile ULONG isoIn6 : 12U;    /*!< Data length */
            volatile ULONG isoIn7 : 12U;    /*!< Data length */
        } bitField;
    } isoInUnion3;
} usb_host_ip3516hs_ptl_struct_t;


typedef struct _usb_host_ip3516hs_sptl_struct
{
    union
    {
        ULONG controlState;
        struct
        {
            volatile ULONG V : 1U;                /*!< Valid */
            volatile ULONG NextPTDPointer : 5U;   /*!< NextPTDPointer */
            ULONG R1 : 1U;                        /*!< Reserved */
            volatile ULONG J : 1U;                /*!< Jump */
            volatile ULONG uFrame : 8U;           /*!< Frame number at which this PTD will be sent. */
            volatile ULONG MaxPacketLength : 11U; /*!< Maximum Packet Length. */
            ULONG R2 : 1U;                        /*!< Reserved */
            volatile ULONG Mult : 2U;             /*!< Multiplier */
            ULONG R3 : 2U;                        /*!< Reserved */
        } stateBitField;
    } control1Union;
    union
    {
        ULONG controlState;
        struct
        {
            volatile ULONG EP : 4U;            /*!< Endpoint number */
            volatile ULONG DeviceAddress : 7U; /*!< Device address */
            volatile ULONG S : 1U;  /*!< This bit indicates whether a split transaction has to be executed. */
            volatile ULONG RL : 4U; /*!< Reload: If RL is set to 0h, hardware ignores the NakCnt value. RL and
                                       NakCnt are set to the same value before a transaction. */
            volatile ULONG SE : 2U; /*!< This specifies the speed for a Control or Interrupt transaction to a device
                                       that is not high-speed: 00-Full-speed, 10-Low-speed */
            volatile ULONG PortNumber : 7U; /*!< Port number */
            volatile ULONG HubAddress : 7U; /*!< Hub Address */
        } stateBitField;
    } control2Union;
    union
    {
        ULONG data;
        struct
        {
            volatile ULONG NrBytesToTransfer : 15U; /*!< Number of Bytes to Transfer. */
            volatile ULONG I : 1U;                  /*!< Interrupt on Complete. */
            volatile ULONG DataStartAddress : 16U;  /*!< Data buffer address */
        } dataBitField;
    } dataUnion;
    union
    {
        ULONG state;
        struct
        {
            volatile ULONG NrBytesToTransfer : 15U; /*!< Number of Bytes Transferred. */
            volatile ULONG
                Token : 2U; /*!< Token: Identifies the token Packet Identifier (PID) for this transaction. */
            volatile ULONG EpType : 2U; /*!< Endpoint type */
            volatile ULONG NakCnt : 4U; /*!< Nak count */
            volatile ULONG Cerr : 2U;   /*!< Error count */
            volatile ULONG DT : 1U;     /*!< Data Toggle */
            volatile ULONG P : 1U;      /*!< Ping */
            volatile ULONG SC : 1U;     /*!< Start/Complete */
            volatile ULONG X : 1U;      /*!< Error */
            volatile ULONG B : 1U;      /*!< Babble */
            volatile ULONG H : 1U;      /*!< Halt */
            volatile ULONG A : 1U;      /*!< Active */
        } dataBitField;
    } stateUnion;
    union
    {
        ULONG status;
        struct
        {
            volatile ULONG uSA : 8U; /*!< This field is only used for periodic split transactions or if the port is
                                           enabled in HS mode. */
            volatile ULONG Status0 : 3U; /*!< Isochronous IN or OUT status at uSOF0 */
            volatile ULONG Status1 : 3U; /*!< Isochronous IN or OUT status at uSOF1 */
            volatile ULONG Status2 : 3U; /*!< Isochronous IN or OUT status at uSOF2 */
            volatile ULONG Status3 : 3U; /*!< Isochronous IN or OUT status at uSOF3 */
            volatile ULONG Status4 : 3U; /*!< Isochronous IN or OUT status at uSOF4 */
            volatile ULONG Status5 : 3U; /*!< Isochronous IN or OUT status at uSOF5 */
            volatile ULONG Status6 : 3U; /*!< Isochronous IN or OUT status at uSOF6 */
            volatile ULONG Status7 : 3U; /*!< Isochronous IN or OUT status at uSOF7 */
        } statusBitField;
    } statusUnion;
    union
    {
        ULONG isoIn;
        struct
        {
            volatile ULONG uSCS : 8U; /*!< All bits can be set to one for every transfer. It specifies which uSOF the
                                              complete split needs to be sent. */
            volatile ULONG isoIn0 : 8U; /*!< Data length */
            volatile ULONG isoIn1 : 8U; /*!< Data length */
            volatile ULONG isoIn2 : 8U; /*!< Data length */
        } bitField;
    } isoInUnion1;
    union
    {
        ULONG isoIn;
        struct
        {
            volatile ULONG isoIn3 : 8U; /*!< Data length */
            volatile ULONG isoIn4 : 8U; /*!< Data length */
            volatile ULONG isoIn5 : 8U; /*!< Data length */
            volatile ULONG isoIn6 : 8U; /*!< Data length */
        } bitField;
    } isoInUnion2;
    union
    {
        ULONG isoIn;
        struct
        {
            volatile ULONG isoIn7 : 8U; /*!< Data length */
            volatile ULONG R : 24U;     /*!< Reserved */
        } bitField;
    } isoInUnion3;
} usb_host_ip3516hs_sptl_struct_t;


#define USB_HOST_IP3516HS_PTD_TOKEN_OUT     0x00U
#define USB_HOST_IP3516HS_PTD_TOKEN_IN      0x01U
#define USB_HOST_IP3516HS_PTD_TOKEN_SETUP   0x02U

#define UX_IP3516_PIPE_STATE_FREE             (0)
#define UX_IP3516_PIPE_STATE_IDLE             (1)
#define UX_IP3516_PIPE_STATE_SETUP            (2)
#define UX_IP3516_PIPE_STATE_SETUP_DATA       (3)
#define UX_IP3516_PIPE_STATE_SETUP_STATUS     (4)
#define UX_IP3516_PIPE_STATE_BULK_DATA        (5)

typedef struct UX_IP3516_PIPE_STRUCT
{
    UCHAR           ux_ip3516_pipe_state;
    UCHAR           ux_ip3516_pipe_index;
    struct UX_ENDPOINT_STRUCT           *ux_ip3516_ed_endpoint;
    struct UX_TRANSFER_STRUCT           *ux_ip3516_ed_transfer_request;
    UCHAR           *ux_ip3516_ed_buffer_address;
    USHORT          ux_ip3516_ed_buffer_len;
}UX_IP3516_PIPE;

typedef struct UX_IP3516_PERIODIC_PIPE_STRUCT
{
    UCHAR                               ux_ip3516_pipe_state;
    UCHAR                               ux_ip3516_pipe_index;

    UCHAR                               ux_ip3516_ed_interval_mask;
    UCHAR                               ux_ip3516_ed_interval_position;

    UCHAR                               ux_ip3516_ed_uSA;

    struct UX_ENDPOINT_STRUCT           *ux_ip3516_ed_endpoint;
    struct UX_TRANSFER_STRUCT           *ux_ip3516_ed_transfer_request;

    UCHAR           *ux_ip3516_ed_buffer_address;
    USHORT          ux_ip3516_ed_buffer_len;

}UX_IP3516_PERIODIC_PIPE;

/* Define the IP3516 structure.  */

typedef struct UX_HCD_IP3516_STRUCT
{                                      

    struct UX_HCD_STRUCT
                    *ux_hcd_ip3516_hcd_owner;
    ULONG           ux_hcd_ip3516_hcor;
    ULONG           *ux_hcd_ip3516_base;
    UINT            ux_hcd_ip3516_nb_root_hubs;
    struct UX_IP3516_TD_STRUCT              
                    *ux_hcd_ip3516_done_head;
    struct UX_IP3516_ED_STRUCT              
                    *ux_hcd_ip3516_asynch_head_list;
    struct UX_IP3516_ED_STRUCT              
                    *ux_hcd_ip3516_asynch_first_list;
    struct UX_IP3516_ED_STRUCT              
                    *ux_hcd_ip3516_asynch_last_list;
    struct UX_IP3516_HSISO_TD_STRUCT
                    *ux_hcd_ip3516_hsiso_scan_list;
    struct UX_IP3516_FSISO_TD_STRUCT
                    *ux_hcd_ip3516_fsiso_scan_list;
    struct UX_TRANSFER_STRUCT
                    *ux_hcd_ip3516_iso_done_transfer_head;
    struct UX_TRANSFER_STRUCT
                    *ux_hcd_ip3516_iso_done_transfer_tail;
    struct UX_IP3516_ED_STRUCT
                    *ux_hcd_ip3516_interrupt_ed_list;
    UX_MUTEX        ux_hcd_ip3516_periodic_mutex;
    UX_SEMAPHORE    ux_hcd_ip3516_protect_semaphore;
    ULONG           ux_hcd_ip3516_frame_list_size;
    ULONG           ux_hcd_ip3516_interrupt_count;
    ULONG           ux_hcd_ip3516_embedded_tt;

    usb_host_ip3516hs_atl_struct_t * ux_hcd_ip3516_atl_array;
    usb_host_ip3516hs_ptl_struct_t * ux_hcd_ip3516_int_ptl_array;
    UX_IP3516_PIPE    ux_hcd_ip3516_atl_pipes[32];
    UX_IP3516_PERIODIC_PIPE
                    ux_hcd_ip3516_int_ptl_pipes[32];

    TX_BYTE_POOL    ux_hcd_ip3516_usb_memory_pool;
} UX_HCD_IP3516;




/* Define IP3516 function prototypes.  */

UINT    _ux_hcd_ip3516_asynchronous_endpoint_create(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
UINT    _ux_hcd_ip3516_asynchronous_endpoint_destroy(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
UINT    _ux_hcd_ip3516_controller_disable(UX_HCD_IP3516 *hcd_ip3516);
VOID    _ux_hcd_ip3516_done_queue_process(UX_HCD_IP3516 *hcd_ip3516);
UINT    _ux_hcd_ip3516_endpoint_reset(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
UINT    _ux_hcd_ip3516_entry(UX_HCD *hcd, UINT function, VOID *parameter);
UINT    _ux_hcd_ip3516_frame_number_get(UX_HCD_IP3516 *hcd_ip3516, ULONG *frame_number);
VOID    _ux_hcd_ip3516_frame_number_set(UX_HCD_IP3516 *hcd_ip3516, ULONG frame_number);
UINT    _ux_hcd_ip3516_initialize(UX_HCD *hcd);
UINT    _ux_hcd_ip3516_interrupt_endpoint_create(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
UINT    _ux_hcd_ip3516_interrupt_endpoint_destroy(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
VOID    _ux_hcd_ip3516_interrupt_handler(VOID);
UINT    _ux_hcd_ip3516_isochronous_endpoint_create(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
UINT    _ux_hcd_ip3516_isochronous_endpoint_destroy(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint);
UINT    _ux_hcd_ip3516_least_traffic_list_get(UX_HCD_IP3516 *hcd_ip3516, ULONG interval_mask);
UINT    _ux_hcd_ip3516_port_disable(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index);
UINT    _ux_hcd_ip3516_port_reset(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index);
UINT    _ux_hcd_ip3516_port_resume(UX_HCD_IP3516 *hcd_ip3516, UINT port_index);
ULONG   _ux_hcd_ip3516_port_status_get(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index);
UINT    _ux_hcd_ip3516_port_suspend(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index);
UINT    _ux_hcd_ip3516_power_down_port(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index);
UINT    _ux_hcd_ip3516_power_on_port(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index);
VOID    _ux_hcd_ip3516_power_root_hubs(UX_HCD_IP3516 *hcd_ip3516);
ULONG   _ux_hcd_ip3516_register_read(UX_HCD_IP3516 *hcd_ip3516, ULONG ip3516_register);
VOID    _ux_hcd_ip3516_register_write(UX_HCD_IP3516 *hcd_ip3516, ULONG ip3516_register, ULONG value);
UINT    _ux_hcd_ip3516_request_bulk_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request);
UINT    _ux_hcd_ip3516_request_control_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request);
UINT    _ux_hcd_ip3516_request_interrupt_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request);
UINT    _ux_hcd_ip3516_request_isochronous_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request);
UINT    _ux_hcd_ip3516_request_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request);
UINT    _ux_hcd_ip3516_transfer_abort(UX_HCD_IP3516 *hcd_ip3516,UX_TRANSFER *transfer_request);
VOID    _ux_hcd_ip3516_transfer_request_process(UX_TRANSFER *transfer_request);

#define ux_hcd_ip3516_initialize                      _ux_hcd_ip3516_initialize
#define ux_hcd_ip3516_interrupt_handler               _ux_hcd_ip3516_interrupt_handler

#endif

