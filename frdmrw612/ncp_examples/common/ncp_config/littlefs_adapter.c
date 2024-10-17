/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include <stdio.h>
#include "littlefs_adapter.h"
#include <cli.h>
#include <cli_utils.h>
#include "ncp_config.h"

/*
WIFI LITTLE FS is close to OT LITTLE FS in the linker
LITTLEFS_REGION_SIZE = 0x200000 + 0x10000 + 0x10000 + 0x1000;
m_text_start         = 0x08001280;
m_text_size          = 0x03FFED80 - LITTLEFS_REGION_SIZE;
 */
#define LITTLEFS_START_ADDR   0x03DEF000

/* Minimum block read size definition */
#ifndef LITTLEFS_READ_SIZE
#define LITTLEFS_READ_SIZE 16
#endif

/* Minimum block program size definition */
#ifndef LITTLEFS_PROG_SIZE
#define LITTLEFS_PROG_SIZE 256
#endif

/* Erasable block size definition */
#ifndef LITTLEFS_BLOCK_SIZE
#define LITTLEFS_BLOCK_SIZE 4096
#endif

/* Erasable block count definition */
#ifndef LITTLEFS_BLOCK_COUNT
#define LITTLEFS_BLOCK_COUNT 16
#endif

/* Minimum block cache size definition */
#ifndef LITTLEFS_CACHE_SIZE
#define LITTLEFS_CACHE_SIZE 256
#endif

/* Minimum lookahead buffer size definition */
#ifndef LITTLEFS_LOOKAHEAD_SIZE
#define LITTLEFS_LOOKAHEAD_SIZE 16
#endif

struct lfs_mflash_ctx
{
    uint32_t start_addr;
};

static int lfs_mflash_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *, lfs_size_t);
static int lfs_mflash_prog(const struct lfs_config *, lfs_block_t, lfs_off_t, const void *, lfs_size_t);
static int lfs_mflash_erase(const struct lfs_config *, lfs_block_t);
static int lfs_mflash_sync(const struct lfs_config *);

#ifdef LFS_THREADSAFE
/* LittleFS lock/unlock callback*/
static int lfs_create_lock(void);
static int lfs_lock(const struct lfs_config *lfsc);
static int lfs_unlock(const struct lfs_config *lfsc);
#endif

/* LittleFS context */
static struct lfs_mflash_ctx LittleFS_ctx = {LITTLEFS_START_ADDR};

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const struct lfs_config LittleFS_config = {.context = (void *)&LittleFS_ctx,
                                           .read    = lfs_mflash_read,
                                           .prog    = lfs_mflash_prog,
                                           .erase   = lfs_mflash_erase,
                                           .sync    = lfs_mflash_sync,
#ifdef LFS_THREADSAFE
                                           .lock   = lfs_lock,
                                           .unlock = lfs_unlock,
#endif
                                           .read_size      = LITTLEFS_READ_SIZE,
                                           .prog_size      = LITTLEFS_PROG_SIZE,
                                           .block_size     = LITTLEFS_BLOCK_SIZE,
                                           .block_count    = LITTLEFS_BLOCK_COUNT,
                                           .block_cycles   = 100,
                                           .cache_size     = LITTLEFS_CACHE_SIZE,
                                           .lookahead_size = LITTLEFS_LOOKAHEAD_SIZE};

lfs_t lfs;
static struct lfs_config cfg;
int lfs_mounted;
#ifdef LFS_THREADSAFE
OSA_MUTEX_HANDLE_DEFINE(lfs_mutex);
#endif

extern const wifi_flash_table_type_t g_wifi_flash_table_system[];
extern const wifi_flash_table_type_t g_wifi_flash_table_provision[];
extern const wifi_flash_table_type_t g_wifi_flash_table_wlan_sta[];
extern const wifi_flash_table_type_t g_wifi_flash_table_wlan_uap[];

#define LFS_MAX_STR_LEN 256

/*******************************************************************************
 * Code
 ******************************************************************************/

static int lfs_mflash_read(const struct lfs_config *lfsc, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    struct lfs_mflash_ctx *ctx;
    uint32_t flash_addr;

    assert(lfsc);
    ctx = (struct lfs_mflash_ctx *)lfsc->context;
    assert(ctx);

    flash_addr = ctx->start_addr + block * lfsc->block_size + off;

    if (mflash_drv_read(flash_addr, buffer, size) != kStatus_Success)
        return LFS_ERR_IO;

    return LFS_ERR_OK;
}

static int lfs_mflash_prog(
    const struct lfs_config *lfsc, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    status_t status = kStatus_Success;
    struct lfs_mflash_ctx *ctx;
    uint32_t flash_addr;

    assert(lfsc);
    ctx = (struct lfs_mflash_ctx *)lfsc->context;
    assert(ctx);

    flash_addr = ctx->start_addr + block * lfsc->block_size + off;

    assert(mflash_drv_is_page_aligned(size));

    for (uint32_t page_ofs = 0; page_ofs < size; page_ofs += MFLASH_PAGE_SIZE)
    {
        status = mflash_drv_page_program(flash_addr + page_ofs, (void *)((uintptr_t)buffer + page_ofs));
        if (status != kStatus_Success)
            break;
    }

    if (status != kStatus_Success)
        return LFS_ERR_IO;

    return LFS_ERR_OK;
}

static int lfs_mflash_erase(const struct lfs_config *lfsc, lfs_block_t block)
{
    status_t status = kStatus_Success;
    struct lfs_mflash_ctx *ctx;
    uint32_t flash_addr;

    assert(lfsc);
    ctx = (struct lfs_mflash_ctx *)lfsc->context;
    assert(ctx);

    flash_addr = ctx->start_addr + block * lfsc->block_size;

    for (uint32_t sector_ofs = 0; sector_ofs < lfsc->block_size; sector_ofs += MFLASH_SECTOR_SIZE)
    {
        status = mflash_drv_sector_erase(flash_addr + sector_ofs);
        if (status != kStatus_Success)
            break;
    }

    if (status != kStatus_Success)
        return LFS_ERR_IO;

    return LFS_ERR_OK;
}

static int lfs_mflash_sync(const struct lfs_config *lfsc)
{
    return LFS_ERR_OK;
}

static int lfs_get_default_config(struct lfs_config *lfsc)
{
    *lfsc = LittleFS_config; /* copy pre-initialized lfs config structure */
    return 0;
}

static void lfs_format_handler(int argc, char **argv)
{
    int res;

    if (lfs_mounted)
    {
        PRINTF("LFS is mounted, please unmount it first.\r\n");
        return;
    }

    if (argc != 2 || strcmp(argv[1], "yes"))
    {
        PRINTF("Are you sure? Please issue command \"format yes\" to proceed.\r\n");
        return;
    }

    res = lfs_format(&lfs, &cfg);
    if (res)
    {
        PRINTF("\rError formatting LFS: %d\r\n", res);
    }
    else
    {
        PRINTF("\rformatted LFS: %d\r\n", res);
    }

    return;
}

static void lfs_mount_handler(int argc, char **argv)
{
    int res;

    if (lfs_mounted)
    {
        PRINTF("LFS already mounted\r\n");
        return;
    }

    res = lfs_mount(&lfs, &cfg);
    if (res)
    {
        PRINTF("\rError mounting LFS\r\n");
    }
    else
    {
        lfs_mounted = 1;
        PRINTF("LFS mounted\r\n");
    }

    return;
}

static void lfs_unmount_handler(int argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    res = lfs_unmount(&lfs);
    if (res)
    {
        PRINTF("\rError unmounting LFS: %i\r\n", res);
    }

    lfs_mounted = 0;
    return;
}

static void lfs_cd_handler(int argc, char **argv)
{
    PRINTF("There is no concept of current directory in this example.\r\nPlease always specify the full path.\r\n");
    return;
}

static void lfs_ls_handler(int argc, char **argv)
{
    int res;
    char *path;
    lfs_dir_t dir;
    struct lfs_info info;
    int files;
    int dirs;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    if (argc > 2)
    {
        PRINTF("Invalid number of parameters\r\n");
        return;
    }

    if (argc < 2)
    {
        path = "/";
    }
    else
    {
        path = argv[1];
    }

    /* open the directory */
    res = lfs_dir_open(&lfs, &dir, path);
    if (res)
    {
        PRINTF("\rError opening directory: %i\r\n", res);
        return;
    }

    PRINTF(" Directory of %s\r\n", path);
    files = 0;
    dirs  = 0;

    /* iterate until end of directory */
    while ((res = lfs_dir_read(&lfs, &dir, &info)) != 0)
    {
        if (res < 0)
        {
            /* break the loop in case of an error */
            PRINTF("\rError reading directory: %i\r\n", res);
            break;
        }

        if (info.type == LFS_TYPE_REG)
        {
            PRINTF("%8d %s\r\n", info.size, info.name);
            files++;
        }
        else if (info.type == LFS_TYPE_DIR)
        {
            PRINTF("%     DIR %s\r\n", info.name);
            dirs++;
        }
        else
        {
            PRINTF("%???\r\n");
        }
    }

    res = lfs_dir_close(&lfs, &dir);
    if (res)
    {
        PRINTF("\rError closing directory: %i\r\n", res);
        return;
    }

    PRINTF(" %d File(s), %d Dir(s)\r\n", files, dirs);

    return;
}

static void lfs_rm_handler(int argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    res = lfs_remove(&lfs, argv[1]);

    if (res)
    {
        PRINTF("\rError while removing: %i\r\n", res);
    }

    return;
}

static void lfs_mkdir_handler(int argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    res = lfs_mkdir(&lfs, argv[1]);

    if (res)
    {
        PRINTF("\rError creating directory: %i\r\n", res);
    }

    return;
}

static void lfs_write_handler(int argc, char **argv)
{
    int res;
    lfs_file_t file;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    res = lfs_file_open(&lfs, &file, argv[1], LFS_O_WRONLY | LFS_O_APPEND | LFS_O_CREAT);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return;
    }

    res = lfs_file_write(&lfs, &file, argv[2], strlen(argv[2]));
    if (res > 0)
        res = lfs_file_write(&lfs, &file, "\r\n", 2);

    if (res < 0)
    {
        PRINTF("\rError writing file: %i\r\n", res);
    }

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
    }

    return;
}

static void lfs_cat_handler(int argc, char **argv)
{
    int res;
    lfs_file_t file;
    uint8_t buf[16];
    int i;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    res = lfs_file_open(&lfs, &file, argv[1], LFS_O_RDONLY);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return;
    }

    do
    {
        res = lfs_file_read(&lfs, &file, buf, sizeof(buf));
        if (res < 0)
        {
            PRINTF("\rError reading file: %i\r\n", res);
            break;
        }
        for (i = 0; i < res; i++)
            PRINTF("%c", buf[i]);
        /*        SHELL_Write(s_shellHandle, (char *)buf, res); */
    } while (res);

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
    }

    return;
}

static void lfs_config_help(void)
{
    PRINTF("LFS config help\r\n");
    PRINTF("Example:\r\n");
    PRINTF("config sys/prov/wlan_sta/wlan_uap dump\r\n");
    PRINTF("config sys/prov/wlan_sta/wlan_uap get <idx>\r\n");
    PRINTF("config sys/prov/wlan_sta/wlan_uap set <idx> <decimal value>/<string>\r\n");
}

static void lfs_config_dump_wlan_sta(lfs_file_t *file)
{
    int i;
    char str[LFS_MAX_STR_LEN];

    PRINTF("Config dump wlan_sta:\r\n");
    for (i = 0; i < WLAN_STA_MAX_TYPE; i++)
    {
        memset(str, 0x00, LFS_MAX_STR_LEN);
        wifi_load_wlan_sta_config(file, i, str, g_wifi_flash_table_wlan_sta[i].len);
        PRINTF("%d: %s%s\r\n", i, g_wifi_flash_table_wlan_sta[i].name, str);
    }
}

static void lfs_config_dump_wlan_uap(lfs_file_t *file)
{
    int i;
    char str[LFS_MAX_STR_LEN];

    PRINTF("Config dump wlan_uap:\r\n");
    for (i = 0; i < WLAN_UAP_MAX_TYPE; i++)
    {
        memset(str, 0x00, LFS_MAX_STR_LEN);
        wifi_load_wlan_uap_config(file, i, str, g_wifi_flash_table_wlan_uap[i].len);
        PRINTF("%d: %s%s\r\n", i, g_wifi_flash_table_wlan_uap[i].name, str);
    }
}

static void lfs_config_dump_prov(lfs_file_t *file)
{
    int i;
    char str[LFS_MAX_STR_LEN];

    PRINTF("Config dump provision:\r\n");
    for (i = 0; i < PROV_MAX_TYPE; i++)
    {
        memset(str, 0x00, LFS_MAX_STR_LEN);
        wifi_load_provision_config(file, i, str, g_wifi_flash_table_provision[i].len);
        PRINTF("%d: %s%s\r\n", i, g_wifi_flash_table_provision[i].name, str);
    }
}

static void lfs_config_dump_sys(lfs_file_t *file)
{
    int i;
    char str[LFS_MAX_STR_LEN];

    PRINTF("Config dump sys:\r\n");
    for (i = 0; i < SYS_MAX_TYPE; i++)
    {
        memset(str, 0x00, LFS_MAX_STR_LEN);
        wifi_load_system_config(file, i, str, g_wifi_flash_table_system[i].len);
        PRINTF("%d: %s%s\r\n", i, g_wifi_flash_table_system[i].name, str);
    }
}

static void lfs_config_do(lfs_file_t *file, int action, int type, int idx, void *buf)
{
    char str[LFS_MAX_STR_LEN] = {0};
    int str_len;

    if (action == 1)
    {
        if (type == WIFI_CONFIG_SYS)
        {
            str_len = strlen((char *)buf);
            memcpy(str, buf, str_len);
            wifi_save_system_config(file, idx, str, str_len + 1);
        }
        else if (type == WIFI_CONFIG_PROV)
        {
            str_len = strlen((char *)buf);
            memcpy(str, buf, str_len);
            wifi_save_provision_config(file, idx, str, str_len + 1);
        }
        else if (type == WIFI_CONFIG_WLAN_STA)
        {
            str_len = strlen((char *)buf);
            memcpy(str, buf, str_len);
            wifi_save_wlan_sta_config(file, idx, str, str_len + 1);
            if (idx == WLAN_MAC || idx == WLAN_BSSID)
            {
                /* mac address buffer restored */
                PRINTF("wlan mac config is read only, please use wlan-mac command\r\n");
            }
        }
        else if (type == WIFI_CONFIG_WLAN_UAP)
        {
            str_len = strlen((char *)buf);
            memcpy(str, buf, str_len);
            wifi_save_wlan_uap_config(file, idx, str, str_len + 1);
            if (idx == WLAN_MAC || idx == WLAN_BSSID)
            {
                /* mac address buffer restored */
                PRINTF("wlan mac config is read only, please use wlan-mac command\r\n");
            }
        }
    }
    else
    {
        if (type == WIFI_CONFIG_SYS)
        {
            wifi_load_system_config(file, idx, str, g_wifi_flash_table_system[idx].len);
            PRINTF("Config type[%d] index[%d]:\r\n", type, idx);
            PRINTF("%s%s\r\n", g_wifi_flash_table_system[idx].name, str);
        }
        else if (type == WIFI_CONFIG_PROV)
        {
            wifi_load_provision_config(file, idx, str, g_wifi_flash_table_provision[idx].len);
            PRINTF("Config type[%d] index[%d]:\r\n", type, idx);
            PRINTF("%s%s\r\n", g_wifi_flash_table_provision[idx].name, str);
        }
        else if (type == WIFI_CONFIG_WLAN_STA)
        {
            wifi_load_wlan_sta_config(file, idx, str, g_wifi_flash_table_wlan_sta[idx].len);
            PRINTF("Config type[%d] index[%d]:\r\n", type, idx);
            PRINTF("%s%s\r\n", g_wifi_flash_table_wlan_sta[idx].name, str);
        }
        else if (type == WIFI_CONFIG_WLAN_UAP)
        {
            wifi_load_wlan_uap_config(file, idx, str, g_wifi_flash_table_wlan_uap[idx].len);
            PRINTF("Config type[%d] index[%d]:\r\n", type, idx);
            PRINTF("%s%s\r\n", g_wifi_flash_table_wlan_uap[idx].name, str);
        }
    }
}

static int lfs_config_bss_handler(lfs_file_t *file, char *profile_name, int *type)
{
    int res;
    char file_path[FULL_PATH_NAME_SIZE];
    char role[2];

    snprintf(file_path, FULL_PATH_NAME_SIZE, "/etc/%s_conf", profile_name);

    res  = lfs_file_open(&lfs, file, file_path, LFS_O_RDWR);
    if (res != 0)
    {
        PRINTF("open file %s fail res %d\r\n", file_path, res);
        return LFS_ERR_INVAL;
    }
    res = wifi_load_wlan_uap_config(file, WLAN_ROLE, role, sizeof(role));
    if(res != 0)
    {
        PRINTF("open file %s, get bss role fail, res %d\r\n", file_path, res);
        return LFS_ERR_INVAL;
    }
    enum wlan_bss_role bss_role = (enum wlan_bss_role)atoi(role);
    switch (bss_role)
    {
        case WLAN_BSS_ROLE_STA:
            *type = WIFI_CONFIG_WLAN_STA;
            break;
        case WLAN_BSS_ROLE_UAP:
            *type = WIFI_CONFIG_WLAN_UAP;
            break;
        default:
            break;
    }

    return LFS_ERR_OK;
}

static void lfs_config_handler(int argc, char **argv)
{
    int type = WIFI_CONFIG_WLAN_STA;
    int set;
    int idx;
    int res;
    lfs_file_t file;
    char bss_name[] = "wlan_bss";

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return;
    }

    if (argc < 3 || argc > 5)
    {
        PRINTF("LFS invalid argument number\r\n");
        lfs_config_help();
        return;
    }

    if (!strcmp(argv[1], "sys"))
    {
        type = WIFI_CONFIG_SYS;
        res  = lfs_file_open(&lfs, &file, SYS_CONFIG_FILE_PATH, LFS_O_RDWR);
        if (res != 0)
        {
            PRINTF("open file %s fail res %d\r\n", SYS_CONFIG_FILE_PATH, res);
            return;
        }
    }
    else if (!strcmp(argv[1], "prov"))
    {
        type = WIFI_CONFIG_PROV;
        res  = lfs_file_open(&lfs, &file, PROV_CONFIG_FILE_PATH, LFS_O_RDWR);
        if (res != 0)
        {
            PRINTF("open file %s fail res %d\r\n", PROV_CONFIG_FILE_PATH, res);
            return;
        }
    }
    else if (!strncmp(argv[1], bss_name, strlen(bss_name)))
    {
        res = lfs_config_bss_handler(&file, argv[1], &type);
        if(res != LFS_ERR_OK)
        {
            PRINTF("Pls enter correct bss profile name: wlan_bssA, wlan_bssB, wlan_bssC, wlan_bssD, wlan_bssE\r\n");
            goto done;
        }
    }
    else
    {
        PRINTF("LFS unknown config type\r\n");
        lfs_config_help();
        return;
    }

    if (!strcmp(argv[2], "get"))
    {
        set = 0;
    }
    else if (!strcmp(argv[2], "set"))
    {
        set = 1;
    }
    else if (!strcmp(argv[2], "dump"))
    {
        lfs_config_help();

        if (type == WIFI_CONFIG_SYS)
            lfs_config_dump_sys(&file);
        else if (type == WIFI_CONFIG_PROV)
            lfs_config_dump_prov(&file);
        else if (type == WIFI_CONFIG_WLAN_STA)
            lfs_config_dump_wlan_sta(&file);
        else if (type == WIFI_CONFIG_WLAN_UAP)
            lfs_config_dump_wlan_uap(&file);

        goto done;
    }
    else
    {
        PRINTF("LFS unknown config action\r\n");
        lfs_config_help();
        goto done;
    }

    if (argv[3] == NULL)
    {
        PRINTF("LFS unknown config index\r\n");
        goto done;
    }

    idx = atoi(argv[3]);

    /* get action will not use argv[4], do not need to check NULL */
    lfs_config_do(&file, set, type, idx, argv[4]);

done:
    res = lfs_file_close(&lfs, &file);
    if (res != 0)
        PRINTF("close file fail res %d\r\n", res);
}

int lfs_save_file(char *path, unsigned int flag, int offset, unsigned char *buf, int len)
{
    int res = -1;
    int wr_len;
    lfs_file_t file;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return res;
    }

    res = lfs_file_open(&lfs, &file, path, flag);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return res;
    }

    if (offset != 0)
    {
        res = lfs_file_seek(&lfs, &file, offset, LFS_SEEK_SET);
        if (res != offset)
        {
            PRINTF("\rlfs_file_seek offset %d, res %d\r\n", offset, res);
            return -1;
        }
    }

    wr_len = lfs_file_write(&lfs, &file, buf, len);
    if (wr_len != len)
    {
        PRINTF("\rError writing file: %i\r\n", wr_len);
        return -1;
    }

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
        return res;
    }
    return wr_len;
}

int lfs_load_file(char *path, unsigned int flag, int offset, unsigned char *buf, int len)
{
    int res = -1;
    int rd_len;
    lfs_file_t file;

    if (!lfs_mounted)
    {
        PRINTF("LFS not mounted\r\n");
        return res;
    }

    res = lfs_file_open(&lfs, &file, path, flag);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return res;
    }

    if (offset != 0)
    {
        res = lfs_file_seek(&lfs, &file, offset, LFS_SEEK_SET);
        if (res != offset)
        {
            PRINTF("\rlfs_file_seek offset %d, res %d\r\n", offset, res);
            return res;
        }
    }

    rd_len = lfs_file_read(&lfs, &file, buf, len);
    if (rd_len < 0)
    {
        PRINTF("\rError reading file len %d, read len %d\r\n", len, rd_len);
        return rd_len;
    }

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
        return res;
    }
    return rd_len;
}

static struct cli_command littlefs_cli[] = {
    {"format", NULL, lfs_format_handler}, {"mount", NULL, lfs_mount_handler}, {"unmount", NULL, lfs_unmount_handler},
    {"cd", NULL, lfs_cd_handler},         {"ls", NULL, lfs_ls_handler},       {"rm", NULL, lfs_rm_handler},
    {"mkdir", NULL, lfs_mkdir_handler},   {"write", NULL, lfs_write_handler}, {"cat", NULL, lfs_cat_handler},
    {"config", NULL, lfs_config_handler}};

static int littlefs_cli_init(void)
{
    if (cli_register_commands(littlefs_cli, (int)(sizeof(littlefs_cli) / sizeof(struct cli_command))) != 0)
    {
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

#ifdef LFS_THREADSAFE
/* TODO use freeRTOS port */
int lfs_create_lock(void)
{
    return OSA_MutexCreate((osa_mutex_handle_t)lfs_mutex);
}

int lfs_lock(const struct lfs_config *lfsc)
{
    return OSA_MutexLock((osa_mutex_handle_t)lfs_mutex, osaWaitForever_c);
}

int lfs_unlock(const struct lfs_config *lfsc)
{
    return OSA_MutexUnlock((osa_mutex_handle_t)lfs_mutex);
}
#endif

int littlefs_init(void)
{
    int ret;

    lfs_get_default_config(&cfg);

    ret = littlefs_cli_init();
    if (ret != WM_SUCCESS)
    {
        PRINTF("littlefs_cli_init fail\r\n");
        return -WM_FAIL;
    }

    /* flash driver init */
    ret = mflash_drv_init();
    if (ret != 0)
    {
        PRINTF("mflash_drv_init fail\r\n");
        return -WM_FAIL;
    }

#ifdef LFS_THREADSAFE
    /* create littlefs mutex */
    ret = lfs_create_lock();
    if (ret != WM_SUCCESS)
    {
        PRINTF("create littlefs mutex fail\r\n");
        return -WM_FAIL;
    }
#endif

    /* mount, if fail, try format flash and mount again */
    ret = lfs_mount(&lfs, &cfg);
    if (ret)
    {
        PRINTF("\rMounting LFS fail, try to format flash and mount again\r\n");

        ret = lfs_format(&lfs, &cfg);
        if (ret)
        {
            PRINTF("\rError formatting LFS: %d\r\n", ret);
            return -WM_FAIL;
        }
        else
        {
            PRINTF("\rformatted LFS: %d\r\n", ret);

            ret = lfs_mount(&lfs, &cfg);
            if (ret)
            {
                PRINTF("\rError mounting LFS: %d\r\n", ret);
                return -WM_FAIL;
            }
            else
            {
                lfs_mounted = 1;
                PRINTF("LFS formatted and mounted\r\n");
            }
        }
    }
    else
    {
        lfs_mounted = 1;
        PRINTF("LFS mounted\r\n");
    }

    return WM_SUCCESS;
}
