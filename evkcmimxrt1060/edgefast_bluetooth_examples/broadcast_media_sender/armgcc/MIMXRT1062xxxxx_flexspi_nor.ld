/*
** ###################################################################
**     Processors:          MIMXRT1062CVJ5A
**                          MIMXRT1062CVJ5B
**                          MIMXRT1062CVL5A
**                          MIMXRT1062CVL5B
**                          MIMXRT1062DVJ6A
**                          MIMXRT1062DVJ6B
**                          MIMXRT1062DVL6A
**                          MIMXRT1062DVL6B
**                          MIMXRT1062DVN6B
**                          MIMXRT1062XVN5B
**
**     Compiler:            GNU C Compiler
**     Reference manual:    IMXRT1060RM Rev.3, 07/2021 | IMXRT106XSRM Rev.0
**     Version:             rev. 0.2, 2022-03-25
**     Build:               b221009
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2023 NXP
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

LITTLEFS_REGION_SIZE = 0x00200000;

m_text_start         = 0x60002400;
m_text_size          = 0x007FDC00 - LITTLEFS_REGION_SIZE;

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0x1000;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0400;
VECTOR_RAM_SIZE = DEFINED(__ram_vector_table__) ? 0x00000400 : 0;

/* Specify the memory areas */
MEMORY
{
  m_flash_config        (RX)  : ORIGIN = 0x60000000, LENGTH = 0x00001000
  m_ivt                 (RX)  : ORIGIN = 0x60001000, LENGTH = 0x00001000
  m_interrupts          (RX)  : ORIGIN = 0x60002000, LENGTH = 0x00000400
  m_text                (RX)  : ORIGIN = m_text_start, LENGTH = m_text_size
  m_qacode              (RX)  : ORIGIN = 0x00000000, LENGTH = 0x00020000
  m_data                (RW)  : ORIGIN = 0x20000000, LENGTH = 0x00040000
  m_data2               (RW)  : ORIGIN = 0x20200000, LENGTH = 0x000C0000
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
  __NCACHE_REGION_START = ORIGIN(m_data);
  __NCACHE_REGION_SIZE  = 0;

  .flash_config :
  {
    . = ALIGN(4);
    __FLASH_BASE = .;
    KEEP(* (.boot_hdr.conf))     /* flash config section */
    . = ALIGN(4);
  } > m_flash_config

  ivt_begin = ORIGIN(m_flash_config) + LENGTH(m_flash_config);

  .ivt : AT(ivt_begin)
  {
    . = ALIGN(4);
    KEEP(* (.boot_hdr.ivt))           /* ivt section */
    KEEP(* (.boot_hdr.boot_data))     /* boot section */
    KEEP(* (.boot_hdr.dcd_data))      /* dcd section */
    . = ALIGN(4);
  } > m_ivt

  /* The startup code goes first into internal RAM */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    __Vectors = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  /* The program code and other data goes into internal RAM */
  .text :
  {
    . = ALIGN(4);
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj

        */srCvtFrm.c.obj

        *libethermind_lc3_decoder.a:lc3_decoder_api.c.obj
        *libethermind_lc3_decoder.a:lc3_decode.c.obj
        *libethermind_lc3_encoder.a:lc3_encoder_api.c.obj
        *libethermind_lc3_encoder.a:lc3_encode.c.obj
        *libethermind_lc3_common.a:lc3_common.c.obj
        *libethermind_lc3_common.a:lc3_fft.c.obj
        *libethermind_lc3_common.a:lc3_init.c.obj
        *libethermind_lc3_common.a:lc3_math_util.c.obj
        *libethermind_lc3_common.a:lc3_tables.c.obj
    ) .text)                 /* .text sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj

        */srCvtFrm.c.obj

        *libethermind_lc3_decoder.a:lc3_decoder_api.c.obj
        *libethermind_lc3_decoder.a:lc3_decode.c.obj
        *libethermind_lc3_encoder.a:lc3_encoder_api.c.obj
        *libethermind_lc3_encoder.a:lc3_encode.c.obj
        *libethermind_lc3_common.a:lc3_common.c.obj
        *libethermind_lc3_common.a:lc3_fft.c.obj
        *libethermind_lc3_common.a:lc3_init.c.obj
        *libethermind_lc3_common.a:lc3_math_util.c.obj
        *libethermind_lc3_common.a:lc3_tables.c.obj
    ) .text*)                /* .text* sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj

        */srCvtFrm.c.obj

        *libethermind_lc3_decoder.a:lc3_decoder_api.c.obj
        *libethermind_lc3_decoder.a:lc3_decode.c.obj
        *libethermind_lc3_encoder.a:lc3_encoder_api.c.obj
        *libethermind_lc3_encoder.a:lc3_encode.c.obj
        *libethermind_lc3_common.a:lc3_common.c.obj
        *libethermind_lc3_common.a:lc3_fft.c.obj
        *libethermind_lc3_common.a:lc3_init.c.obj
        *libethermind_lc3_common.a:lc3_math_util.c.obj
        *libethermind_lc3_common.a:lc3_tables.c.obj
    ) .rodata)                 /* .rodata sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */fsl_adapter_flexspi_nor_flash.c.obj
        */mflash_drv.c.obj
        */fsl_flexspi.c.obj

        */srCvtFrm.c.obj

        *libethermind_lc3_decoder.a:lc3_decoder_api.c.obj
        *libethermind_lc3_decoder.a:lc3_decode.c.obj
        *libethermind_lc3_encoder.a:lc3_encoder_api.c.obj
        *libethermind_lc3_encoder.a:lc3_encode.c.obj
        *libethermind_lc3_common.a:lc3_common.c.obj
        *libethermind_lc3_common.a:lc3_fft.c.obj
        *libethermind_lc3_common.a:lc3_init.c.obj
        *libethermind_lc3_common.a:lc3_math_util.c.obj
        *libethermind_lc3_common.a:lc3_tables.c.obj
    ) .rodata*)                /* .rodata* sections (code) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
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
    . += VECTOR_RAM_SIZE;
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
    . = ALIGN(32);
    *(m_usb_dma_init_data)
    . = ALIGN(32);
    /* Explicit placement of flash and frequently executed functions in RAM */
    */fsl_adapter_flexspi_nor_flash.c.obj(.text .text* .rodata .rodata*)
    */mflash_drv.c.obj(.text .text* .rodata .rodata*)
    */fsl_flexspi.c.obj(.text .text* .rodata .rodata*)

    */srCvtFrm.c.obj(.text .text* .rodata .rodata*)

    *libethermind_lc3_decoder.a:lc3_decoder_api.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_decoder.a:lc3_decode.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_encoder.a:lc3_encoder_api.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_encoder.a:lc3_encode.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_common.a:lc3_common.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_common.a:lc3_fft.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_common.a:lc3_init.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_common.a:lc3_math_util.c.obj(.text .text* .rodata .rodata*)
    *libethermind_lc3_common.a:lc3_tables.c.obj(.text .text* .rodata .rodata*)

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
  } > m_data2

  __ram_function_flash_start = __DATA_ROM + (__data_end__ - __data_start__); /* Symbol is used by startup for TCM data initialization */

  .ram_function : AT(__ram_function_flash_start)
  {
    . = ALIGN(32);
    __ram_function_start__ = .;
    *(CodeQuickAccess)
    . = ALIGN(128);
    __ram_function_end__ = .;
  } > m_qacode

  __NDATA_ROM = __ram_function_flash_start + (__ram_function_end__ - __ram_function_start__);
  .ncache.init : AT(__NDATA_ROM)
  {
    __noncachedata_start__ = .;   /* create a global symbol at ncache data start */
    *(NonCacheable.init)
    . = ALIGN(4);
    __noncachedata_init_end__ = .;   /* create a global symbol at initialized ncache data end */
  } > m_data
  . = __noncachedata_init_end__;
  .ncache :
  {
    *(NonCacheable)
    . = ALIGN(4);
    __noncachedata_end__ = .;     /* define a global symbol at ncache data end */
  } > m_data

  __DATA_END = __NDATA_ROM + (__noncachedata_init_end__ - __noncachedata_start__);
  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")

  /* Uninitialized data section */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(4);
    __START_BSS = .;
    __bss_start__ = .;
    . = ALIGN(32);
    *(m_usb_dma_noninit_data)
    . = ALIGN(32);
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data2

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

  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data) + LENGTH(m_data);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);

  .ARM.attributes 0 : { *(.ARM.attributes) }

  ASSERT(__StackLimit >= __HeapLimit, "region m_data2 overflowed with stack and heap")
}

