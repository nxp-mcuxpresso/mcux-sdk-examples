/*
** ###################################################################
**     Processors:          KW45B41Z83AFPA
**                          KW45B41Z83AFTA
**
**     Compiler:            GNU C Compiler
**     Reference manual:    KW45B41, Rev. 0 Draft F, July 2020
**     Version:             rev. 1.0, 2020-05-12
**     Build:               b201013
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2021 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/

GROUP (
  libm.a
)

/* Entry Point */
ENTRY(Reset_Handler)

m_flash_start                       = 0x00000000;
m_flash_end                         = 0x000FFFFF;
m_flash_size                        = 0x00100000;
m_sector_size                       = 0x2000;
m_vector_table_size                 = 0x400;

_flash_start = m_flash_start;

m_interrupts_start                  = m_flash_start;
m_interrupts_end                    = m_interrupts_start + m_vector_table_size - 1;
__VECTOR_TABLE                      = m_interrupts_start;

m_flash_config_start                = 0x00000400;
m_flash_config_size                 = 0x00000010;
m_flash_config_end                  = m_flash_config_start + m_flash_config_size - 1;

/* This section is not enabled by default but you need it if gHwParamsProdDataPlacement_c is defined to
 * gHwParamsProdDataMainFlashMode_c or gHwParamsProdDataMainFlash2IfrMode_c in your project */
/*
 * m_fsl_prodInfo_size                 = DEFINED(gUseProdInfoMainFlashMode_d) ? m_sector_size : 0;
 */
m_fsl_prodInfo_size                 = m_sector_size;
m_fsl_prodInfo_end                  = m_flash_end;
m_fsl_prodInfo_start                = m_fsl_prodInfo_end - m_fsl_prodInfo_size + 1;

PROD_DATA_BASE_ADDR                 = m_fsl_prodInfo_start;

m_ot_settings_page_size             = m_sector_size;
m_ot_settings_page_count            = DEFINED(gUseOtSettingsStorage_d) ? 2 : 0;
m_ot_settings_size                  = DEFINED(gUseOtSettingsStorage_d) ? m_ot_settings_page_size * m_ot_settings_page_count : 0;
m_ot_settings_start                 = m_fsl_prodInfo_start - m_ot_settings_size;
m_ot_settings_end                   = m_fsl_prodInfo_start - 1;

NV_STORAGE_MAX_SECTORS              = DEFINED(gUseNVMLink_d) ? (DEFINED(gNvmSectors) ? gNvmSectors : 4) : 0;
NV_STORAGE_SECTOR_SIZE              = m_sector_size;
NV_STORAGE_END_ADDRESS              = m_ot_settings_start - 1;
NV_STORAGE_SIZE                     = (NV_STORAGE_MAX_SECTORS * NV_STORAGE_SECTOR_SIZE);
NV_STORAGE_START_ADDRESS            = NV_STORAGE_END_ADDRESS - NV_STORAGE_SIZE + 1;
NV_VIRTUAL_PAGE_BOUND               = NV_STORAGE_END_ADDRESS - (NV_STORAGE_MAX_SECTORS / 2 * NV_STORAGE_SECTOR_SIZE);

INT_STORAGE_END                     = NV_STORAGE_START_ADDRESS - 1;
INT_STORAGE_SIZE                    = DEFINED(gUseInternalStorageLink_d) ? m_sector_size * ( ( (INT_STORAGE_END + 1 - m_interrupts_start) / 2 ) / m_sector_size + 1) : 0;
INT_STORAGE_START                   = INT_STORAGE_END - INT_STORAGE_SIZE + 1;
INT_STORAGE_SECTOR_SIZE             = m_sector_size;

__MATTER_FACTORY_DATA_END           = INT_STORAGE_START - 1;
__MATTER_FACTORY_DATA_SIZE          = DEFINED(gUseFactoryData_d) ? m_sector_size : 0;
__MATTER_FACTORY_DATA_START         = __MATTER_FACTORY_DATA_END - __MATTER_FACTORY_DATA_SIZE + 1;

m_text_start                        = m_flash_config_end + 1;
m_text_end                          = __MATTER_FACTORY_DATA_START - 1;
m_text_size                         = m_text_end - m_text_start;

m_ram0_start                        = 0x14000000;
m_ram0_end                          = 0x14003FFF;
m_ram1_start                        = 0x20000000;
m_ram1_end                          = 0x2001BFFF;
/* This region is used by bootrom when exiting power down modes */
m_ram1_reserved_start               = m_ram1_start;
m_ram1_reserved_size                = 0x4000;
m_ram1_reserved_end                 = m_ram1_start + m_ram1_reserved_size - 1;

m_warmboot_stack_size               = 0x400;
m_warmboot_stack_start              = DEFINED(gLowpowerPowerDownEnable_d) ? m_ram1_reserved_end + 1 : m_ram1_start;
m_warmboot_stack_end                = m_warmboot_stack_start + m_warmboot_stack_size - 1;

firmware_ram_lower_limit            = DEFINED(lp_ram_lower_limit) ? lp_ram_lower_limit : m_data1_start;
firmware_ram_upper_limit            = DEFINED(lp_ram_upper_limit) ? lp_ram_upper_limit : m_data1_end;

/* Used by the fwk_platform_lowpower module */
gLowpowerPowerDownEnable_d          = DEFINED(gLowpowerPowerDownEnable_d) ? 1 : 0;

m_ram_vector_table_size             = m_vector_table_size;
HEAP_SIZE                           = DEFINED(__heap_size__)  ? __heap_size__  : 0x0800;
STACK_SIZE                          = DEFINED(__stack_size__) ? __stack_size__ : 0x01000;

m_cstack_start                      = DEFINED(m_cstack_start) ? m_cstack_start : DEFINED(m_cstack_ram1) ? m_warmboot_stack_end + 1 : m_ram0_start;
m_cstack_end                        = m_cstack_start + STACK_SIZE - 1;

m_interrupts_ram_start              = DEFINED(m_cstack_ram1) ? m_ram0_start : m_cstack_end + 1;
m_interrupts_ram_end                = m_interrupts_ram_start + m_ram_vector_table_size - 1;
__VECTOR_RAM                        = m_interrupts_ram_start;
__RAM_VECTOR_TABLE_SIZE_BYTES       = m_ram_vector_table_size;

m_data0_start                       = m_interrupts_ram_end + 1;
m_data0_end                         = m_ram0_end;
m_data0_size                        = m_ram0_end - m_data0_start + 1;

m_data1_start                       = DEFINED(m_cstack_ram1) ? m_cstack_end + 1 : m_warmboot_stack_end + 1;
m_data1_end                         = m_ram1_end;
m_data1_size                        = m_data1_end - m_data1_start + 1;

__HEAP_start__                      = m_data1_end - HEAP_SIZE;
__HEAP_end__                        = m_data1_end;

m_sqram_debug_size                  = 0x7C;  /*New section allowing to share debug info to CM33 (LL error/warnings).*/
m_sqram_btblebuf_size               = 0x5400;

/* Section used to share a flag on both cores for lowpower purpose */
m_lowpower_flag_size              = 0x08;
m_lowpower_flag_start             = 0x489C0000 + m_sqram_debug_size;

rpmsg_sh_mem_size                   = 0x1800;
rpmsg_sh_mem_start                  = 0x489C9FFF - rpmsg_sh_mem_size + 1;
rpmsg_sh_mem_end                    = 0x489C9FFF;

/* Last reserved word of the debug section used for the sfc module */
m_sfc_log_start                     = 0x489C0078;

/* Specify the memory areas */
MEMORY
{
  m_interrupts          (RX)  : ORIGIN = m_interrupts_start, LENGTH = m_vector_table_size
  m_flash_config        (RX)  : ORIGIN = m_flash_config_start, LENGTH = m_flash_config_size
  m_text                (RX)  : ORIGIN = m_text_start, LENGTH = m_text_size
  m_factory_data        (RX)  : ORIGIN = __MATTER_FACTORY_DATA_START, LENGTH = __MATTER_FACTORY_DATA_SIZE
  m_internal_storage    (RX)  : ORIGIN = INT_STORAGE_START, LENGTH = INT_STORAGE_SIZE
  fNVM_region           (RX)  : ORIGIN = NV_STORAGE_START_ADDRESS, LENGTH = NV_STORAGE_SIZE / 2
  sNVM_region           (RX)  : ORIGIN = (NV_VIRTUAL_PAGE_BOUND + 1), LENGTH = NV_STORAGE_SIZE / 2
  m_ot_settings         (RX)  : ORIGIN = m_ot_settings_start, LENGTH = m_ot_settings_size
  m_fsl_prod_info       (RX)  : ORIGIN = m_fsl_prodInfo_start, LENGTH = m_fsl_prodInfo_size
  m_cstack              (RW)  : ORIGIN = m_cstack_start, LENGTH = STACK_SIZE
  m_ram_interrupts      (RW)  : ORIGIN = m_interrupts_ram_start, LENGTH = m_ram_vector_table_size
  m_data0               (RW)  : ORIGIN = m_data0_start, LENGTH = m_data0_size
  m_data1               (RW)  : ORIGIN = m_data1_start, LENGTH = m_data1_size
  rpmsg_sh_mem          (RW)  : ORIGIN = rpmsg_sh_mem_start, LENGTH = rpmsg_sh_mem_size
  m_lowpower_flag       (RW)  : ORIGIN = m_lowpower_flag_start, LENGTH = m_lowpower_flag_size
}

/* Define output sections */
SECTIONS
{
  /* NOINIT section for rpmsg_sh_mem */
  .noinit_rpmsg_sh_mem (NOLOAD) : ALIGN(4)
  {
     __RPMSG_SH_MEM_START__ = .;
     *(.noinit.$rpmsg_sh_mem*)
     . = ALIGN(4) ;
     __RPMSG_SH_MEM_END__ = .;
  } > rpmsg_sh_mem
  /* The startup code goes first into internal flash */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  .flash_config :
  {
    . = ALIGN(4);
    KEEP(*(.FlashConfig))    /* Flash Configuration Field (FCF) */
    . = ALIGN(4);
  } > m_flash_config

  /* ZB OTA related sections, should not consume any memory if .ro_nonce/.ro_ota_header/.ro_se_lnkkey not used */
  .ro_nonce : ALIGN(0x10)
  {
    _FlsNonceStart = ABSOLUTE(.);
    *(.ro_nonce) /* nonce value is 16 bytes.*/
    FILL(0xff)
    . = ALIGN (0x10);
  } > m_text

  .ro_ota_header : ALIGN(0x10)
  {
    _enc_start = ABSOLUTE(.);
    _enc_offset = (_enc_start & 0x0000000F);
    _FlsOtaHeader = ABSOLUTE(.);
    *(.ro_ota_header) /* Ota Header 69 bytes*/
    FILL(0xff)
    . = ALIGN (0x10);
  } > m_text
    
  .ro_se_lnkKey (ALIGN((. - _enc_offset), 16) + _enc_offset):
  {
    _FlsLinkKey = ABSOLUTE(.);
    *(.ro_se_lnkKey)  /* Link Key 16 bytes*/
    FILL(0xff)
    . = ALIGN (0x10);
  } > m_text

  /* The program code and other data goes into internal flash */
  .text :
  {
    . = ALIGN(4);
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.rodata)               /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  } > m_text

  .NVM_TABLE :
  {
    . = ALIGN(4);
    PROVIDE(__start_NVM_TABLE = .);
    KEEP(*(.NVM_TABLE));
    PROVIDE(__stop_NVM_TABLE = .);
    . = ALIGN(4);
  } > m_text

  .VERSION_TAGS :
  {
    . = ALIGN(4);
    PROVIDE(__start_VERSION_TAGS = .);
    KEEP(*(.VERSION_TAGS));
    PROVIDE(__stop_VERSION_TAGS = .);
    . = ALIGN(4);
  } > m_text

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > m_text

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_text

  ._settings_handler_static :
  {
    . = ALIGN(4);
    _settings_handler_static_list_start = .;
    KEEP(*(SORT(._settings_handler_static.static.*)))
    _settings_handler_static_list_end = .;
    . = ALIGN(4);
  } > m_text

 .ctors :
  {
    __CTOR_LIST__ = .;
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } > m_text

  .dtors :
  {
    __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } > m_text

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > m_text

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > m_text

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > m_text

  __etext = .;    /* define a global symbol at end of code */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization */

  .fsl_prod_info :
  {
    . = ALIGN(m_sector_size);
    *(.PROD_DATA*)
  } > m_fsl_prod_info

  .fEraseNVM :
  {
    . = ALIGN(m_sector_size);
    *(.fEraseNVM)
  } > fNVM_region

  .sEraseNVM :
  {
    . = ALIGN(m_sector_size);
    *(.sEraseNVM*)
  } > sNVM_region

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(.ramfunc*)             /* for functions in ram */
    EXCLUDE_FILE (*k32w1_nbu_ble_15_4_dyn*.bin) *(.data)                 /* .data sections */
    EXCLUDE_FILE (*k32w1_nbu_ble_15_4_dyn*.bin) *(.data*)                /* .data* sections */
    KEEP(*(.jcr*))
    . = ALIGN(4);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data0

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__);
  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")

  .stack :
  {
    . = ALIGN(8);
    /* STACK_SIZE is assumed to be multiple of 8 bytes,
     * thus __StackTop which is pointing to the end of
     * this section should be 8 byte aligned.
     */
    . += STACK_SIZE;
    __StackTop = .;
  } > m_cstack

  /* Initializes stack on the end of block */

  /* m_cstack_end is manually calculated and in the end it's not 8 byte aligned
   * breaking the AAPCS standard: 5.2.1.2 Stack constraints at a public interface
   * thus breaking va_args for 64 bit values.
   */
  /* __StackTop   = m_cstack_end; */
  __StackLimit = m_cstack_start;
  PROVIDE(__stack = __StackTop);

  .interrupts_ram :
  {
    . = ALIGN(4);
    __interrupts_ram_start__ = .; /* Create a global symbol at data start */
    *(.m_interrupts_ram)     /* This is a user defined section */
    . = ALIGN(4);
    __interrupts_ram_end__ = .; /* Define a global symbol at data end */
  } > m_ram_interrupts

  /* Uninitialized data section */
  .bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(4);
    __START_BSS = .;
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data1

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    *(.heap)
    __HeapLimit = __HEAP_end__;
  } > m_data1



  .ARM.attributes 0 : { *(.ARM.attributes) }

  /* ASSERT(__StackLimit >= __HeapLimit, "region m_data1 overflowed with stack and heap") */
}

