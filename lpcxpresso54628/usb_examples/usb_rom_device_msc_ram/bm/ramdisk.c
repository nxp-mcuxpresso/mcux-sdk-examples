/*
 * @brief	USB Mass Storage data RAM module
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

/** \file
 *
 *  Functions to manage the physical Dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */
#include <stdlib.h>
#include <string.h>
#include "lpc_types.h"
#include "msc_disk.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Initial Image to be loaded into DiskImage */
static const uint8_t FAT12_header[] = {
    0xeb, 0x3c, 0x90, 0x4d, 0x53, 0x44, 0x4f, 0x53, 0x35, 0x2e, 0x30, 0x00, 0x02, 0x01, 0x01, 0x00, 0x01, 0x10, 0x00,
    0x10, 0x00, 0xf8, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x29, 0x74, 0x19, 0x02, 0x27, 0x4c, 0x50, 0x43, 0x31, 0x31, 0x78, 0x78, 0x20, 0x55, 0x53, 0x42, 0x46, 0x41, 0x54,
    0x31, 0x32, 0x20, 0x20, 0x20, 0x33, 0xc9, 0x8e, 0xd1, 0xbc, 0xf0, 0x7b, 0x8e, 0xd9, 0xb8, 0x00, 0x20, 0x8e, 0xc0,
    0xfc, 0xbd, 0x00, 0x7c, 0x38, 0x4e, 0x24, 0x7d, 0x24, 0x8b, 0xc1, 0x99, 0xe8, 0x3c, 0x01, 0x72, 0x1c, 0x83, 0xeb,
    0x3a, 0x66, 0xa1, 0x1c, 0x7c, 0x26, 0x66, 0x3b, 0x07, 0x26, 0x8a, 0x57, 0xfc, 0x75, 0x06, 0x80, 0xca, 0x02, 0x88,
    0x56, 0x02, 0x80, 0xc3, 0x10, 0x73, 0xeb, 0x33, 0xc9, 0x8a, 0x46, 0x10, 0x98, 0xf7, 0x66, 0x16, 0x03, 0x46, 0x1c,
    0x13, 0x56, 0x1e, 0x03, 0x46, 0x0e, 0x13, 0xd1, 0x8b, 0x76, 0x11, 0x60, 0x89, 0x46, 0xfc, 0x89, 0x56, 0xfe, 0xb8,
    0x20, 0x00, 0xf7, 0xe6, 0x8b, 0x5e, 0x0b, 0x03, 0xc3, 0x48, 0xf7, 0xf3, 0x01, 0x46, 0xfc, 0x11, 0x4e, 0xfe, 0x61,
    0xbf, 0x00, 0x00, 0xe8, 0xe6, 0x00, 0x72, 0x39, 0x26, 0x38, 0x2d, 0x74, 0x17, 0x60, 0xb1, 0x0b, 0xbe, 0xa1, 0x7d,
    0xf3, 0xa6, 0x61, 0x74, 0x32, 0x4e, 0x74, 0x09, 0x83, 0xc7, 0x20, 0x3b, 0xfb, 0x72, 0xe6, 0xeb, 0xdc, 0xa0, 0xfb,
    0x7d, 0xb4, 0x7d, 0x8b, 0xf0, 0xac, 0x98, 0x40, 0x74, 0x0c, 0x48, 0x74, 0x13, 0xb4, 0x0e, 0xbb, 0x07, 0x00, 0xcd,
    0x10, 0xeb, 0xef, 0xa0, 0xfd, 0x7d, 0xeb, 0xe6, 0xa0, 0xfc, 0x7d, 0xeb, 0xe1, 0xcd, 0x16, 0xcd, 0x19, 0x26, 0x8b,
    0x55, 0x1a, 0x52, 0xb0, 0x01, 0xbb, 0x00, 0x00, 0xe8, 0x3b, 0x00, 0x72, 0xe8, 0x5b, 0x8a, 0x56, 0x24, 0xbe, 0x0b,
    0x7c, 0x8b, 0xfc, 0xc7, 0x46, 0xf0, 0x3d, 0x7d, 0xc7, 0x46, 0xf4, 0x29, 0x7d, 0x8c, 0xd9, 0x89, 0x4e, 0xf2, 0x89,
    0x4e, 0xf6, 0xc6, 0x06, 0x96, 0x7d, 0xcb, 0xea, 0x03, 0x00, 0x00, 0x20, 0x0f, 0xb6, 0xc8, 0x66, 0x8b, 0x46, 0xf8,
    0x66, 0x03, 0x46, 0x1c, 0x66, 0x8b, 0xd0, 0x66, 0xc1, 0xea, 0x10, 0xeb, 0x5e, 0x0f, 0xb6, 0xc8, 0x4a, 0x4a, 0x8a,
    0x46, 0x0d, 0x32, 0xe4, 0xf7, 0xe2, 0x03, 0x46, 0xfc, 0x13, 0x56, 0xfe, 0xeb, 0x4a, 0x52, 0x50, 0x06, 0x53, 0x6a,
    0x01, 0x6a, 0x10, 0x91, 0x8b, 0x46, 0x18, 0x96, 0x92, 0x33, 0xd2, 0xf7, 0xf6, 0x91, 0xf7, 0xf6, 0x42, 0x87, 0xca,
    0xf7, 0x76, 0x1a, 0x8a, 0xf2, 0x8a, 0xe8, 0xc0, 0xcc, 0x02, 0x0a, 0xcc, 0xb8, 0x01, 0x02, 0x80, 0x7e, 0x02, 0x0e,
    0x75, 0x04, 0xb4, 0x42, 0x8b, 0xf4, 0x8a, 0x56, 0x24, 0xcd, 0x13, 0x61, 0x61, 0x72, 0x0b, 0x40, 0x75, 0x01, 0x42,
    0x03, 0x5e, 0x0b, 0x49, 0x75, 0x06, 0xf8, 0xc3, 0x41, 0xbb, 0x00, 0x00, 0x60, 0x66, 0x6a, 0x00, 0xeb, 0xb0, 0x4e,
    0x54, 0x4c, 0x44, 0x52, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0d, 0x0a, 0x52, 0x65, 0x6d, 0x6f, 0x76, 0x65, 0x20,
    0x64, 0x69, 0x73, 0x6b, 0x73, 0x20, 0x6f, 0x72, 0x20, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x20, 0x6d, 0x65, 0x64, 0x69,
    0x61, 0x2e, 0xff, 0x0d, 0x0a, 0x44, 0x69, 0x73, 0x6b, 0x20, 0x65, 0x72, 0x72, 0x6f, 0x72, 0xff, 0x0d, 0x0a, 0x50,
    0x72, 0x65, 0x73, 0x73, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x74, 0x6f, 0x20, 0x72, 0x65, 0x73,
    0x74, 0x61, 0x72, 0x74, 0x0d, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xac, 0xcb, 0xd8, 0x55, 0xaa, 0xf8,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t FAT12_root_dir[] = {
    0x4c, 0x50, 0x43, 0x55, 0x53, 0x42, 0x6c, 0x69, 0x62, 0x20, 0x20, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x45, 0x41, 0x44, 0x4d, 0x45,
    0x20, 0x20, 0x54, 0x58, 0x54, 0x21, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0xbb, 0x32, 0x00, 0x00, 0xdc, 0x83, 0xbb,
    0x32, 0x02, 0x00, 0x5d, 0x00, 0x00, 0x00, 0x41, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x65, 0x00, 0x78, 0x00, 0x0f,
    0x00, 0x2f, 0x2e, 0x00, 0x68, 0x00, 0x74, 0x00, 0x6d, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff,
    0xff, 0x49, 0x4e, 0x44, 0x45, 0x58, 0x20, 0x20, 0x20, 0x48, 0x54, 0x4d, 0x20, 0x00, 0x00, 0x0f, 0x93, 0x4a, 0x41,
    0x4a, 0x41, 0x00, 0x00, 0x0f, 0x93, 0x4a, 0x41, 0x05, 0x00, 0x68, 0x01, 0x00, 0x00,
};

static const uint8_t FAT12_file_readme_txt[] = {
    0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x55, 0x53, 0x42, 0x20, 0x4d, 0x65,
    0x6d, 0x6f, 0x72, 0x79, 0x20, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x20, 0x64, 0x65, 0x6d, 0x6f,
    0x6e, 0x73, 0x74, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t FAT12_file_index_htm[] = {
    0x3c, 0x21, 0x44, 0x4f, 0x43, 0x54, 0x59, 0x50, 0x45, 0x20, 0x68, 0x74, 0x6d, 0x6c, 0x3e, 0x0a, 0x3c, 0x68, 0x74,
    0x6d, 0x6c, 0x3e, 0x0a, 0x09, 0x3c, 0x68, 0x65, 0x61, 0x64, 0x3e, 0x0a, 0x09, 0x09, 0x3c, 0x73, 0x74, 0x79, 0x6c,
    0x65, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x63, 0x73, 0x73, 0x22, 0x3e, 0x0a,
    0x09, 0x09, 0x09, 0x68, 0x31, 0x20, 0x7b, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x72, 0x65, 0x64, 0x3b, 0x74, 0x65,
    0x78, 0x74, 0x2d, 0x61, 0x6c, 0x69, 0x67, 0x6e, 0x3a, 0x63, 0x65, 0x6e, 0x74, 0x65, 0x72, 0x3b, 0x7d, 0x0a, 0x09,
    0x09, 0x09, 0x68, 0x32, 0x20, 0x7b, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x62, 0x6c, 0x75, 0x65, 0x3b, 0x7d, 0x0a,
    0x09, 0x09, 0x09, 0x70, 0x20, 0x7b, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3a, 0x67, 0x72, 0x65, 0x65, 0x6e, 0x3b, 0x7d,
    0x0a, 0x09, 0x09, 0x09, 0x62, 0x6f, 0x64, 0x79, 0x20, 0x7b, 0x62, 0x61, 0x63, 0x6b, 0x67, 0x72, 0x6f, 0x75, 0x6e,
    0x64, 0x3a, 0x20, 0x52, 0x47, 0x42, 0x28, 0x32, 0x31, 0x35, 0x2c, 0x32, 0x33, 0x32, 0x2c, 0x32, 0x34, 0x34, 0x29,
    0x3b, 0x7d, 0x0a, 0x09, 0x09, 0x3c, 0x2f, 0x73, 0x74, 0x79, 0x6c, 0x65, 0x3e, 0x0a, 0x09, 0x3c, 0x2f, 0x68, 0x65,
    0x61, 0x64, 0x3e, 0x0a, 0x0a, 0x09, 0x3c, 0x62, 0x6f, 0x64, 0x79, 0x3e, 0x0a, 0x0a, 0x09, 0x09, 0x3c, 0x68, 0x31,
    0x3e, 0x4e, 0x58, 0x50, 0x20, 0x4c, 0x50, 0x43, 0x35, 0x34, 0x31, 0x31, 0x78, 0x20, 0x55, 0x53, 0x42, 0x20, 0x44,
    0x45, 0x4D, 0x4F, 0x3c, 0x2f, 0x68, 0x31, 0x3e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x09, 0x09, 0x3c,
    0x68, 0x72, 0x3e, 0x0a, 0x09, 0x09, 0x3c, 0x70, 0x3e, 0x46, 0x6f, 0x72, 0x20, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x69,
    0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x76, 0x69, 0x73, 0x69, 0x74, 0x3c, 0x61, 0x20,
    0x68, 0x72, 0x65, 0x66, 0x3d, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x6c, 0x70,
    0x63, 0x77, 0x61, 0x72, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x22, 0x3e, 0x20, 0x77, 0x77, 0x77, 0x2e, 0x6c, 0x70, 0x63,
    0x77, 0x61, 0x72, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x20, 0x3c, 0x2f, 0x61, 0x3e, 0x20, 0x3c, 0x2f, 0x70, 0x3e, 0x0a,
    0x0a, 0x09, 0x3c, 0x2f, 0x62, 0x6f, 0x64, 0x79, 0x3e, 0x0a, 0x3c, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x3e, 0x0a, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const struct FAT12_img
{
    const uint8_t *data;
    int offset;
    int size;
} FAT12_img[] = {
    /* Each sector is of size 512 Bytes i.e., 0x200 Bytes */
    {FAT12_header, 0, sizeof(FAT12_header)},                       /* FAT12 at sector 0,1 */
    {FAT12_root_dir, 0x400, sizeof(FAT12_root_dir)},               /* Root dir at sector 2 */
    {FAT12_file_readme_txt, 0x600, sizeof(FAT12_file_readme_txt)}, /* README.TXT file data */
    {FAT12_file_index_htm, 0xC00, sizeof(FAT12_file_index_htm)},   /* INDEX.HTM file data */
};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void DataRam_Initialize(void)
{
    int i;
    extern uint8_t g_memDiskArea[];
    memset(g_memDiskArea, 0, MSC_MEM_DISK_SIZE);
    for (i = 0; i < sizeof(FAT12_img) / sizeof(FAT12_img[0]); i++)
        memcpy(&g_memDiskArea[FAT12_img[i].offset], FAT12_img[i].data, FAT12_img[i].size);
}
