//*****************************************************************************
// cr_section_macros.h
//
// Set of macros to allow code/data to be placed into alternate memory banks
//*****************************************************************************
//
// Copyright 2010-2012 Code Red Technologies Ltd
// Copyright 2013-2014, 2017-2018, 2020-2021 NXP
// All rights reserved.
//
// NXP Confidential. This software is owned or controlled by NXP and may only be
// used strictly in accordance with the applicable license terms.
//
// By expressly accepting such terms or by downloading, installing, activating
// and/or otherwise using the software, you are agreeing that you have read, and
// that you agree to comply with and are bound by, such license terms.
// 
// If you do not agree to be bound by the applicable license terms, then you may not
// retain, install, activate or otherwise use the software.
//*****************************************************************************
#ifndef CR_SECTION_MACROS_H_INCLUDED
#define CR_SECTION_MACROS_H_INCLUDED

// A macro for placing text (code), data, or bss into a named RAM section
// These will be automatically placed into the named section by the linker.
//
// RAM banks are numbered starting from 1. The actual configuration is 
// dependent on the selected MCU type.
//
// Example:
//        __SECTION(data,RAM1) char buffer[1024] ;
//
// This will place the 1024 byte buffer into the RAM1
//
#define __SECTION_EXT(type, bank, name) __attribute__ ((section("." #type ".$" #bank "." #name)))
#define __SECTION(type, bank) __attribute__ ((section("." #type ".$" #bank)))
#define __SECTION_SIMPLE(type) __attribute__ ((section("." #type)))

#define __DATA_EXT(bank, name) __SECTION_EXT(data, bank, name)
#define __DATA(bank) __SECTION(data, bank)

#define __BSS_EXT(bank, name) __SECTION_EXT(bss, bank, name)
#define __BSS(bank) __SECTION(bss, bank)

// Macros for placing text (code), data, or bss into a section that is automatically
// placed after the vectors in the target image.
#define __AFTER_VECTORS_EXT(name) __attribute__ ((section(".after_vectors.$" #name)))
#define __AFTER_VECTORS __attribute__ ((section(".after_vectors")))

// Macros for causing functions to be relocated to RAM
#define __RAM_FUNC_EXT(name) __attribute__ ((section(".ramfunc.$" #name)))
#define __RAM_FUNC __attribute__ ((section(".ramfunc")))

// Macros to be used in preference to __RAM_FUNC to better match __DATA behaviour
#define __RAMFUNC_EXT(bank, name) __SECTION_EXT(ramfunc, bank, name)
#define __RAMFUNC(bank) __SECTION(ramfunc, bank)

// Macros for placing data or bss into a section that has the NOLOAD option set in the linker script
#define __NOINIT_DEF __SECTION_SIMPLE(noinit)
#define __NOINIT_EXT(bank, name) __SECTION_EXT(noinit, bank, name)
#define __NOINIT(bank) __SECTION(noinit, bank)

// Macros for placing text (code), or rodata into a different (flash) bank
#define __RODATA_EXT(bank,name) __SECTION_EXT(rodata, bank, name)
#define __RODATA(bank) __SECTION(rodata, bank)

#define __TEXT_EXT(bank,name) __SECTION_EXT(text, bank, name)
#define __TEXT(bank) __SECTION(text, bank)

#endif /* CR_SECTION_MACROS_H_INCLUDED */
