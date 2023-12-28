/*
** ###################################################################
**     Processors:          MIMXRT595SFAWC_cm33
**                          MIMXRT595SFFOC_cm33
**
**     Compiler:            GNU C Compiler
**     Reference manual:    iMXRT500RM Rev.0, 01/2021
**     Version:             rev. 4.0, 2020-05-12
**     Build:               b210303
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2021,2023 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/



/* Entry Point */
ENTRY(Reset_Handler)

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0x1000;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0400;
M_VECTOR_RAM_SIZE = DEFINED(__ram_vector_table__) ? 0x00000180 : 0;

/* Specify the memory areas */
/* The SRAM region [0x10000-0x1BFFF] is reserved for ROM code. */
/* The SRAM region [0x0-0xFFFF], [0x1C000-0x1FFFF] are reserved for app-specific use cases. */
/* The SRAM region [0x20000-0x7FFFF] is reserved for Non-cached shared memory between M33 and DSP. */
/* The SRAM region [0x80000-0x27FFFF] is reserved for DSP code and data. */

LITTLEFS_REGION_SIZE = 0x00800000;

m_text_start         = 0x08001180;
m_text_size          = 0x03FFEE80 - LITTLEFS_REGION_SIZE;

MEMORY
{
  m_flash_config        (RX)  : ORIGIN = 0x08000400, LENGTH = 0x00000200
  m_interrupts          (RX)  : ORIGIN = 0x08001000, LENGTH = 0x00000180
  m_text                (RX)  : ORIGIN = m_text_start, LENGTH = m_text_size
  m_data                (RW)  : ORIGIN = 0x20080000, LENGTH = 0x00280000
  m_usb_sram            (RW)  : ORIGIN = 0x40140000, LENGTH = 0x00004000
}

/* NOTE: The last LITTLEFS_REGION_SIZE Flash( in range (m_text_start + m_text_size) ~ (m_text_start + m_text_size + LITTLEFS_REGION_SIZE -1)) is reserved for litteFS data */

/*** littleFS configuration Start ***/
EDGEFAST_BT_LITTLEFS_STORAGE_START_ADDRESS = m_text_start + m_text_size;
EDGEFAST_BT_LITTLEFS_STORAGE_SECTOR_SIZE   = 0x1000;  /* 4k flash sector size */
EDGEFAST_BT_LITTLEFS_STORAGE_MAX_SECTORS   = LITTLEFS_REGION_SIZE / EDGEFAST_BT_LITTLEFS_STORAGE_SECTOR_SIZE;
/*** littleFS configuration End ***/

/* Define output sections */
SECTIONS
{
  .flash_config :
  {
    . = ALIGN(4);
    __FLASH_BASE = .;
    KEEP(* (.flash_conf))     /* flash config section */
    . = ALIGN(4);
  } > m_flash_config

  /* The startup code goes first into internal ram */
  .interrupts :
  {
    . = ALIGN(4);
    __VECTOR_TABLE = .;
    __Vectors = .;
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  /* The program code and other data goes into internal ram */
  .text :
  {
    . = ALIGN(4);
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj
        */controller_low_power.c.obj
        */board_lp.c.obj
        */fwk_platform.c.obj
        */fwk_platform_lowpower.c.obj
        */fsl_pm_core.c.obj
        */fsl_pm_device.c.obj
        */fsl_power.c.obj
        */fsl_gpio.c.obj
        */fsl_adapter_rt_gpio.c.obj
        */fsl_component_generic_list.c.obj
        */fsl_common_arm.c.obj
        *lib_a-memset.o
    ) .text)                 /* .text sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj
        */controller_low_power.c.obj
        */board_lp.c.obj
        */fwk_platform.c.obj
        */fwk_platform_lowpower.c.obj
        */fsl_pm_core.c.obj
        */fsl_pm_device.c.obj
        */fsl_power.c.obj
        */fsl_gpio.c.obj
        */fsl_adapter_rt_gpio.c.obj
        */fsl_component_generic_list.c.obj
        */fsl_common_arm.c.obj
        *lib_a-memset.o
    ) .text*)                /* .text* sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj
        */controller_low_power.c.obj
        */board_lp.c.obj
        */fwk_platform.c.obj
        */fwk_platform_lowpower.c.obj
        */fsl_pm_core.c.obj
        */fsl_pm_device.c.obj
        */fsl_power.c.obj
        */fsl_gpio.c.obj
        */fsl_adapter_rt_gpio.c.obj
        */fsl_component_generic_list.c.obj
        */fsl_common_arm.c.obj
        *lib_a-memset.o
    ) .rodata)               /* .rodata sections (constants, strings, etc.) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj
        */controller_low_power.c.obj
        */board_lp.c.obj
        */fwk_platform.c.obj
        */fwk_platform_lowpower.c.obj
        */fsl_pm_core.c.obj
        */fsl_pm_device.c.obj
        */fsl_power.c.obj
        */fsl_gpio.c.obj
        */fsl_adapter_rt_gpio.c.obj
        */fsl_component_generic_list.c.obj
        */fsl_common_arm.c.obj
        *lib_a-memset.o
    ) .rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  } > m_text
  ._bt_settings_area :
  {
    . = ALIGN(4);
    _bt_settings_start = .;
    KEEP(*(SORT(._bt_settings.static.*)))
    _bt_settings_end = .;
    . = ALIGN(4);
  } > m_text

  ._settings_handler_static :
  {
    . = ALIGN(4);
    _settings_handler_static_list_start = .;
    KEEP(*(SORT(._settings_handler_static.static.*)))
    _settings_handler_static_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_conn_cb :
  {
    . = ALIGN(4);
    _bt_conn_cb_list_start = .;
    KEEP(*(SORT(._bt_conn_cb.static.*)))
    _bt_conn_cb_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_gatt_service_static :
  {
    . = ALIGN(4);
    _bt_gatt_service_static_list_start = .;
    KEEP(*(SORT(._bt_gatt_service_static.static.*)))
    _bt_gatt_service_static_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_l2cap_fixed_chan :
  {
    . = ALIGN(4);
    _bt_l2cap_fixed_chan_list_start = .;
    KEEP(*(SORT(._bt_l2cap_fixed_chan.static.*)))
    _bt_l2cap_fixed_chan_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_l2cap_br_fixed_chan :
  {
    . = ALIGN(4);
    _bt_l2cap_br_fixed_chan_list_start = .;
    KEEP(*(SORT(._bt_l2cap_br_fixed_chan.static.*)))
    _bt_l2cap_br_fixed_chan_list_end = .;
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

  .interrupts_ram :
  {
    . = ALIGN(4);
    __VECTOR_RAM__ = .;
    __interrupts_ram_start__ = .; /* Create a global symbol at data start */
    *(.m_interrupts_ram)     /* This is a user defined section */
    . += M_VECTOR_RAM_SIZE;
    . = ALIGN(4);
    __interrupts_ram_end__ = .; /* Define a global symbol at data end */
  } > m_data

  __VECTOR_RAM = DEFINED(__ram_vector_table__) ? __VECTOR_RAM__ : ORIGIN(m_interrupts);
  __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(CodeQuickAccess)       /* CodeQuickAccess sections */
    *(DataQuickAccess)       /* DataQuickAccess sections */
    *(m_usb_dma_init_data)
    /* Explicit placement of flash and frequently executed functions in RAM */
    */fsl_adapter_flexspi_nor_flash.c.obj
    */mflash_drv.c.obj
    */fsl_flexspi.c.obj
    */controller_low_power.c.obj
    */board_lp.c.obj
    */fwk_platform.c.obj
    */fwk_platform_lowpower.c.obj
    */fsl_pm_core.c.obj
    */fsl_pm_device.c.obj
    */fsl_power.c.obj
    */fsl_gpio.c.obj
    */fsl_adapter_rt_gpio.c.obj
    */fsl_component_generic_list.c.obj
    */fsl_common_arm.c.obj
    *lib_a-memset.o(.text*)
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    KEEP(*(.jcr*))
    . = ALIGN(4);
    _k_mem_slab_list_start = .;
    KEEP(*(SORT(._k_mem_slab*)))
    _k_mem_slab_list_end = .;
    . = ALIGN(4);
    _net_buf_pool_list = .;
    KEEP(*(SORT(._net_buf_pool*)))
    . = ALIGN(4);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__);
  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")
  _image_size = __DATA_END - __VECTOR_TABLE;

  /* Uninitialized data section */
  .bss :
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
  } > m_data

  .heap :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    . += HEAP_SIZE;
    __HeapLimit = .;
    __heap_limit = .; /* Add for _sbrk */
  } > m_data

  .stack :
  {
    . = ALIGN(8);
    . += STACK_SIZE;
  } > m_data

  m_usb_bdt (NOLOAD) :
  {
    . = ALIGN(512);
    *(m_usb_bdt)
  } > m_usb_sram

  m_usb_global (NOLOAD) :
  {
    *(m_usb_global)
  } > m_usb_sram

  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data) + LENGTH(m_data);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);

  .ARM.attributes 0 : { *(.ARM.attributes) }

  ASSERT(__StackLimit >= __HeapLimit, "region m_data overflowed with stack and heap")
}

