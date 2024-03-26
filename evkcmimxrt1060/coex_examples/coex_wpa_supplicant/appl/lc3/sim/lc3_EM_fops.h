#ifdef LC3_TEST

/**
 *  \file EM_fops.h
 *
 *  This file has all definitions of constants and declarations APIs
 *  for File Operations.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_EM_FOPS_
#define _H_EM_FOPS_

/* ------------------------- Header File Inclusion */
#include "lc3_EM_os.h"
#include "lc3_EM_debug.h"
#include "ff.h"
   
/* ------------------------- Debug Macros */
#define EM_FOPS_ERR                     EM_debug_error
#define EM_FOPS_TRC                     EM_debug_trace
#define EM_FOPS_INF                     EM_debug_info
#define EM_FOPS_debug_dump_bytes        EM_debug_dump_bytes

/* ------------------------- Global Definitions */
/* Platform type for File handle */
typedef FILE * EM_fops_file_handle;

/** Bitmap mask for File Attributes */
#define EM_FOPS_MASK_READONLY                 0x00000001
#define EM_FOPS_MASK_HIDDEN                   0x00000002
#define EM_FOPS_MASK_SYSTEM                   0x00000004
#define EM_FOPS_MASK_FOLDER                   0x00000010
#define EM_FOPS_MASK_ARCHIVE                  0x00000020

/** File Seek Positions */
#define EM_FOPS_SEEK_SET                      SEEK_SET
#define EM_FOPS_SEEK_CUR                      SEEK_CUR
#define EM_FOPS_SEEK_END                      SEEK_END

#define EM_FOPS_BASE                          "1:"
#define EM_FOPS_PATH_SEP                      "/"
#define EM_FOPS_MAX_DIRECTORY_SIZE            256
#define EM_FOPS_MAX_FN_SIZE                   256

/** FOPS Error */
#define EM_FOPS_ERR_ID                          0xF000

#define EM_FOPS_MUTEX_INIT_FAILED               \
        (MUTEX_INIT_FAILED | EM_FOPS_ERR_ID)
#define EM_FOPS_COND_INIT_FAILED                \
        (COND_INIT_FAILED | EM_FOPS_ERR_ID)
#define EM_FOPS_MUTEX_LOCK_FAILED               \
        (MUTEX_LOCK_FAILED | EM_FOPS_ERR_ID)
#define EM_FOPS_MUTEX_UNLOCK_FAILED             \
        (MUTEX_UNLOCK_FAILED | EM_FOPS_ERR_ID)
#define EM_FOPS_MEMORY_ALLOCATION_FAILED        \
        (MEMORY_ALLOCATION_FAILED | EM_FOPS_ERR_ID)

#define EM_FOPS_ERR_GET_CURRECT_DIRECTORY       (0x0011 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_GET_FILE_ATTRIBUTES         (0x0012 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_SET_PATH_FORWARD            (0x0013 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_SET_PATH_BACKWARD           (0x0014 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_CREATE_FOLDER               (0x0015 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_OPEN                   (0x0016 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_WRITE                  (0x0017 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_READ                   (0x0018 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FOLDER_DELETE               (0x0019 | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_DELETE                 (0x001A | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_NOT_FOUND              (0x001B | EM_FOPS_ERR_ID)
#define EM_FOPS_INVALID_PARAMETER_VALUE         (0x001C | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_SEEK_FAILED            (0x001D | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_COPY                   (0x001E | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_FILE_MOVE                   (0x001F | EM_FOPS_ERR_ID)
#define EM_FOPS_ERR_SET_FILE_ATTRIBUTES         (0x0020 | EM_FOPS_ERR_ID)

/* ------------------------- Macros */
/** Create File path with seperator */
#define EM_FOPS_PATH_JOIN(a,b)                a EM_FOPS_PATH_SEP b

/** To set/get/reset a Bit in File Operations Bitmaps */
#define EM_FOPS_SET_BIT(bitmap, mask)         (bitmap) |= (mask)
#define EM_FOPS_GET_BIT(bitmap, mask)         (((bitmap) & (mask)) == (mask))
#define EM_FOPS_RESET_BIT(bitmap, mask)       (bitmap) ^= (mask)

#define EM_fops_file_print(fd,...)            (void)fprintf((fd), __VA_ARGS__)

/* -------------------------------------------- Data Structures */
typedef struct _EM_FOPS_FILINFO
{
    CHAR    fname[EM_FOPS_MAX_FN_SIZE];
    UINT32  fhsize;
    UINT32  flsize;

    UINT32  fdyear;
    UINT32  fdmonth;
    UINT32  fdday;

    UINT32  fthour;
    UINT32  ftmin;
    UINT32  ftsec;

    UINT16  fattrib;
} EM_FOPS_FILINFO;

/* ------------------------- Function Declarations */

EM_RESULT EM_fops_get_current_directory
           (
               /* OUT */  UCHAR  * current_directory
           );

EM_RESULT EM_fops_get_file_attributes
           (
               /* IN */   UCHAR   * object_name,
               /* OUT */  UINT32  * file_attribute
           );

EM_RESULT EM_fops_set_file_attributes
           (
               /* IN */   UCHAR   * object_name,
               /* IN */   UINT32    file_attribute
           );

EM_RESULT EM_fops_set_path_forward
           (
               /* IN */  UCHAR *folder_name
           );

EM_RESULT EM_fops_set_path_backward( void );

EM_RESULT EM_fops_create_folder
           (
               /* IN */  UCHAR * folder_name
           );

EM_RESULT EM_fops_file_open
           (
               /* IN */  UCHAR                * file_name,
               /* IN */  UCHAR                * mode,
               /* OUT */ EM_fops_file_handle  *file_handle
           );

EM_RESULT EM_fops_file_write
           (
               /* IN */   UCHAR               * buffer,
               /* IN */   UINT16                buf_length,
               /* IN */   EM_fops_file_handle   file_handle,
               /* OUT */  UINT16              * bytes_written
           );

EM_RESULT EM_fops_file_read
           (
               /* IN */   UCHAR               * buffer,
               /* IN */   UINT16                buf_length,
               /* IN */   EM_fops_file_handle   file_handle,
               /* OUT */  UINT16              * bytes_read
           );

#if 0
EM_RESULT EM_fops_file_print
          (
              /* IN */   EM_fops_file_handle   file_handle,
              /* IN */   CHAR * fmt,
              /* IN */   ...
          );
#endif /* 0 */

EM_RESULT EM_fops_file_put
          (
              /* IN */   EM_fops_file_handle   file_handle,
              /* IN */   UCHAR               * buffer,
              /* INOUT */   UINT16           * buf_length
          );

EM_RESULT EM_fops_file_get
          (
              /* IN */   EM_fops_file_handle   file_handle,
              /* IN */   UCHAR               * buffer,
              /* INOUT */   UINT16           * buf_length
          );

EM_RESULT EM_fops_file_get_formatted
          (
              /* IN */    EM_fops_file_handle   file_handle,
              /* IN */    const CHAR          * format,
              /* INOUT */ void                * parameter,
              /* INOUT */ UINT16              * length
          );

EM_RESULT EM_fops_file_close
           (
               /* IN */  EM_fops_file_handle  file_handle
           );

EM_RESULT EM_fops_object_delete
           (
               /* IN */  UCHAR * object_name
           );

EM_RESULT EM_fops_file_size
           (
               /* IN */  EM_fops_file_handle   file_handle,
               /* OUT */ UINT32              * file_size
           );

EM_RESULT EM_fops_file_seek
           (
               /* IN */  EM_fops_file_handle   file_handle,
               /* IN */  INT32                 offset,
               /* IN */  INT32                 whence
           );

EM_RESULT EM_fops_file_copy
           (
               /* IN */  UCHAR * existing_file_name,
               /* IN */  UCHAR * new_file_name,
               /* IN */  UCHAR   fail_if_exists
           );

EM_RESULT EM_fops_file_move
           (
               /* IN */  UCHAR * existing_file_name,
               /* IN */  UCHAR * new_file_name
           );

EM_RESULT EM_vfops_create_object_name
           (
               /* IN */  UCHAR * path,
               /* IN */  UCHAR * object,
               /* OUT */ UCHAR * obj_name
           );

EM_RESULT EM_vfops_set_path_backward
           (
               /* OUT */ UCHAR *path
           );

EM_RESULT EM_vfops_set_path_forward
           (
               /* INOUT */  UCHAR * path,
               /* IN */     UCHAR * folder
           );

void EM_fops_list_directory (CHAR * path);

#endif /* _H_EM_FOPS_ */
#endif
