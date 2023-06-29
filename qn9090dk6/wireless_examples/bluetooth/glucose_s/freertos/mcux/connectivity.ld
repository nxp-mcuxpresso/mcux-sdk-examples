/*
** ###################################################################
**     Processors:          QN9090/K32W061
**
**     Compiler:            GNU C Compiler
**     Reference manual:    QN9090/K32W061 Series Reference Manual
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright  2019 - 2023 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/


/******************* Map of QN9090/K32W061 FLASH ***********************************


             0x000a_0000    m_flash_end
    - - - +---------------+ - - - - - - - -
          |   _ _ _ _ _   |
          | Flash config  |
   8.5k   |   - - - - -   |
          |               |
          |  RESERVED     |  0x9de00
    - - - +---------------+ - - - - - - - -
          |               | NV_STORAGE_START_ADDRESS
   31.5k  |               |
          |   NVM_region  |
          |               |
          |               | NV_STORAGE_END_ADDRESS
    - - - +---------------+ - - - - - - - -
          |               | INT_STORAGE_START
          |               |
          |               |
   MAX    | OTAP Internal |
   296k   |    Storage*   |
          |               |
          |               |
          |               |
          |               | INT_STORAGE_END (Must be aligned on a 8K to support the OTA remap mechanism)
    - - - +---------------+ - - - - - - - -
          |               | m_text_end
          |               |
          |               |
    MAX   |  Application  |
    296k  |               |
          |               |
          |               | m_text_start
    - - - +---------------+ - - - - - - - -
             0x0000_0000

 * - This section is optional for application that does not support OTA (gUseInternalStorageLink_d not defined or equal to 0).
In this case the application (m_text_end) could be up to NV_STORAGE_END_ADDRESS-1 and its MAX size will be 600k
 *****************************************************************************/

/* Entry Point */
ENTRY(ResetISR)


/*** Internal flash memory characteristics ***/
m_flash_start   = 0x00000000;
m_flash_end     = 0x0009FFFF;
m_flash_size    = 0x000A0000;
m_sector_size   = 512;

m_update_page_address = m_flash_size - 17 * m_sector_size;
m_config_page_address = m_flash_size - 2 * m_sector_size;   /* CONFIG page of page N-2  */


BOOT_SCRATCH_DATA_SZ = 0x400;
/*
 * stack size for the boot rom during warm boot and application
 * 256 is sufficient (pwrm_test) but keep it large to 1024
 */
BOOT_RESUME_STACK_SIZE = 0x400;


  __base_RAM_Bank0 = 0x04000000;             /* RAM0 Bank0 */
  __size_RAM_Bank0 = 0x00004000;             /* 16kB */
  __top_RAM_Bank0  = __base_RAM_Bank0 + __size_RAM_Bank0;

  __base_RAM_Bank1 = 0x04004000;             /* RAM0 Bank1 */
  __size_RAM_Bank1 = 0x00004000;             /* 16kB */
  __top_RAM_Bank1  = __base_RAM_Bank1 + __size_RAM_Bank1;

  __base_RAM_Bank2 = 0x04008000;             /* RAM0 Bank2 */
  __size_RAM_Bank2 = 0x00004000;             /* 16kB */
  __top_RAM_Bank1  = __base_RAM_Bank2 + __size_RAM_Bank2;

  __base_RAM_Bank3 = 0x0400c000;             /* RAM0 Bank3 */
  __size_RAM_Bank3 = 0x00004000;             /* 16kB */
  __top_RAM_Bank3  = __base_RAM_Bank3 + __size_RAM_Bank3;

  __base_RAM_Bank4 = 0x04010000;             /* RAM0 Bank4 */
  __size_RAM_Bank4 = 0x00002000;             /* 8kB */
  __top_RAM_Bank4  = __base_RAM_Bank4 + __size_RAM_Bank4;

  __base_RAM_Bank5 = 0x04012000;             /* RAM0 Bank5 */
  __size_RAM_Bank5 = 0x00002000;             /* 8kB */
  __top_RAM_Bank5  = __base_RAM_Bank5 + __size_RAM_Bank5;

  __base_RAM_Bank6 = 0x04014000;             /* RAM0 Bank6 */
  __size_RAM_Bank6 = 0x00001000;             /* 4kB */
  __top_RAM_Bank6  = __base_RAM_Bank6 + __size_RAM_Bank6;

  __base_RAM_Bank7 = 0x04015000;             /* RAM0 Bank7 */
  __size_RAM_Bank7 = 0x00001000;             /* 4kB */
  /*
   * the 32 top bytes of the RAM0/Bank7 are used for WarmBoot.
   * These impose that the Bank7 remain under retention.
   */
  __BootInterfaceSz       = 32;
  __top_RAM_Bank7  = __base_RAM_Bank7 + __size_RAM_Bank7 -__BootInterfaceSz ;

  __base_RAM_Bank8 = 0x04020000;             /* RAM0 Bank8 */
  __size_RAM_Bank8 = 0x00004000;
  __top_RAM_Bank8  = __base_RAM_Bank8 + __size_RAM_Bank8;

  __base_RAM_Bank9 = 0x04024000;             /* RAM0 Bank9 */
  __size_RAM_Bank9 = 0x00004000;
  __top_RAM_Bank9  = __base_RAM_Bank9 + __size_RAM_Bank9;

  __base_RAM_Bank10 = 0x04028000;             /* RAM0 Bank10 */
  __size_RAM_Bank10 = 0x00004000;
  __top_RAM_Bank10  = __base_RAM_Bank10 + __size_RAM_Bank10;

  __base_RAM_Bank11 = 0x0402c000;             /* RAM0 Bank11 */
  __size_RAM_Bank11 = 0x00004000;
  __top_RAM_Bank11  = __base_RAM_Bank11 + __size_RAM_Bank11;


  /* Define a symbol for the top of each memory region */
  __base_Flash640       = 0x0;            /* Flash640 */
  __size_Flash640       = 0xa0000;        /* 640K bytes */
  __top_Flash640        = __base_Flash640 + __size_Flash640; 
  
  __base_Flash          = __base_Flash640;
  __size_Flash          = __size_Flash640;
  __top_Flash           = __top_Flash640;
  /* RAM0 bank : [0x04000000..0x04000400] trampled at Warm boot
   * Likewise we choose to place the Boot Resume stack contiguous to this area in 
   * [0x04000400..0x04000800].
   * so that we have a 2kB region that can be unretained and be used as a scratch area.
   * Ideally to minimize retained memory one should concentrate in Bank0 all data not
   * requiring retention.
   */
  __base_unretainedRAM0 = 0x4000000;
  __size_unretainedRAM0 = BOOT_SCRATCH_DATA_SZ + BOOT_RESUME_STACK_SIZE;
  __top_unretainedRAM0  = __base_unretainedRAM0 + __size_unretainedRAM0; /* start of normal use RAM0 */

  /* The remainder of Bank0 is usable it is up to the papplication designer to decide
   * to retain Bank0 or not. If so the default below allow for maximum RAM capacity, gaining 14kB
   * but requiring to retain Bank0 at the expense of power ~420nA.
   */
  __base_RAM0           = __top_unretainedRAM0;      /* RAM0 */
  __size_RAM0           = __top_RAM_Bank7 - __top_unretainedRAM0;      /* RAM0 */
  /* __base_RAM will be set to __base_RAM0 if the approach favours capacity and
   * otherwise the Stack of the startup task will be placed in a way such that the top of the stack 
   * that require retention be in Bank1 while the */ 
  __base_RAM            = __base_RAM0; 
  __size_RAM            = __size_RAM0;
  __top_RAM             = __top_RAM_Bank7;
  __base_RAM1           = __base_RAM_Bank8  ;      /* RAM1 and RAM2 are an alias for the second bank*/
  __base_RAM2           = __base_RAM_Bank8 ;       /* RAM2 */
  __top_RAM1            = __top_RAM_Bank11;
  __size_RAM1           = __top_RAM_Bank11 - __base_RAM1;
  __top_RAM2            = __top_RAM_Bank11;


/****************************************************
 *******         User Defines                 *******
 ****************************************************/
UseNVMLink              = DEFINED(gUseNVMLink_d)            ? gUseNVMLink_d             : 0;
NVMSectorCountLink      = DEFINED(gNVMSectorCountLink_d)    ? gNVMSectorCountLink_d     : ( DEFINED(gUseNVMLink_d) ? gUseNVMLink_d*63 : 0 );
ram_vector_table        = DEFINED(__ram_vector_table__)     ? __ram_vector_table__      : 0;
UseInternalStorageLink  = DEFINED(gUseInternalStorageLink_d)? gUseInternalStorageLink_d : 0;

m_app_size  = DEFINED(__app_stated_size__) ? __app_stated_size__ : 0x48000;
m_app_start = DEFINED(__app_load_address__) ? __app_load_address__ : 0x0;

m_app_end                 = m_app_start + m_app_size - 1;


HEAP_SIZE               = DEFINED(__heap_size__)            ? __heap_size__             : 0;
STACK_SIZE              = DEFINED(__stack_size__)           ? __stack_size__            : ( DEFINED(__special_stack_size__) ? __special_stack_size__ : (0x1000 - 32));
vector_table_size       = 0x120;


/****************************************************
 ******          Flash regions sizes           ******
 ****************************************************/
NV_STORAGE_SIZE         = NVMSectorCountLink * m_sector_size;
NV_STORAGE_MAX_SECTORS  = NVMSectorCountLink;
NV_STORAGE_SECTOR_SIZE = m_sector_size;

/*
*  m_int_start_addr must be 8k aligned for remap to work
* can be as big as 0x4c000 i.e. 311k
*/
m_int_start_addr        = m_app_end + 1;

/****************************************************
 ******        Flash Regions, low to high      ******
 ****************************************************/
m_text_start            = m_app_start;



/****************************************************
 *****        Flash Regions, high to low       ******
 ****************************************************/
NV_STORAGE_START_ADDRESS = m_update_page_address - 1;
NV_STORAGE_END_ADDRESS  = NV_STORAGE_START_ADDRESS - NV_STORAGE_SIZE + 1;
INT_STORAGE_START       = NV_STORAGE_END_ADDRESS - 1;
INT_STORAGE_END         = (UseInternalStorageLink) ? m_int_start_addr : INT_STORAGE_START;
INT_STORAGE_SIZE        = INT_STORAGE_START - INT_STORAGE_END;
m_text_end              = INT_STORAGE_END - 1;



/* Other Defines */
m_text_size             = m_text_end + 1 - m_text_start ;


INT_STORAGE_SECTOR_SIZE = m_sector_size;
M_VECTOR_RAM_SIZE       = DEFINED(__ram_vector_table__) ? vector_table_size : 0x0;

__StartupTaskTop = __top_RAM_Bank7;
__size_of_ram    = __size_RAM;

PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RAM_Bank7 );
__StackBottom = _vStackTop - STACK_SIZE; /* only the top 256 bytes of the startup_task are retained */


/* Specify the memory areas */
MEMORY
{
  /* Define each memory region */
  /* Internal Flash */
  Flash640        (RX)   : ORIGIN = m_app_start,     LENGTH = m_text_size

  /* SRAM */
  BANK0          (rwx)   : ORIGIN = __base_RAM_Bank0, LENGTH = __size_RAM_Bank0
  BANK1          (rwx)   : ORIGIN = __base_RAM_Bank1, LENGTH = __size_RAM_Bank1
  BANK2          (rwx)   : ORIGIN = __base_RAM_Bank2, LENGTH = __size_RAM_Bank2
  BANK3          (rwx)   : ORIGIN = __base_RAM_Bank3, LENGTH = __size_RAM_Bank3
  BANK4          (rwx)   : ORIGIN = __base_RAM_Bank4, LENGTH = __size_RAM_Bank4
  BANK5          (rwx)   : ORIGIN = __base_RAM_Bank5, LENGTH = __size_RAM_Bank5
  BANK6          (rwx)   : ORIGIN = __base_RAM_Bank6, LENGTH = __size_RAM_Bank6
  BANK7          (rwx)   : ORIGIN = __base_RAM_Bank7, LENGTH = __size_RAM_Bank7

  SCRATCH_RAM    (rwx)   : ORIGIN = __base_RAM_Bank0, LENGTH = __size_unretainedRAM0   /* 2K bytes (alias SCRATCH_RAM) */

  RAM0           (rwx)   : ORIGIN = __base_RAM, LENGTH = __size_of_ram    /* 87K bytes (alias RAM) */
  RAM1           (rwx)   : ORIGIN = __base_RAM1, LENGTH = __size_RAM1  /* 64K bytes (alias RAM2) */
}



SECTIONS
{
    /* MAIN TEXT SECTION */
    .m_interrupts : ALIGN(4)
    {
        _flash_start = ABSOLUTE(.);
        _flash_beg = ABSOLUTE(.);

        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        __VECTOR_TABLE = .;
        KEEP(*(.isr_vector))
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    /* MAIN TEXT SECTION */
    .data_bss : ALIGN(10)
    {
        /* Global Section Table */
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM2));
        LONG(    ADDR(.data_RAM2));
        LONG(  SIZEOF(.data_RAM2));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM2));
        LONG(  SIZEOF(.bss_RAM2));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .ro_nonce : ALIGN(0x10)
    {
        _FlsNonceStart = ABSOLUTE(.);
        *(.ro_nonce) /* nonce value is 16 bytes.*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .ro_ota_header : ALIGN(0x10)
    {
        _enc_start = ABSOLUTE(.);
        _enc_offset = (_enc_start & 0x0000000F);
        _FlsOtaHeader = ABSOLUTE(.);
        *(.ro_ota_header) /* Ota Header 69 bytes*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .ro_se_lnkKey (ALIGN((. - _enc_offset), 16) + _enc_offset):
    {
        _FlsLinkKey = ABSOLUTE(.);
        *(.ro_se_lnkKey)  /* Link Key 16 bytes*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .filler :
    {
        BYTE(0xff)
        FILL(0xff);
        . = ALIGN(0x40);
    } > Flash640

    .text : ALIGN(0x40)
    {
        FILL(0xff)

       *(.after_vectors*)
       *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
    } > Flash640

    .NVM_TABLE :
    {
      . = ALIGN(4);
      PROVIDE(__start_NVM_TABLE = .);
      KEEP(*(.NVM_TABLE));
      PROVIDE(__stop_NVM_TABLE = .);
      . = ALIGN(4);
    } > Flash640

    .VERSION_TAGS :
    {
        . = ALIGN(4);
        PROVIDE(__start_VERSION_TAGS = .);
        KEEP(*(.VERSION_TAGS));
        PROVIDE(__stop_VERSION_TAGS = .);
        . = ALIGN(4);
    } > Flash640

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > Flash640
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > Flash640
    __exidx_end = .;

    _etext = .;

    /* DATA section for RAM1 */
    .data_RAM2 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM2 = .) ;
        *(.ramfunc.$RAM2)
        *(.ramfunc.$RAM1)
        *(.data.$RAM2*)
        *(.data.$RAM1*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM2 = .) ;
     } > RAM1 AT>Flash640

    .interrupts_ram :
    {
        . = ALIGN(4);
        __VECTOR_RAM__ = .;
        __interrupts_ram_start__ = .;   /* Create a global symbol at data start */
        *(.m_interrupts_ram)            /* This is a user defined section */
        . += M_VECTOR_RAM_SIZE;
        . = ALIGN(4);
        __interrupts_ram_end__ = .;     /* Define a global symbol at data end */
    } > RAM0
    .scratch_area (NOLOAD): ALIGN(4)
    {
       __scratch_area_start__ = .;
       . = ALIGN(4) ;
       . += 0x400;
       _boot_resume_stack = .;
       . += BOOT_RESUME_STACK_SIZE;
       _end_boot_resume_stack = .;
       __scratch_area_top__ = .;

       /* Between 0x04000800 and __StackBottom we have available space to be used for unretained
        * data preferably. Hint may be used for communication buffers. Unfortunately not possible
        * for BLE exchange memory
        */
    } > SCRATCH_RAM
    /* Main DATA section (RAM0) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
    } > RAM0 AT>Flash640

    __VECTOR_RAM = __VECTOR_RAM__;
    __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

    /* BSS section for RAM1 */
    .bss_RAM2 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
       . = ALIGN(4);
       *(.bss_RAM2)
       *(.bss.$RAM2*)
       *(.bss.$RAM1*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM2 = .) ;
    } > RAM1

    /* MAIN BSS SECTION */
    .bss (NOLOAD) : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
       /* PDM discard bss */
        *(.discard.bss.*)
        . = ALIGN(4) ;
        _ebss = .;

        PROVIDE(end = .);
    } > RAM0

    .ll_exchange_mem (NOLOAD): ALIGN(4)
    {
        __ll_exchange_mem_start__ = .;
        . = ALIGN(4) ;
        *(.ll.exchange_mem*)
        __ll_exchange_mem_top__ = .;
    } > RAM0

    /* NOINIT section for RAM1 */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM2*)
       *(.noinit.$RAM1*)
       . = ALIGN(4) ;
    } > RAM1

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*)
        . = ALIGN(4) ;
        _end_noinit = .;
    } > RAM0
    /* For now, Heap does not need to ne retained as configuration is baremetal
       and no allocator is implemented. Used only ponctually by stdio (sprintf) */
    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_pvHeapLimit = _pvHeapStart + (HEAP_SIZE));
    /* end of firmware RAM to be retained in power down mode only if MemManagerLight unused */
    _end_fw_retention = .;

    /* MemManagerLight heap : top can reach up to top of Bank6 but retained area may
     * recede lower than Bank frontiers allowing to avoid retention of Banks  */
    .heap (NOLOAD): ALIGN(4)
    {
        __HEAP_start__ = .;
        *(.heap*)
        . = ALIGN(4) ;
    } > RAM0
    __HEAP_end__ = __top_RAM_Bank6; /* top of Heap in RAM0 */

    PROVIDE(_scratch_buf_start = __scratch_area_start__);
    PROVIDE(_scratch_buf_end = __scratch_area_top__);
}

ASSERT(m_app_size % 8192 == 0, "Stated size must be a multiple of 8k")

GROUP(libcr_nohost.a libcr_c.a libcr_eabihelpers.a)
