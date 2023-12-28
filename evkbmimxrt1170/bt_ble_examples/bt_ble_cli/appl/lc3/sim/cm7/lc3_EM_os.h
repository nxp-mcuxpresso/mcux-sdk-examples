
/**
 *  \file EM_os.h
 *
 *  This header file is part of EtherMind OS Abstraction module.
 *  In this file the platform specific data types are abstracted and
 *  mapped to data types used within the EtherMind Stack.
 *
 *  Version: Windows User Mode
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifdef LC3_TEST

#ifndef _H_EM_OS_
#define _H_EM_OS_

/* -------------------------------------------- Header File Inclusion */
/* Platform headers */
#include "lc3_EM_platform.h"

/* -------------------------------------------- Global Definitions */
#define EM_SUCCESS                  0x0000
#define EM_FAILURE                  0xFFFF

/* -------------------------------------------- Structures/Data Types */
/* 'signed' data type of size '1 octet' */
typedef char CHAR;

/* 'signed' data type of size '1 octet' */
typedef char INT8;

/* 'unsigned' data type of size '1 octet' */
typedef unsigned char UCHAR;

/* 'unsigned' data type of size '1 octet' */
typedef unsigned char UINT8;

/* 'signed' data type of size '2 octet' */
typedef short int INT16;

/* 'unsigned' data type of size '2 octet' */
typedef unsigned short int UINT16;

/* 'signed' data type of size '2 octet' */
typedef int INT32;

/* 'unsigned' data type of size '2 octet' */
typedef unsigned int UINT32;

/* 'unsigned' data type of size '8 octet' */
typedef unsigned char UINT64_T[8];

/* 'unsigned' data type of size '1 octet' */
typedef unsigned char BOOLEAN;

/* Function Return Value type */
typedef UINT16 EM_RESULT;

/* --------------------------------------------------- Macros */

/* Abstractions for String library functions */
#define EM_str_len(s)                 strlen((char *)(s))
#define EM_str_n_len(s, sz)           strnlen((char *)(s), (sz))
#define EM_str_copy(d, s)             (void)strcpy((char *)(d), (char *)(s))
#define EM_str_n_copy(d, s, n)        (void)strncpy((char *)(d), (char *)(s), n)
#define EM_str_cmp(s1, s2)            strcmp((char *)(s1), (char *)(s2))
#define EM_str_n_cmp(s1, s2, n)       strncmp((char *)(s1), (char *)(s2), n)
#define EM_str_cat(d, s)              (void)strcat((char *)(d), (char *)(s))
#define EM_str_n_cat(d, s, sz)        (void)strncat((char *)(d), (char *)(s), (sz))
#define EM_str_str(s, ss)             strstr((char *)(s), (char *)(ss))
#define EM_str_n_casecmp(s1, s2, n)   _strnicmp ((char *)(s1), (char *)(s2), n)
#define EM_str_print(...)             (void)sprintf(__VA_ARGS__)

/* Abstractions for memory functions */
#define EM_mem_move(d, s, n)          (void)memmove((d), (s), (n))
#define EM_mem_cmp(p1, p2, n)         memcmp((p1), (p2), (n))
#define EM_mem_set(p, v, n)           (void)memset((p), (v), (n))
#define EM_mem_copy(p1, p2, n)        (void)memcpy((p1), (p2), (n))

#define EM_alloc_mem(sz)              malloc(sz)
#define EM_free_mem(ptr)              free(ptr)

/* -------------------------------------------- Data Structures */

/* -------------------------------------------- Function Declarations */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
};
#endif

#endif /* _H_EM_OS_ */
#endif
