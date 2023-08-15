/* This linker script generated from xt-genldscripts.tpp for LSP */
/* Linker Script for default link */
MEMORY
{
  iram1_0_seg :                       	org = 0x00000000, len = 0x400
  iram1_1_seg :                       	org = 0x00180400, len = 0x17C
  iram1_2_seg :                       	org = 0x0018057C, len = 0x20
  iram1_3_seg :                       	org = 0x0018059C, len = 0x20
  iram1_4_seg :                       	org = 0x001805BC, len = 0x20
  iram1_5_seg :                       	org = 0x001805DC, len = 0x20
  iram1_6_seg :                       	org = 0x001805FC, len = 0x20
  iram1_7_seg :                       	org = 0x0018061C, len = 0x20
  iram1_8_seg :                       	org = 0x0018063C, len = 0x400
  iram1_9_seg :                       	org = 0x00180A3C, len = 0xFF5C4
  dram0_0_seg :                       	org = 0x00840000, len = 0x13E000
  dram0_1_seg :                       	org = 0x0097E000, len = 0x2000
}

PHDRS
{
  iram1_0_phdr PT_LOAD;
  iram1_1_phdr PT_LOAD;
  iram1_2_phdr PT_LOAD;
  iram1_3_phdr PT_LOAD;
  iram1_4_phdr PT_LOAD;
  iram1_5_phdr PT_LOAD;
  iram1_6_phdr PT_LOAD;
  iram1_7_phdr PT_LOAD;
  iram1_8_phdr PT_LOAD;
  iram1_9_phdr PT_LOAD;
  noCache_phdr PT_LOAD;
  dram0_0_phdr PT_LOAD;
  dram0_0_bss_phdr PT_LOAD;
  dram0_1_phdr PT_LOAD;
  dram0_1_bss_phdr PT_LOAD;
}


/*  Default entry point:  */
ENTRY(_ResetVector)


/*  Memory boundary addresses:  */
_memmap_mem_iram0_start = 0x0;
_memmap_mem_iram0_end   = 0x400000;
_memmap_mem_iram1_start = 0x400000;
_memmap_mem_iram1_end   = 0x800000;
_memmap_mem_dram0_start = 0x800000;
_memmap_mem_dram0_end   = 0xc00000;
_memmap_mem_dram1_start = 0xc00000;
_memmap_mem_dram1_end   = 0x1000000;

/*  Memory segment boundary addresses:  */
_memmap_seg_iram1_0_start = 0x000000;
_memmap_seg_iram1_0_max   = 0x000400;
_memmap_seg_iram1_1_start = 0x180400;
_memmap_seg_iram1_1_max   = 0x18057c;
_memmap_seg_iram1_2_start = 0x18057c;
_memmap_seg_iram1_2_max   = 0x18059c;
_memmap_seg_iram1_3_start = 0x18059c;
_memmap_seg_iram1_3_max   = 0x1805bc;
_memmap_seg_iram1_4_start = 0x1805bc;
_memmap_seg_iram1_4_max   = 0x1805dc;
_memmap_seg_iram1_5_start = 0x1805dc;
_memmap_seg_iram1_5_max   = 0x1805fc;
_memmap_seg_iram1_6_start = 0x1805fc;
_memmap_seg_iram1_6_max   = 0x18061c;
_memmap_seg_iram1_7_start = 0x18061c;
_memmap_seg_iram1_7_max   = 0x18063c;
_memmap_seg_iram1_8_start = 0x18063c;
_memmap_seg_iram1_8_max   = 0x180a3c;
_memmap_seg_iram1_9_start = 0x180a3c;
_memmap_seg_iram1_9_max   = 0x280000;
_memmap_seg_dram0_0_start = 0x840000;
_memmap_seg_dram0_0_max   = 0x97e000;
_memmap_seg_dram0_1_start = 0x97e000;
_memmap_seg_dram0_1_max   = 0x980000;

_rom_store_table = 0;
PROVIDE(_memmap_reset_vector = 0x000000);
PROVIDE(_memmap_vecbase_reset = 0x180400);
/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x00021002;
_memmap_cacheattr_wt_base = 0x00021002;
_memmap_cacheattr_bp_base = 0x00022002;
_memmap_cacheattr_unused_mask = 0xFFF00FF0;
_memmap_cacheattr_wb_trapnull = 0x22221222;
_memmap_cacheattr_wba_trapnull = 0x22221222;
_memmap_cacheattr_wbna_trapnull = 0x22221222;
_memmap_cacheattr_wt_trapnull = 0x22221222;
_memmap_cacheattr_bp_trapnull = 0x22222222;
_memmap_cacheattr_wb_strict = 0xFFF21FF2;
_memmap_cacheattr_wt_strict = 0xFFF21FF2;
_memmap_cacheattr_bp_strict = 0xFFF22FF2;
_memmap_cacheattr_wb_allvalid = 0x22221222;
_memmap_cacheattr_wt_allvalid = 0x22221222;
_memmap_cacheattr_bp_allvalid = 0x22222222;
_memmap_region_map = 0x00000019;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

SECTIONS
{

  .ResetVector.text : ALIGN(4)
  {
    _ResetVector_text_start = ABSOLUTE(.);
    KEEP (*(.ResetVector.text))
    . = ALIGN (4);
    _ResetVector_text_end = ABSOLUTE(.);
  } >iram1_0_seg :iram1_0_phdr

  .ResetHandler.text : ALIGN(4)
  {
    _ResetHandler_text_start = ABSOLUTE(.);
    *(.ResetHandler.literal .ResetHandler.text)
    . = ALIGN (4);
    _ResetHandler_text_end = ABSOLUTE(.);
    _memmap_seg_iram1_0_end = ALIGN(0x8);
  } >iram1_0_seg :iram1_0_phdr


  .WindowVectors.text : ALIGN(4)
  {
    _WindowVectors_text_start = ABSOLUTE(.);
    KEEP (*(.WindowVectors.text))
    . = ALIGN (4);
    _WindowVectors_text_end = ABSOLUTE(.);
  } >iram1_1_seg :iram1_1_phdr

  .Level2InterruptVector.literal : ALIGN(4)
  {
    _Level2InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level2InterruptVector.literal)
    . = ALIGN (4);
    _Level2InterruptVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_1_end = ALIGN(0x8);
  } >iram1_1_seg :iram1_1_phdr


  .Level2InterruptVector.text : ALIGN(4)
  {
    _Level2InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level2InterruptVector.text))
    . = ALIGN (4);
    _Level2InterruptVector_text_end = ABSOLUTE(.);
  } >iram1_2_seg :iram1_2_phdr

  .Level3InterruptVector.literal : ALIGN(4)
  {
    _Level3InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level3InterruptVector.literal)
    . = ALIGN (4);
    _Level3InterruptVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_2_end = ALIGN(0x8);
  } >iram1_2_seg :iram1_2_phdr


  .Level3InterruptVector.text : ALIGN(4)
  {
    _Level3InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level3InterruptVector.text))
    . = ALIGN (4);
    _Level3InterruptVector_text_end = ABSOLUTE(.);
  } >iram1_3_seg :iram1_3_phdr

  .DebugExceptionVector.literal : ALIGN(4)
  {
    _DebugExceptionVector_literal_start = ABSOLUTE(.);
    *(.DebugExceptionVector.literal)
    . = ALIGN (4);
    _DebugExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_3_end = ALIGN(0x8);
  } >iram1_3_seg :iram1_3_phdr


  .DebugExceptionVector.text : ALIGN(4)
  {
    _DebugExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DebugExceptionVector.text))
    . = ALIGN (4);
    _DebugExceptionVector_text_end = ABSOLUTE(.);
  } >iram1_4_seg :iram1_4_phdr

  .NMIExceptionVector.literal : ALIGN(4)
  {
    _NMIExceptionVector_literal_start = ABSOLUTE(.);
    *(.NMIExceptionVector.literal)
    . = ALIGN (4);
    _NMIExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_4_end = ALIGN(0x8);
  } >iram1_4_seg :iram1_4_phdr


  .NMIExceptionVector.text : ALIGN(4)
  {
    _NMIExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.NMIExceptionVector.text))
    . = ALIGN (4);
    _NMIExceptionVector_text_end = ABSOLUTE(.);
  } >iram1_5_seg :iram1_5_phdr

  .KernelExceptionVector.literal : ALIGN(4)
  {
    _KernelExceptionVector_literal_start = ABSOLUTE(.);
    *(.KernelExceptionVector.literal)
    . = ALIGN (4);
    _KernelExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_5_end = ALIGN(0x8);
  } >iram1_5_seg :iram1_5_phdr


  .KernelExceptionVector.text : ALIGN(4)
  {
    _KernelExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.KernelExceptionVector.text))
    . = ALIGN (4);
    _KernelExceptionVector_text_end = ABSOLUTE(.);
  } >iram1_6_seg :iram1_6_phdr

  .UserExceptionVector.literal : ALIGN(4)
  {
    _UserExceptionVector_literal_start = ABSOLUTE(.);
    *(.UserExceptionVector.literal)
    . = ALIGN (4);
    _UserExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_6_end = ALIGN(0x8);
  } >iram1_6_seg :iram1_6_phdr


  .UserExceptionVector.text : ALIGN(4)
  {
    _UserExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.UserExceptionVector.text))
    . = ALIGN (4);
    _UserExceptionVector_text_end = ABSOLUTE(.);
  } >iram1_7_seg :iram1_7_phdr

  .DoubleExceptionVector.literal : ALIGN(4)
  {
    _DoubleExceptionVector_literal_start = ABSOLUTE(.);
    *(.DoubleExceptionVector.literal)
    . = ALIGN (4);
    _DoubleExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram1_7_end = ALIGN(0x8);
  } >iram1_7_seg :iram1_7_phdr


  .DoubleExceptionVector.text : ALIGN(4)
  {
    _DoubleExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DoubleExceptionVector.text))
    . = ALIGN (4);
    _DoubleExceptionVector_text_end = ABSOLUTE(.);
  } >iram1_8_seg :iram1_8_phdr

  .iram1.text : ALIGN(4)
  {
    _iram1_text_start = ABSOLUTE(.);
    *(.iram1.literal .iram1.text)
    . = ALIGN (4);
    _iram1_text_end = ABSOLUTE(.);
    _memmap_seg_iram1_8_end = ALIGN(0x8);
  } >iram1_8_seg :iram1_8_phdr


  .srom.text : ALIGN(4)
  {
    _srom_text_start = ABSOLUTE(.);
    *(.srom.literal .srom.text)
    . = ALIGN (4);
    _srom_text_end = ABSOLUTE(.);
  } >iram1_9_seg :iram1_9_phdr

  __llvm_prf_names : ALIGN(4)
  {
    __llvm_prf_names_start = ABSOLUTE(.);
    *(__llvm_prf_names)
    . = ALIGN (4);
    __llvm_prf_names_end = ABSOLUTE(.);
  } >iram1_9_seg :iram1_9_phdr

  .sram.text : ALIGN(4)
  {
    _sram_text_start = ABSOLUTE(.);
    *(.sram.literal .sram.text)
    . = ALIGN (4);
    _sram_text_end = ABSOLUTE(.);
  } >iram1_9_seg :iram1_9_phdr

  .iram0.text : ALIGN(4)
  {
    _iram0_text_start = ABSOLUTE(.);
    *(.iram0.literal .iram0.text)
    . = ALIGN (4);
    _iram0_text_end = ABSOLUTE(.);
  } >iram1_9_seg :iram1_9_phdr

  .text : ALIGN(4)
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
    *(.entry.text)
    *(.init.literal)
    KEEP(*(.init))
    *(.literal.sort.* SORT(.text.sort.*))
    KEEP (*(.literal.keepsort.* SORT(.text.keepsort.*) .literal.keep.* .text.keep.* .literal.*personality* .text.*personality*))
    *(.literal .text .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.fini.literal)
    KEEP(*(.fini))
    *(.gnu.version)
    . = ALIGN (4);
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >iram1_9_seg :iram1_9_phdr

  .clib.text : ALIGN(4)
  {
    _clib_text_start = ABSOLUTE(.);
    *(.clib.literal .clib.text)
    . = ALIGN (4);
    _clib_text_end = ABSOLUTE(.);
  } >iram1_9_seg :iram1_9_phdr

  .rtos.text : ALIGN(4)
  {
    _rtos_text_start = ABSOLUTE(.);
    *(.rtos.literal .rtos.text)
    . = ALIGN (4);
    _rtos_text_end = ABSOLUTE(.);
  } >iram1_9_seg :iram1_9_phdr

  .srom.rodata : ALIGN(4)
  {
    _srom_rodata_start = ABSOLUTE(.);
    *(.srom.rodata)
    . = ALIGN (4);
    _srom_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .dram0.rodata : ALIGN(4)
  {
    _dram0_rodata_start = ABSOLUTE(.);
    *(.dram0.rodata)
    . = ALIGN (4);
    _dram0_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.rodata : ALIGN(4)
  {
    _clib_rodata_start = ABSOLUTE(.);
    *(.clib.rodata)
    . = ALIGN (4);
    _clib_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rtos.rodata : ALIGN(4)
  {
    _rtos_rodata_start = ABSOLUTE(.);
    *(.rtos.rodata)
    . = ALIGN (4);
    _rtos_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rodata : ALIGN(4)
  {
    _rodata_start = ABSOLUTE(.);
    *(.rodata)
    *(SORT(.rodata.sort.*))
    KEEP (*(SORT(.rodata.keepsort.*) .rodata.keep.*))
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE__ = ABSOLUTE(.);
    KEEP (*(.xt_except_table))
    KEEP (*(.gcc_except_table))
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    KEEP (*(.eh_frame))
    /*  C++ constructor and destructor tables, properly ordered:  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS__ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    . = ALIGN(4);		/* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_bss_start)
    LONG(_bss_end)
    LONG(_dram1_bss_start)
    LONG(_dram1_bss_end)
    _bss_table_end = ABSOLUTE(.);
    . = ALIGN (4);
    _rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.data : ALIGN(4)
  {
    _clib_data_start = ABSOLUTE(.);
    *(.clib.data)
    . = ALIGN (4);
    _clib_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.percpu.data : ALIGN(4)
  {
    _clib_percpu_data_start = ABSOLUTE(.);
    *(.clib.percpu.data)
    . = ALIGN (4);
    _clib_percpu_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rtos.percpu.data : ALIGN(4)
  {
    _rtos_percpu_data_start = ABSOLUTE(.);
    *(.rtos.percpu.data)
    . = ALIGN (4);
    _rtos_percpu_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rtos.data : ALIGN(4)
  {
    _rtos_data_start = ABSOLUTE(.);
    *(.rtos.data)
    . = ALIGN (4);
    _rtos_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .dram0.data : ALIGN(4)
  {
    _dram0_data_start = ABSOLUTE(.);
    *(.dram0.data)
    . = ALIGN (4);
    _dram0_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(.data)
    *(SORT(.data.sort.*))
    KEEP (*(SORT(.data.keepsort.*) .data.keep.*))
    *(.data.*)
    *(.gnu.linkonce.d.*)
    KEEP(*(.gnu.linkonce.d.*personality*))
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    KEEP(*(.jcr))
    *(__llvm_prf_cnts)
    *(__llvm_prf_data)
    *(__llvm_prf_vnds)
    . = ALIGN (4);
    _data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.bss)
    *(SORT(.bss.sort.*))
    KEEP (*(SORT(.bss.keepsort.*) .bss.keep.*))
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    *(.clib.bss)
    *(.clib.percpu.bss)
    *(.rtos.percpu.bss)
    *(.rtos.bss)
    *(.dram0.bss)
    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
    _end = ALIGN(0x8);
    PROVIDE(end = ALIGN(0x8));
    _stack_sentry = ALIGN(0x8);
    _memmap_seg_dram0_0_end = ALIGN(0x8);
  } >dram0_0_seg :dram0_0_bss_phdr

  PROVIDE(__stack = 0x97e000);
  _heap_sentry = 0x97e000;

  .dram1.rodata : ALIGN(4)
  {
    _dram1_rodata_start = ABSOLUTE(.);
    *(.dram1.rodata)
    . = ALIGN (4);
    _dram1_rodata_end = ABSOLUTE(.);
  } >dram0_1_seg :dram0_1_phdr

  .dram1.data : ALIGN(4)
  {
    _dram1_data_start = ABSOLUTE(.);
    *(.dram1.data)
    . = ALIGN (4);
    _dram1_data_end = ABSOLUTE(.);
  } >dram0_1_seg :dram0_1_phdr

  .dram1.bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _dram1_bss_start = ABSOLUTE(.);
    *(.dram1.bss)
    . = ALIGN (8);
    _dram1_bss_end = ABSOLUTE(.);
    _memmap_seg_dram0_1_end = ALIGN(0x8);
  } >dram0_1_seg :dram0_1_bss_phdr

  .debug  0 :  { *(.debug) }
  .line  0 :  { *(.line) }
  .debug_srcinfo  0 :  { *(.debug_srcinfo) }
  .debug_sfnames  0 :  { *(.debug_sfnames) }
  .debug_aranges  0 :  { *(.debug_aranges) }
  .debug_pubnames  0 :  { *(.debug_pubnames) }
  .debug_info  0 :  { *(.debug_info) }
  .debug_abbrev  0 :  { *(.debug_abbrev) }
  .debug_line  0 :  { *(.debug_line) }
  .debug_frame  0 :  { *(.debug_frame) }
  .debug_str  0 :  { *(.debug_str) }
  .debug_loc  0 :  { *(.debug_loc) }
  .debug_macinfo  0 :  { *(.debug_macinfo) }
  .debug_weaknames  0 :  { *(.debug_weaknames) }
  .debug_funcnames  0 :  { *(.debug_funcnames) }
  .debug_typenames  0 :  { *(.debug_typenames) }
  .debug_varnames  0 :  { *(.debug_varnames) }
  .xt.insn 0 :
  {
    *(.xt.insn)
    KEEP (*(.gnu.linkonce.x.*))
  }
  .xt.prop 0 :
  {
    *(.xt.prop)
    *(.xt.prop.*)
    KEEP (*(.gnu.linkonce.prop.*))
  }
  .xt.lit 0 :
  {
    *(.xt.lit)
    *(.xt.lit.*)
    KEEP (*(.gnu.linkonce.p.*))
  }
  .debug.xt.callgraph 0 :
  {
    KEEP (*(.debug.xt.callgraph .debug.xt.callgraph.* .gnu.linkonce.xt.callgraph.*))
  }
}

