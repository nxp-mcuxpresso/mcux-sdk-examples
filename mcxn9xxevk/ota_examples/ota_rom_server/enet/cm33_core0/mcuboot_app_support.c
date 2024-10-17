/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include "fsl_debug_console.h"
#include "mcuboot_app_support.h"
#include "mflash_drv.h"
#include "sblconfig.h"
#include "sysflash/sysflash.h"
#include "flash_map.h"

#define PRIMARY_SLOT_ACTIVE   0
#define SECONDARY_SLOT_ACTIVE 1


#ifdef MCUBOOT_BOOT_MAX_ALIGN

_Static_assert(MCUBOOT_BOOT_MAX_ALIGN >= 8 && MCUBOOT_BOOT_MAX_ALIGN <= 32,
               "Unsupported value for MCUBOOT_BOOT_MAX_ALIGN");

#define BOOT_MAX_ALIGN          MCUBOOT_BOOT_MAX_ALIGN
#define BOOT_MAGIC_ALIGN_SIZE   ALIGN_UP(BOOT_MAGIC_SZ, BOOT_MAX_ALIGN)
#else
#define BOOT_MAX_ALIGN          8
#define BOOT_MAGIC_ALIGN_SIZE   BOOT_MAGIC_SZ
#endif

#define BOOT_MAGIC_SZ  16

#define BOOT_FLAG_SET  1

struct image_trailer {
    uint8_t swap_type;
    uint8_t pad1[BOOT_MAX_ALIGN - 1];
    uint8_t copy_done;
    uint8_t pad2[BOOT_MAX_ALIGN - 1];
    uint8_t image_ok;
    uint8_t pad3[BOOT_MAX_ALIGN - 1];
#if BOOT_MAX_ALIGN > BOOT_MAGIC_SZ
    uint8_t pad4[BOOT_MAGIC_ALIGN_SIZE - BOOT_MAGIC_SZ];
#endif
    uint8_t magic[BOOT_MAGIC_SZ];
};

union boot_img_magic_t
{
    struct {
        uint16_t align;
        uint8_t magic[14];
    };
    uint8_t val[16];
};

#if BOOT_MAX_ALIGN == 8
const union boot_img_magic_t boot_img_magic = {
    .val = {
        0x77, 0xc2, 0x95, 0xf3,
        0x60, 0xd2, 0xef, 0x7f,
        0x35, 0x52, 0x50, 0x0f,
        0x2c, 0xb6, 0x79, 0x80
    }
};
#else
const union boot_img_magic_t boot_img_magic = {
    .align = BOOT_MAX_ALIGN,
    .magic = {
        0x2d, 0xe1,
        0x5d, 0x29, 0x41, 0x0b,
        0x8d, 0x77, 0x67, 0x9c,
        0x11, 0x0f, 0x1f, 0x8a
    }
};
#endif

#define BOOT_IMG_MAGIC  (boot_img_magic.val)

#if BOOT_MAX_ALIGN == 8
#define BOOT_IMG_ALIGN  (BOOT_MAX_ALIGN)
#else
#define BOOT_IMG_ALIGN  (boot_img_magic.align)
#endif

#if defined(CONFIG_ENCRYPT_XIP_EXT_ENABLE)

#define ENC_MAGIC_SZ    16

union enc_magic_t
{
    uint8_t val[ENC_MAGIC_SZ];
};

const union enc_magic_t enc_magic = {
    .val = {
        0xAA, 0xBB, 0xCC, 0xDD,
        0x60, 0x4D, 0xBA, 0x70,
        0x34, 0x79, 0x2c, 0x0f,
        0x2c, 0xb6, 0x0f, 0x35
    }
};

#define ENC_MAGIC  (enc_magic.val)

/** Structure holds partial metadata used for active slot
 */
typedef struct
{
    /* ...platform specific...*/
    uint32_t active_slot;                       // 0 - primary, 1 - secondary
    uint8_t pad_0[12];                          // Padding zeroes
    uint8_t hash[16];                           // Hash of encrypted key blocks including padding zeroes
    uint8_t pad_1[16];                          // Padding zeroes
    uint8_t magic[ENC_MAGIC_SZ];                // Magic number
} enc_metadata_t;

/* Returns slot number linked to execution slot */
static uint32_t read_enc_metadata(void)
{
  enc_metadata_t metadata;
  uint32_t off = boot_flash_meta_map[0].fa_off + boot_flash_meta_map[0].fa_size 
                  - sizeof(enc_metadata_t);
  
  memset(&metadata, 0, sizeof(enc_metadata_t));
  if(bl_flash_read(off, (uint32_t *)&metadata, sizeof(enc_metadata_t)) != 0)
     goto error;
  if(memcmp(&metadata.magic, ENC_MAGIC, ENC_MAGIC_SZ) == 0)
  {
    if(metadata.active_slot == 0)
      return PRIMARY_SLOT_ACTIVE;
    else
      return SECONDARY_SLOT_ACTIVE;
  }
error:
  /* Valid metadata should be always present in production phase */
  /* Always return a slot number */
  PRINTF("WARNING: invalid metadata of active slot - debug session?\n");
  PRINTF("WARNING: OTA image will be downloaded to secondary slot\n");
  return PRIMARY_SLOT_ACTIVE;
}
#endif /* CONFIG_ENCRYPT_XIP_EXT_ENABLE */


/** Find out what slot is currently booted.
 *
 * @retval PRIMARY_SLOT_ACTIVE: image is running from primary slot
 *         SECONDARY_SLOT_ACTIVE: image is running from secondary slot
 *
 * !!! TODO !!!
 * There is currently a problem with DIRECT_XIP (REMAP) support and multiple images.
 * Normally when using SWAP modes all images run from PRIMARY slot.
 * When using DIRECT_XIP this is no longer valid. In case of DIRECT_XIP and multiple
 * images this would need to be redesigned to be able to get active slot for
 * different core than the one calling this routine (for example APP calling this for WIFI image).
 * In flash_partitioning.c another array of getter functions per slot could be used for this.
 *
 *
 */

#if defined(MCUBOOT_DIRECT_XIP) && CONFIG_UPDATEABLE_IMAGE_NUMBER > 1
#error "DIRECT_XIP (using remapping) and multiple images is not currently supported"
#endif

static uint32_t get_active_image(uint32_t image)
{
    (void)image;

#if defined(CONFIG_MCUBOOT_FLASH_REMAP_ENABLE) && !defined(CONFIG_MCUBOOT_FLASH_REMAP_BY_SWAP)

    /* Using flash remapping by overlay done by FlexSPI IP */
    if (bl_flash_remap_active())
        return SECONDARY_SLOT_ACTIVE;
    else
        return PRIMARY_SLOT_ACTIVE;
    
#elif defined (CONFIG_ENCRYPT_XIP_EXT_ENABLE)
    return read_enc_metadata();
#else

    /* In other configurations active slot is the PRIMARY one*/
    return PRIMARY_SLOT_ACTIVE;
#endif
}


/** This wrapper function deals with mflash driver limitations such as unaligned
 *  access to memory, copying data into unaligned destination and data size is
 *  not multiple of 4.
 *
 * @retval kStatus_Success: all OK
 *         otherwise something failed
 */
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
static int32_t mflash_drv_read_wrapper(uint32_t addr, void *dst, uint32_t len)
{
    /* temporary buffer size has to be multiple of 4! */
    static uint32_t tmp_buf[64 / sizeof(uint32_t)];
    status_t status;
    uint32_t chunkAlign_cnt = 0;
    uint32_t offset         = addr;
    uint8_t *dst_ptr        = (uint8_t *)dst;

    /* Correction for unaligned start address offset*/
    if (len > 0 && (addr % 4 != 0))
    {
        uint32_t cor         = addr % 4;
        uint32_t restTo4     = 4 - cor;
        uint32_t segment_len = (len >= 4 ? restTo4 : len);

        /* read whole 4B and copy desired segment to destination*/
        status = mflash_drv_read(offset - cor, tmp_buf, 4);
        if (status != kStatus_Success)
            return status;
        memcpy(dst_ptr, (uint8_t *)tmp_buf + cor, segment_len);

        /* correct offset address, destination pointer and remaining data size */
        offset += restTo4;
        dst_ptr += restTo4;
        if (restTo4 > len)
        {
            len = 0;
        }
        else
        {
            len -= restTo4;
        }
    }

    while (len > 0)
    {
        size_t chunk = (len > sizeof(tmp_buf)) ? sizeof(tmp_buf) : len;
        /* mflash demands size to be in multiples of 4 */
        size_t chunkAlign4 = (chunk + 3) & (~3);

        /* directly copy into destination if address and data size is aligned */
        if ((uint32_t)dst_ptr % 4 == 0 && chunk % 4 == 0)
        {
            status = mflash_drv_read(offset, (uint32_t *)(dst_ptr + chunkAlign_cnt * sizeof(tmp_buf)), chunkAlign4);
        }
        else
        {
            status = mflash_drv_read(offset, tmp_buf, chunkAlign4);
            memcpy(dst_ptr + chunkAlign_cnt * sizeof(tmp_buf), (uint8_t *)tmp_buf, chunk);
        }

        if (status != kStatus_Success)
        {
            return status;
        }

        len -= chunk;
        offset += chunk;
        chunkAlign_cnt++;
    }

    return kStatus_Success;
}
#endif /* CONFIG_MCUBOOT_FLASH_REMAP_ENABLE */

static int check_unset(uint8_t *p, int len)
{
    while (len > 0)
    {
        if (*p != 0xff)
        {
            return 0;
        }
        p++;
        len--;
    }
    return 1;
}

static int boot_img_magic_check(const uint8_t *magic)
{
    if (memcmp(magic, BOOT_IMG_MAGIC, BOOT_MAGIC_SZ) == 0) {
        return 1;
    }
    return 0;
}

static status_t boot_swap_test(uint32_t image)
{
    uint32_t off;
    status_t status;
    int faid;
    struct flash_area *fa;

    uint32_t buf[MFLASH_PAGE_SIZE / 4]; /* ensure the buffer is word aligned */
    struct image_trailer *image_trailer_p =
        (struct image_trailer *)((uint8_t *)buf + MFLASH_PAGE_SIZE - sizeof(struct image_trailer));

    if (image >= MCUBOOT_IMAGE_NUMBER)
    {
        return kStatus_InvalidArgument;
    }

    if (get_active_image(image) == PRIMARY_SLOT_ACTIVE)
    {
        faid = FLASH_AREA_IMAGE_SECONDARY(image);
    }
    else
    {
        faid = FLASH_AREA_IMAGE_PRIMARY(image);
    }

    fa  = &boot_flash_map[faid];
    off = fa->fa_off + fa->fa_size;

    memset(buf, 0xff, MFLASH_PAGE_SIZE);
    memcpy(image_trailer_p->magic, BOOT_IMG_MAGIC, BOOT_MAGIC_SZ);

    PRINTF("write magic number offset = 0x%x\r\n", off - MFLASH_PAGE_SIZE);

    status = mflash_drv_sector_erase(off - MFLASH_SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to erase trailer2\r\n", __func__);
        return status;
    }

    status = mflash_drv_page_program(off - MFLASH_PAGE_SIZE, buf);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to write trailer2\r\n", __func__);
        return status;
    }
  
    return status;
}

static status_t boot_swap_ok(int image)
{
    uint32_t off;
    status_t status;
    int faid;
    struct flash_area *fa;

    uint32_t buf[MFLASH_PAGE_SIZE / 4]; /* ensure the buffer is word aligned */
    struct image_trailer *image_trailer_p =
        (struct image_trailer *)((uint8_t *)buf + MFLASH_PAGE_SIZE - sizeof(struct image_trailer));

    if (image < 0 || image >= MCUBOOT_IMAGE_NUMBER)
    {
        return kStatus_InvalidArgument;
    }

    if (get_active_image(image) == PRIMARY_SLOT_ACTIVE)
    {
        faid = FLASH_AREA_IMAGE_PRIMARY(image);
    }
    else
    {
        faid = FLASH_AREA_IMAGE_SECONDARY(image);
    }

    fa          = &boot_flash_map[faid];
    off = fa->fa_off + fa->fa_size;

    status = bl_flash_read(off - MFLASH_PAGE_SIZE, buf, MFLASH_PAGE_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to read trailer\r\n", __func__);
        return status;
    }

    if ((boot_img_magic_check(image_trailer_p->magic) == 0) || (image_trailer_p->copy_done != 0x01))
    {
        /* the image in the slot is likely incomplete (or none) */
        PRINTF("%s: there is no image awaiting confirmation\r\n", __func__);
        status = kStatus_NoData;
        return status;
    }

    if (image_trailer_p->image_ok == BOOT_FLAG_SET)
    {
        /* nothing to be done, report it and return */
        PRINTF("%s: image already confirmed\r\n", __func__);
        return status;
    }

    /* mark image ok */
    image_trailer_p->image_ok = BOOT_FLAG_SET;

    /* erase trailer */
    status = mflash_drv_sector_erase(off - MFLASH_SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to erase trailer1\r\n, __func__");
        return status;
    }

    /* write trailer */
    status = mflash_drv_page_program(off - MFLASH_PAGE_SIZE, buf);
    if (status != kStatus_Success)
    {
        PRINTF("%s: failed to write trailer1\r\n, __func__");
        return status;
    }

    return status;
}


/* Does a sanity check of image based on patterns and sizes.
   It doesn't verify the code signing! */

int32_t bl_verify_image(uint32_t addrphy, uint32_t size)
{
    struct image_header *ih;
    struct image_tlv_info *it;
    uint32_t decl_size;
    uint32_t offset = addrphy;

    /* Secure that buffer size is 4B aligned */
    uint32_t buffer[(sizeof(struct image_header) / sizeof(uint32_t) + 3) & (~3)];

    if (bl_flash_read(offset, (uint32_t *)buffer, sizeof(struct image_header)) != kStatus_Success)
    {
        PRINTF("Flash read failed\n");
        return 0;
    }

    ih = (struct image_header *)buffer;

    /* do we have at least the header */
    if (size < sizeof(struct image_header))
    {
        return 0;
    }

    /* check magic number */
    if (ih->ih_magic != IMAGE_MAGIC)
    {
        return 0;
    }

    /* check that we have at least the amount of data declared by the header */
    decl_size = ih->ih_img_size + ih->ih_hdr_size + ih->ih_protect_tlv_size;
    if (size < decl_size)
    {
        return 0;
    }

    /* check protected TLVs if any */
    if (ih->ih_protect_tlv_size > 0)
    {
        if (ih->ih_protect_tlv_size < sizeof(struct image_tlv_info))
        {
            return 0;
        }
        if (bl_flash_read(offset + ih->ih_img_size + ih->ih_hdr_size, (uint32_t *)buffer, sizeof(struct image_tlv_info)) !=
            kStatus_Success)
        {
            PRINTF("Flash read failed\n");
            return 0;
        }
        it = (struct image_tlv_info *)buffer;
        if ((it->it_magic != IMAGE_TLV_PROT_INFO_MAGIC) || (it->it_tlv_tot != ih->ih_protect_tlv_size))
        {
            PRINTF("Failed to verify protected TLV\n");
            return 0;
        }
    }

    /* check for optional TLVs following the image as declared by the header */
    /* the struct is always present following firmware image or protected TLV*/
    /* if protected area is present */
    if (bl_flash_read(offset + decl_size, (uint32_t *)buffer, sizeof(struct image_tlv_info)) != kStatus_Success)
    {
        PRINTF("Flash read failed\n");
        return 0;
    }
    it = (struct image_tlv_info *)buffer;
    if ((it->it_magic != IMAGE_TLV_INFO_MAGIC))
    {
        PRINTF("Failed to verify optional TLV\n");
        return 0;
    }

    return 1;
}

/** Find out the destination slot (partition) for storage OTA image
 *
 * @param ptn partition_t struct for storing destination OTA image
 *
 * @retval kStatus_Success: ptn content is valid
 *         kStatus_Fail: ptn content is invalid
 *
 * @note Passed address is physical
 */
status_t bl_get_update_partition_info(uint32_t image, partition_t *ptn)
{
    uint32_t state;
    status_t ret;
    int faid;

    if (image >= MCUBOOT_IMAGE_NUMBER)
    {
        return kStatus_InvalidArgument;
    }

    memset(ptn, 0x0, sizeof(*ptn));

    ret = bl_get_image_state(image, &state);
    if (ret != kStatus_Success)
        goto error;
    if (state == kSwapType_Testing)
    {
        PRINTF("Test state detected, cannot determine update parition\n");
        goto error;
    }

    if (get_active_image(image) == PRIMARY_SLOT_ACTIVE)
    {
        faid = FLASH_AREA_IMAGE_SECONDARY(image);
    }
    else
    {
        faid = FLASH_AREA_IMAGE_PRIMARY(image);
    }

    ptn->start = boot_flash_map[faid].fa_off;
    ptn->size  = boot_flash_map[faid].fa_size;

    return kStatus_Success;
error:
    return kStatus_Fail;
}

const char *bl_imgstate_to_str(uint32_t state)
{
    switch (state)
    {
        case kSwapType_None:
            return "None";
        case kSwapType_ReadyForTest:
            return "ReadyForTest";
        case kSwapType_Testing:
            return "Testing";
        case kSwapType_Permanent:
            return "Permanent";
        default:
            return "UNEXPECTED";
    }
}

status_t bl_update_image_state(uint32_t image, uint32_t state)
{
    status_t status;

    switch (state)
    {
        case kSwapType_ReadyForTest:
            status = boot_swap_test(image);
            break;

        case kSwapType_Permanent:
            status = boot_swap_ok(image);
            break;

        default:
            status = kStatus_InvalidArgument;
            break;
    }

    return status;
}

status_t bl_get_image_state(uint32_t image, uint32_t *state)
{
    status_t status;
    uint32_t off;
    int faid;
    struct flash_area *fa;

    struct image_trailer image_trailer1;
    struct image_trailer image_trailer2;

    struct image_trailer *image_trailer_active;
    struct image_trailer *image_trailer_future;

    if (image >= MCUBOOT_IMAGE_NUMBER)
    {
        return kStatus_InvalidArgument;
    }

    /* Primary Slot Trailer */
    faid = FLASH_AREA_IMAGE_PRIMARY(image);
    fa   = &boot_flash_map[faid];

    off    = fa->fa_off + fa->fa_size - sizeof(struct image_trailer);
    status = bl_flash_read(off, (uint32_t *)&image_trailer1, sizeof(struct image_trailer));
    if (status)
    {
        PRINTF("%s: failed to read trailer in primary slot\r\n", __func__);
        return status;
    }

    /* Secondary Slot Trailer */
    faid = FLASH_AREA_IMAGE_SECONDARY(image);
    fa   = &boot_flash_map[faid];

    off    = fa->fa_off + fa->fa_size - sizeof(struct image_trailer);
    status = bl_flash_read(off, (uint32_t *)&image_trailer2, sizeof(struct image_trailer));
    if (status)
    {
        PRINTF("%s: failed to read trailer in secondary slot\r\n", __func__);
        return status;
    }

    if (get_active_image(image) == PRIMARY_SLOT_ACTIVE)
    {
        image_trailer_active = &image_trailer1;
        image_trailer_future = &image_trailer2;
    }
    else
    {
        image_trailer_active = &image_trailer2;
        image_trailer_future = &image_trailer1;
    }

    if (boot_img_magic_check(image_trailer_active->magic) && (image_trailer_active->image_ok == 0xff) &&
        (image_trailer_active->copy_done == 0x01))
    {
        /* State III (revert scheduled for next reboot => image is under test) */
        *state = kSwapType_Testing;
        return kStatus_Success;
    }

    if (boot_img_magic_check(image_trailer_future->magic))
    {
        if (check_unset(&image_trailer_future->image_ok, sizeof(image_trailer_future->image_ok)))
        {
            /* State I (request for swaping upon next reboot) */
            *state = kSwapType_ReadyForTest;
            return kStatus_Success;
        }
        else if (image_trailer_future->image_ok == 0x01)
        {
            /* State II (image marked for permanent change) */
            *state = kSwapType_Permanent;
            return kStatus_Success;
        }
    }

    /* State IV (none of the above) */
    *state = kSwapType_None;
    return kStatus_Success;
}


int bl_flash_remap_active(void)
{
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
    return (*((volatile uint32_t *)FLASH_REMAP_OFFSET_REG) > 0) ? 1 : 0;
#else
    return 0;
#endif
}


static void print_hash(const void *src, size_t size)
{
    const unsigned char *src8 = src;
    for (size_t i = 0; i < size; i++)
    {
        PRINTF("%02X", src8[i]);
    }
}

void bl_print_image_info(bl_hashfunc_t hashfunc)
{
    for (int image = 0; image < MCUBOOT_IMAGE_NUMBER; image++)
    {
        status_t ret;
        uint32_t imgstate = kSwapType_Fail;
        const char *name = boot_image_names[image];
        int remap_active = bl_flash_remap_active();
        uint32_t slot_offset_phys[2];
        
        /* to be able to reorder in case of active remap to show physical layout */
        slot_offset_phys[0] = boot_flash_map[image*2].fa_off;
        slot_offset_phys[1] = boot_flash_map[image*2+1].fa_off;

        ret = bl_get_image_state(image, &imgstate);
        if (ret != kStatus_Success)
        {
            PRINTF("Failed to get state of image %u (ret %d)", image, ret);
        }
        
        if (remap_active)
        {
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_BY_SWAP
            const char *remap_type = "REMAP_SWAP";
            uint32_t offset;
            
            /* swap slot offsets to reflect the swapping done by HW */
            offset              = slot_offset_phys[0];
            slot_offset_phys[0] = slot_offset_phys[1];
            slot_offset_phys[1] = offset;
#else
            const char *remap_type = "REMAP_OVERLAY";
            /* no need to adjust offset as the read operation is done using
               directly the FlexSPI IP */
#endif
            PRINTF("Flash %s active.\n\n", remap_type);
        }
        
        PRINTF("Image %d; name %s; state %s:\n",
               image, name, bl_imgstate_to_str(imgstate));

        for (int slot = 0; slot < 2; slot++)
        {
            int faid = (image * 2) + slot;
            struct flash_area *fa = &boot_flash_map[faid];
            static struct image_header ih;
            int slotused;
            
            PRINTF("\n  Slot %d %s; offset 0x%x; size 0x%x (%u): \n",
                    faid, fa->fa_name, fa->fa_off, fa->fa_size, fa->fa_size);
            
            ret = bl_flash_read(slot_offset_phys[slot], (uint32_t *)&ih, sizeof(ih));          
            if (ret != 0)
            {
                PRINTF("    Failed to read!\n");
                continue;
            }
            
            slotused = (ih.ih_magic == IMAGE_MAGIC);

            if (slotused)
            {
                struct image_version *iv = &ih.ih_ver;
                uint8_t sha256[32];
                uint32_t img_size = ih.ih_img_size;
                uint32_t img_off = ih.ih_hdr_size;
                void *log_addr = mflash_drv_phys2log(fa->fa_off, 0);

                PRINTF("    <IMAGE: size %u; version %u.%u.%u+%u>\n",
                       ih.ih_img_size,
                       iv->iv_major, iv->iv_minor, iv->iv_revision, iv->iv_build_num);
                
                if (hashfunc)
                {
                    hashfunc(slot_offset_phys[slot] + img_off, img_size, sha256);
                    PRINTF("    SHA256 of image payload: ");
                    print_hash(sha256, 10);
                    PRINTF("...\n");
                }
                
                /* print logical address only when remap is active or log_addr != phy_addr */
                if (remap_active || (uint32_t)log_addr != fa->fa_off)
                {
                    PRINTF("    log_addr 0x%x", log_addr);
                    if (remap_active)
                    {
                        void *log_addr_remap;
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_BY_SWAP
                        /* REMAP_SWAP: read from one slot translates to the other slot */
                        log_addr_remap = mflash_drv_phys2log(slot_offset_phys[slot], 0);
#else
                        /* REMAP_OVERLAY: read  from primary slot translates to the secondary slot
                                          read from secondary slot is not affected */
                        log_addr_remap = mflash_drv_phys2log(boot_flash_map[1].fa_off, 0);
#endif
                        PRINTF(" remaps to 0x%x", log_addr_remap);
                    }
                    PUTCHAR('\n');
                }
                
                if (boot_flash_map[get_active_image(image)].fa_off == slot_offset_phys[slot])
                {
                    PRINTF("    *ACTIVE*\n");
#if defined(CONFIG_ENCRYPT_XIP_EXT_ENABLE) && !defined(CONFIG_ENCRYPT_XIP_EXT_OVERWRITE_ONLY)
                    PRINTF("    Encrypted XIP: This slot is linked to execution slot\n");
#endif
                }
            }
            else
            {
                PRINTF("    <No Image Found>\n");
            }
        }
    }
    PUTCHAR('\n');
}

/** Wrapper function for flash memory access which deals with flash remap and
 *  peripheral limitation.
 *
 * @retval 0  OK
 *         -1 Fail
 */
int32_t bl_flash_read(uint32_t addr, uint32_t *buffer, uint32_t len)
{
    uint8_t *buffer_u8 = (uint8_t *)buffer;

    /* inter-page lenght of the first read can be smaller than page size */
    size_t plen = MFLASH_PAGE_SIZE - (addr % MFLASH_PAGE_SIZE);

    while (len > 0)
    {
        size_t readsize = (len > plen) ? plen : len;

#if defined(MFLASH_PAGE_INTEGRITY_CHECKS) && MFLASH_PAGE_INTEGRITY_CHECKS
        if (mflash_drv_is_readable(addr) != kStatus_Success)
        {
            /* PRINTF("%s: UNREADABLE PAGE at %x\n", __func__, addr); */
            memset(buffer_u8, 0xff, readsize);
        }
        else
#endif
        {
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
            /* remapped memory is accessible only by flash peripheral */
            if (mflash_drv_read_wrapper(addr, (uint32_t *)buffer_u8, readsize) != kStatus_Success)
            {
                return -1;
            }
#else
            void *flash_ptr = mflash_drv_phys2log(addr, readsize);
            if (flash_ptr == NULL)
            {
                return -1;
            }
            /* use direct memcpy as mflash_drv_read low layer may expects len to be word aligned */
            memcpy(buffer_u8, flash_ptr, readsize);
#endif /* CONFIG_MCUBOOT_FLASH_REMAP_ENABLE */
        }

        len -= readsize;
        addr += readsize;
        buffer_u8 += readsize;

        plen = MFLASH_PAGE_SIZE;
    }

    return 0;
}