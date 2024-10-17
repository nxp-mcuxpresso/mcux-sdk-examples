/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * Copyright 2008-2023 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**@file ncp_host_utils.h
 *
 * \brief Utility functions
 *
 */

#ifndef __NCP_HOST_UTILS_H__
#define __NCP_HOST_UTILS_H__

#include <string.h>
#include <ctype.h>
#include "fsl_debug_console.h"

#if CONFIG_ENABLE_ERROR_LOGS
#define wmlog_e(_mod_name_, _fmt_, ...) \
    (void)PRINTF("%d: [%s]%s" _fmt_ "\n\r", OSA_TimeGetMsec(), _mod_name_, " Error: ", ##__VA_ARGS__)
#else
#define wmlog_e(...)
#endif /* CONFIG_ENABLE_ERROR_LOGS */

#if CONFIG_ENABLE_WARNING_LOGS
#define wmlog_w(_mod_name_, _fmt_, ...) \
    (void)PRINTF("%d: [%s]%s" _fmt_ "\n\r", OSA_TimeGetMsec(), _mod_name_, " Warn: ", ##__VA_ARGS__)
#else
#define wmlog_w(...)
#endif /* CONFIG_ENABLE_WARNING_LOGS */

/* General debug function. User can map his own debug functions to this
ne */
#define wmlog(_mod_name_, _fmt_, ...) (void)PRINTF("%d: [%s] " _fmt_ "\n\r", OSA_TimeGetMsec(), _mod_name_, ##__VA_ARGS__)

/* Function entry */
#define wmlog_entry(_fmt_, ...) (void)PRINTF("%d: > %s (" _fmt_ ")\n\r", OSA_TimeGetMsec(), __func__, ##__VA_ARGS__)

/* function exit */
#define wmlog_exit(_fmt_, ...) (void)PRINTF("%d: < %s" _fmt_ "\n\r", OSA_TimeGetMsec(), __func__, ##__VA_ARGS__)

#define mcu_in_range(c, lo, up) ((uint8_t)(c) >= (lo) && (uint8_t)(c) <= (up))
#define mcu_isdigit(c)          mcu_in_range((c), '0', '9')
#define mcu_isxdigit(c)         (mcu_isdigit(c) || mcu_in_range((c), 'a', 'f') || mcu_in_range((c), 'A', 'F'))
#define mcu_islower(c)          mcu_in_range((c), 'a', 'z')
#define mcu_isspace(c)          ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')
#define mcu_isupper(c)          mcu_in_range((c), 'A', 'Z')
#define mcu_tolower(c)          (mcu_isupper(c) ? (c) - 'A' + 'a' : c)
#define mcu_toupper(c)          (mcu_islower(c) ? (c) - 'a' + 'A' : c)

#define MOD_ERROR_START(x) (x << 12 | 0)
#define MOD_WARN_START(x)  (x << 12 | 1)
#define MOD_INFO_START(x)  (x << 12 | 2)

#define inet_ntoa(addr)     ip4addr_ntoa((const ip4_addr_t *)&(addr))
#define inet_aton(cp, addr) ip4addr_aton(cp, (ip4_addr_t *)addr)
#define inet6_ntoa(addr)    ip6addr_ntoa((const ip6_addr_t *)&(addr))
#define xchar(i)            ((char)((i) < 10 ? '0' + (i) : 'A' + (i)-10))
#define PP_HTONS(x)         ((uint16_t)((((x) & (uint16_t)0x00ffU) << 8) | (((x) & (uint16_t)0xff00U) >> 8)))
#define PP_HTONL(x)                                                                   \
    ((((x) & (uint32_t)0x000000ffUL) << 24) | (((x) & (uint32_t)0x0000ff00UL) << 8) | \
     (((x) & (uint32_t)0x00ff0000UL) >> 8) | (((x) & (uint32_t)0xff000000UL) >> 24))
#define ip6_addr_isipv4mappedipv6(ip6addr) \
    (((ip6addr)->addr[0] == 0) && ((ip6addr)->addr[1] == 0) && (((ip6addr)->addr[2]) == PP_HTONL(0x0000FFFFUL)))

#define FOLD_U32T(u)          ((uint32_t)(((u) >> 16) + ((u)&0x0000ffffUL)))
#define SWAP_BYTES_IN_WORD(w) (((w)&0xff) << 8) | (((w)&0xff00) >> 8)

/* Create Module index */
#define MOD_GENERIC 0
/** Unused */
#define MOD_UNUSED_3 2
/** HTTPD module index */
#define MOD_HTTPD 3
/** Application framework module index */
#define MOD_AF 4
/** FTFS module index */
#define MOD_FTFS 5
/** RFGET module index */
#define MOD_RFGET 6
/** JSON module index  */
#define MOD_JSON 7
/** TELNETD module index */
#define MOD_TELNETD 8
/** SIMPLE MDNS module index */
#define MOD_SMDNS 9
/** EXML module index */
#define MOD_EXML 10
/** DHCPD module index */
#define MOD_DHCPD 11
/** MDNS module index */
#define MOD_MDNS 12
/** SYSINFO module index */
#define MOD_SYSINFO 13
/** Unused module index */
#define MOD_UNUSED_1 14
/** CRYPTO module index */
#define MOD_CRYPTO 15
/** HTTP-CLIENT module index */
#define MOD_HTTPC 16
/** PROVISIONING module index */
#define MOD_PROV 17
/** SPI module index */
#define MOD_SPI 18
/** PSM module index */
#define MOD_PSM 19
/** TTCP module index */
#define MOD_TTCP 20
/** DIAGNOSTICS module index */
#define MOD_DIAG 21
/** Unused module index */
#define MOD_UNUSED_2 22
/** WPS module index */
#define MOD_WPS 23
/** WLAN module index */
#define MOD_WLAN 24
/** USB module index */
#define MOD_USB 25
/** WIFI driver module index */
#define MOD_WIFI 26
/** Critical error module index */
#define MOD_CRIT_ERR 27
/** Last module index .Applications can define their own modules beyond this */
#define MOD_ERR_LAST 50

/* Globally unique success code */
#define WM_SUCCESS 0

enum wm_errno
{
    /* First Generic Error codes */
    WM_GEN_E_BASE = MOD_ERROR_START(MOD_GENERIC),
    WM_FAIL,     /* 1 */
    WM_E_PERM,   /* 2: Operation not permitted */
    WM_E_NOENT,  /* 3: No such file or directory */
    WM_E_SRCH,   /* 4: No such process */
    WM_E_INTR,   /* 5: Interrupted system call */
    WM_E_IO,     /* 6: I/O error */
    WM_E_NXIO,   /* 7: No such device or address */
    WM_E_2BIG,   /* 8: Argument list too long */
    WM_E_NOEXEC, /* 9: Exec format error */
    WM_E_BADF,   /* 10: Bad file number */
    WM_E_CHILD,  /* 11: No child processes */
    WM_E_AGAIN,  /* 12: Try again */
    WM_E_NOMEM,  /* 13: Out of memory */
    WM_E_ACCES,  /* 14: Permission denied */
    WM_E_FAULT,  /* 15: Bad address */
    WM_E_NOTBLK, /* 16: Block device required */
    WM_E_BUSY,   /* 17: Device or resource busy */
    WM_E_EXIST,  /* 18: File exists */
    WM_E_XDEV,   /* 19: Cross-device link */
    WM_E_NODEV,  /* 20: No such device */
    WM_E_NOTDIR, /* 21: Not a directory */
    WM_E_ISDIR,  /* 22: Is a directory */
    WM_E_INVAL,  /* 23: Invalid argument */
    WM_E_NFILE,  /* 24: File table overflow */
    WM_E_MFILE,  /* 25: Too many open files */
    WM_E_NOTTY,  /* 26: Not a typewriter */
    WM_E_TXTBSY, /* 27: Text file busy */
    WM_E_FBIG,   /* 28: File too large */
    WM_E_NOSPC,  /* 29: No space left on device */
    WM_E_SPIPE,  /* 30: Illegal seek */
    WM_E_ROFS,   /* 31: Read-only file system */
    WM_E_MLINK,  /* 32: Too many links */
    WM_E_PIPE,   /* 33: Broken pipe */
    WM_E_DOM,    /* 34: Math argument out of domain of func */
    WM_E_RANGE,  /* 35: Math result not representable */

    /* WMSDK generic error codes */
    WM_E_CRC,     /* 36: Error in CRC check */
    WM_E_UNINIT,  /* 37: Module is not yet initialized */
    WM_E_TIMEOUT, /* 38: Timeout occurred during operation */

    /* Defined for Hostcmd specific API*/
    WM_E_INBIG,   /* 39: Input buffer too big */
    WM_E_INSMALL, /* 40: A finer version for WM_E_INVAL, where it clearly specifies that input is much smaller than
                     minimum requirement */
    WM_E_OUTBIG,  /* 41: Data output exceeds the size provided */
};

/** This is the aligned version of ip4_addr_t,
   used as local variable, on the stack, etc. */
struct ip4_addr
{
    uint32_t addr;
};

typedef uint32_t in_addr_t;

struct in_addr
{
    in_addr_t s_addr;
};

/** ip4_addr_t uses a struct for convenience only, so that the same defines can
 * operate both on ip4_addr_t as well as on ip4_addr_p_t. */
typedef struct ip4_addr ip4_addr_t;

/** This is the aligned version of ip6_addr_t,
    used as local variable, on the stack, etc. */
struct ip6_addr
{
    uint32_t addr[4];
};

/** IPv6 address */
typedef struct ip6_addr ip6_addr_t;

#define IEEEtypes_SSID_SIZE 32
/* Min WPA2 passphrase can be upto 8 ASCII chars */
#define WLAN_PSK_MIN_LENGTH 8U
/** Max WPA2 passphrase can be upto 63 ASCII chars or 64 hexadecimal digits*/
#define WLAN_PSK_MAX_LENGTH 65U
#define SECURE_TYPE_LENGTH  10

typedef NCP_TLV_PACK_START struct _ncp_commission_cfg_t
{
    /** ssid */
    char ssid[IEEEtypes_SSID_SIZE];
    /** length of ssid */
    uint8_t ssid_len;
    /** password */
    char password[WLAN_PSK_MAX_LENGTH];
    /** length of password */
    uint8_t password_len;
    /** secure type */
    char secure[SECURE_TYPE_LENGTH];
    /** length of secure type */
    uint8_t secure_len;
} NCP_TLV_PACK_END ncp_commission_cfg_t;

/**
 * Convert a given hex string to a equivalent binary representation.
 *
 * E.g. If your input string of 4 bytes is {'F', 'F', 'F', 'F'} the output
 * string will be of 2 bytes {255, 255} or to put the same in other way
 * {0xFF, 0xFF}
 *
 * Note that hex2bin is not the same as strtoul as the latter will properly
 * return the integer in the correct machine binary format viz. little
 * endian. hex2bin however does only in-place like replacement of two ASCII
 * characters to one binary number taking 1 byte in memory.
 *
 * @param[in] ibuf input buffer
 * @param[out] obuf output buffer
 * @param[in]  max_olen Maximum output buffer length
 *
 * @return length of the binary string
 */
static inline unsigned int hex2bin(const uint8_t *ibuf, uint8_t *obuf, unsigned max_olen)
{
    unsigned int i;      /* loop iteration variable */
    unsigned int j  = 0; /* current character */
    unsigned int by = 0; /* byte value for conversion */
    unsigned char ch;    /* current character */
    unsigned int len = strlen((char *)ibuf);
    /* process the list of characters */
    for (i = 0; i < len; i++)
    {
        if (i == (2 * max_olen))
        {
            (void)PRINTF("hexbin",
                         "Destination full. "
                         "Truncating to avoid overflow.\r\n");
            return j + 1;
        }
        ch = mcu_toupper(*ibuf++); /* get next uppercase character */

        /* do the conversion */
        if (ch >= '0' && ch <= '9')
            by = (by << 4) + ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            by = (by << 4) + ch - 'A' + 10;
        else
        { /* error if not hexadecimal */
            return 0;
        }

        /* store a byte for each pair of hexadecimal digits */
        if (i & 1)
        {
            j       = ((i + 1) / 2) - 1;
            obuf[j] = by & 0xff;
        }
    }
    return j + 1;
}

static inline void print_mac(const char *mac)
{
    (void)PRINTF("%02X:%02X:%02X:%02X:%02X:%02X ", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/* Parse string 'arg' formatted "AA:BB:CC:DD:EE:FF" (assuming 'sep' is ':')
 * into a 6-byte array 'dest' such that dest = {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF}
 * set 'sep' accordingly. */
static inline int get_mac(const char *arg, char *dest, char sep)
{
    unsigned char n;
    int i, j, k;

    if (strlen(arg) < 17)
        return 1;

    (void)memset(dest, 0, 6);

    for (i = 0, k = 0; i < 17; i += 3, k++)
    {
        for (j = 0; j < 2; j++)
        {
            if (arg[i + j] >= '0' && arg[i + j] <= '9')
                n = arg[i + j] - '0';
            else if (arg[i + j] >= 'A' && arg[i + j] <= 'F')
                n = arg[i + j] - 'A' + 10;
            else if (arg[i + j] >= 'a' && arg[i + j] <= 'f')
                n = arg[i + j] - 'a' + 10;
            else
                return 1;

            n <<= 4 * (1 - j);
            dest[k] += n;
        }
        if (i < 15 && arg[i + 2] != sep)
            return 1;
    }

    return 0;
}

/**
 *@brief convert char to hex integer
 *
 *@param chr          char
 *@return             hex integer
 **/
static inline uint8_t hexc2bin(char chr)
{
    if (chr >= '0' && chr <= '9')
        chr -= '0';
    else if (chr >= 'A' && chr <= 'F')
        chr -= ('A' - 10);
    else if (chr >= 'a' && chr <= 'f')
        chr -= ('a' - 10);
    else
    { /* Do Nothing */
    }
    return chr;
}

/**
 *@brief convert string to hex integer
 *
 *@param s            A pointer string buffer
 *@return             hex integer
 **/
static inline uint32_t a2hex(const char *s)
{
    uint32_t val = 0;

    if (!strncasecmp("0x", s, 2))
    {
        s += 2;
    }

    while (*s && (mcu_isdigit((unsigned char)*s) || mcu_islower((unsigned char)*s) || mcu_isupper((unsigned char)*s)))
    {
        val = (val << 4) + hexc2bin(*s++);
    }
    return val;
}

/*
 * @brief convert String to integer
 *
 *@param value        A pointer to string
 *@return             integer
 **/
static inline uint32_t a2hex_or_atoi(char *value)
{
    if (value[0] == '0' && (value[1] == 'X' || value[1] == 'x'))
    {
        return a2hex(value + 2);
    }
    else if (mcu_isdigit((unsigned char)*value) != 0)
    {
        return atoi(value);
    }
    else
    {
        return *value;
    }
}

#define CHAR2INT(x) (('0' <= x && x <= '9') ? \
    (x - '0') : \
    (('a' <= x && x <= 'f') ? \
        (10 + (x - 'a')) : \
        (('A' <= x && x <= 'F') ? (10 + (x - 'A')) : (0))))

static inline uint8_t uuid_str_valid(const char *uuid)
{
	int i, valid;

	if (uuid == NULL)
		return 0;

	for (i = 0, valid = 1; uuid[i] && valid; i++) {
		switch (i) {
		case 8: case 13: case 18: case 23:
			valid = (uuid[i] == '-');
			break;
		default:
			valid = isxdigit((int)uuid[i]);
			break;
		}
	}

	if (i != 16 || !valid)
		return 0;

	return 1;
}

static inline uint8_t uuid2arry(const char *uuid, uint8_t *arry, uint8_t type)
{
    if(type == 2)//UUID16
    {
        arry[1] = (CHAR2INT(uuid[0]) << 4) + CHAR2INT(uuid[1]);
        arry[0] = (CHAR2INT(uuid[2]) << 4) + CHAR2INT(uuid[3]);
    }
    else
    {
        if(!uuid_str_valid(uuid))
            return 1;
        arry[15] = (CHAR2INT(uuid[0]) << 4) + CHAR2INT(uuid[1]);
        arry[14] = (CHAR2INT(uuid[2]) << 4) + CHAR2INT(uuid[3]);
        arry[13] = (CHAR2INT(uuid[4]) << 4) + CHAR2INT(uuid[5]);
        arry[12] = (CHAR2INT(uuid[6]) << 4) + CHAR2INT(uuid[7]);

        arry[11] = (CHAR2INT(uuid[9]) << 4) + CHAR2INT(uuid[10]);
        arry[10] = (CHAR2INT(uuid[11]) << 4) + CHAR2INT(uuid[12]);

        arry[9] = (CHAR2INT(uuid[14]) << 4) + CHAR2INT(uuid[15]);
        arry[8] = (CHAR2INT(uuid[16]) << 4) + CHAR2INT(uuid[17]);

        arry[7] = (CHAR2INT(uuid[19]) << 4) + CHAR2INT(uuid[20]);
        arry[6] = (CHAR2INT(uuid[21]) << 4) + CHAR2INT(uuid[22]);

        arry[5] = (CHAR2INT(uuid[24]) << 4) + CHAR2INT(uuid[25]);
        arry[4] = (CHAR2INT(uuid[26]) << 4) + CHAR2INT(uuid[27]);
        arry[3] = (CHAR2INT(uuid[28]) << 4) + CHAR2INT(uuid[29]);
        arry[2] = (CHAR2INT(uuid[30]) << 4) + CHAR2INT(uuid[31]);
        arry[1] = (CHAR2INT(uuid[32]) << 4) + CHAR2INT(uuid[33]);
        arry[0] = (CHAR2INT(uuid[34]) << 4) + CHAR2INT(uuid[35]);
    }

    return 0;
}

/**
 * Same as ip4addr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param addr ip address in network order to convert
 * @param buf target buffer where the string is stored
 * @param buflen length of buf
 * @return either pointer to buf which now holds the ASCII
 *         representation of addr or NULL if buf was too small
 */
static inline char *ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen)
{
    uint32_t s_addr;
    char inv[3];
    char *rp;
    uint8_t *ap;
    uint8_t rem;
    uint8_t n;
    uint8_t i;
    int len = 0;

    s_addr = addr->addr;

    rp = buf;
    ap = (uint8_t *)&s_addr;
    for (n = 0; n < 4; n++)
    {
        i = 0;
        do
        {
            rem = *ap % (uint8_t)10;
            *ap /= (uint8_t)10;
            inv[i++] = (char)('0' + rem);
        } while (*ap);
        while (i--)
        {
            if (len++ >= buflen)
            {
                return NULL;
            }
            *rp++ = inv[i];
        }
        if (len++ >= buflen)
        {
            return NULL;
        }
        *rp++ = '.';
        ap++;
    }
    *--rp = 0;
    return buf;
}

/**
 * Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         representation of addr
 */
static inline char *ip4addr_ntoa(const ip4_addr_t *addr)
{
    static char str[16];
    (void)memset(str, 0, sizeof(str));	
	
    return ip4addr_ntoa_r(addr, str, 16);
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii representation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
static inline int ip4addr_aton(const char *cp, ip4_addr_t *addr)
{
    uint32_t val;
    uint8_t base;
    char c;
    uint32_t parts[4];
    uint32_t *pp = parts;

    c = *cp;
    for (;;)
    {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!isdigit(c))
        {
            return 0;
        }
        val  = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c    = *++cp;
            }
            else
            {
                base = 8;
            }
        }
        for (;;)
        {
            if (isdigit(c))
            {
                if ((base == 8) && ((uint32_t)(c - '0') >= 8))
                    break;
                val = (val * base) + (uint32_t)(c - '0');
                c   = *++cp;
            }
            else if (base == 16 && isdigit(c))
            {
                val = (val << 4) | (uint32_t)(c + 10 - (islower(c) ? 'a' : 'A'));
                c   = *++cp;
            }
            else
            {
                break;
            }
        }
        if (c == '.')
        {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3)
            {
                return 0;
            }
            *pp++ = val;
            c     = *++cp;
        }
        else
        {
            break;
        }
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
    {
        return 0;
    }
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1)
    {
        case 0:
            return 0; /* initial nondigit */

        case 1: /* a -- 32 bits */
            break;

        case 2: /* a.b -- 8.24 bits */
            if (val > 0xffffffUL)
            {
                return 0;
            }
            if (parts[0] > 0xff)
            {
                return 0;
            }
            val |= parts[0] << 24;
            break;

        case 3: /* a.b.c -- 8.8.16 bits */
            if (val > 0xffff)
            {
                return 0;
            }
            if ((parts[0] > 0xff) || (parts[1] > 0xff))
            {
                return 0;
            }
            val |= (parts[0] << 24) | (parts[1] << 16);
            break;

        case 4: /* a.b.c.d -- 8.8.8.8 bits */
            if (val > 0xff)
            {
                return 0;
            }
            if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
            {
                return 0;
            }
            val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
            break;
        default:
            (void)PRINTF("unhandled");
            break;
    }
	
    /* The memory barrier mechanism of ARM ensures the correct sequence of program execution
     *  in multi-core or multi-threaded environments through DMB/DSB/ISB instructions.  
     *  The DSB/ISB instruction is used to ensure the completion of instructions and synchronization of data. 
     *  It ensures that all instructions before the DSB/ISB instruction have been executed, 
     *  and then the instructions after the DSB/ISB instruction are executed */
    __asm volatile ( "dsb" ::: "memory" );
    __asm volatile ( "isb" );
    if (addr)
    {
        addr->addr = PP_HTONL(val);
    }
    return 1;
}

/**
 * Same as ipaddr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param addr ip6 address in network order to convert
 * @param buf target buffer where the string is stored
 * @param buflen length of buf
 * @return either pointer to buf which now holds the ASCII
 *         representation of addr or NULL if buf was too small
 */
static inline char *ip6addr_ntoa_r(const ip6_addr_t *addr, char *buf, int buflen)
{
    uint32_t current_block_index, current_block_value, next_block_value;
    int i;
    uint8_t zero_flag, empty_block_flag;

    if (ip6_addr_isipv4mappedipv6(addr))
    {
        /* This is an IPv4 mapped address */
        ip4_addr_t addr4;
        char *ret;
#define IP4MAPPED_HEADER "::FFFF:"
        char *buf_ip4  = buf + sizeof(IP4MAPPED_HEADER) - 1;
        int buflen_ip4 = buflen - sizeof(IP4MAPPED_HEADER) + 1;
        if (buflen < (int)sizeof(IP4MAPPED_HEADER))
        {
            return NULL;
        }
        memcpy(buf, IP4MAPPED_HEADER, sizeof(IP4MAPPED_HEADER));
        addr4.addr = addr->addr[3];
        ret        = ip4addr_ntoa_r(&addr4, buf_ip4, buflen_ip4);
        if (ret != buf_ip4)
        {
            return NULL;
        }
        return buf;
    }
    i                = 0;
    empty_block_flag = 0; /* used to indicate a zero chain for "::' */

    for (current_block_index = 0; current_block_index < 8; current_block_index++)
    {
        /* get the current 16-bit block */
        current_block_value = PP_HTONL(addr->addr[current_block_index >> 1]);
        if ((current_block_index & 0x1) == 0)
        {
            current_block_value = current_block_value >> 16;
        }
        current_block_value &= 0xffff;

        /* Check for empty block. */
        if (current_block_value == 0)
        {
            if (current_block_index == 7 && empty_block_flag == 1)
            {
                /* special case, we must render a ':' for the last block. */
                buf[i++] = ':';
                if (i >= buflen)
                {
                    return NULL;
                }
                break;
            }
            if (empty_block_flag == 0)
            {
                /* generate empty block "::", but only if more than one contiguous zero block,
                 * according to current formatting suggestions RFC 5952. */
                next_block_value = PP_HTONL(addr->addr[(current_block_index + 1) >> 1]);
                if ((current_block_index & 0x1) == 0x01)
                {
                    next_block_value = next_block_value >> 16;
                }
                next_block_value &= 0xffff;
                if (next_block_value == 0)
                {
                    empty_block_flag = 1;
                    buf[i++]         = ':';
                    if (i >= buflen)
                    {
                        return NULL;
                    }
                    continue; /* move on to next block. */
                }
            }
            else if (empty_block_flag == 1)
            {
                /* move on to next block. */
                continue;
            }
        }
        else if (empty_block_flag == 1)
        {
            /* Set this flag value so we don't produce multiple empty blocks. */
            empty_block_flag = 2;
        }

        if (current_block_index > 0)
        {
            buf[i++] = ':';
            if (i >= buflen)
            {
                return NULL;
            }
        }

        if ((current_block_value & 0xf000) == 0)
        {
            zero_flag = 1;
        }
        else
        {
            buf[i++]  = xchar(((current_block_value & 0xf000) >> 12));
            zero_flag = 0;
            if (i >= buflen)
            {
                return NULL;
            }
        }

        if (((current_block_value & 0xf00) == 0) && (zero_flag))
        {
            /* do nothing */
        }
        else
        {
            buf[i++]  = xchar(((current_block_value & 0xf00) >> 8));
            zero_flag = 0;
            if (i >= buflen)
            {
                return NULL;
            }
        }

        if (((current_block_value & 0xf0) == 0) && (zero_flag))
        {
            /* do nothing */
        }
        else
        {
            buf[i++]  = xchar(((current_block_value & 0xf0) >> 4));
            zero_flag = 0;
            if (i >= buflen)
            {
                return NULL;
            }
        }

        buf[i++] = xchar((current_block_value & 0xf));
        if (i >= buflen)
        {
            return NULL;
        }
    }

    buf[i] = 0;

    return buf;
}

/**
 * Convert numeric IPv6 address into ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip6 address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         representation of addr
 */
static inline char *ip6addr_ntoa(const ip6_addr_t *addr)
{
    static char str[40];
    return ip6addr_ntoa_r(addr, str, 40);
}

/* Non-reentrant getopt implementation */
extern int cli_optind;
extern char *cli_optarg;
static inline int cli_getopt(int argc, char **argv, const char *fmt)
{
    char *opt, *c;

    if (cli_optind == argc)
        return -1;
    cli_optarg = NULL;
    opt        = argv[cli_optind];
    if (opt[0] != '-')
        return -1;
    if (opt[0] == 0 || opt[1] == 0)
        return '?';
    cli_optind++;
    c = strchr(fmt, opt[1]);
    if (c == NULL)
        return opt[1];
    if (c[1] == ':')
    {
        if (cli_optind < argc)
            cli_optarg = argv[cli_optind++];
    }
    return c[0];
}

static inline uint16_t inet_chksum(const void *dataptr, int len)
{
    const uint8_t *pb = (const uint8_t *)dataptr;
    const uint16_t *ps;
    uint16_t t   = 0;
    uint32_t sum = 0;
    int odd      = ((uintptr_t)pb & 1);

    /* Get aligned to u16_t */
    if (odd && len > 0)
    {
        ((uint8_t *)&t)[1] = *pb++;
        len--;
    }

    /* Add the bulk of the data */
    ps = (const uint16_t *)(const void *)pb;
    while (len > 1)
    {
        sum += *ps++;
        len -= 2;
    }

    /* Consume left-over byte, if any */
    if (len > 0)
    {
        ((uint8_t *)&t)[0] = *(const uint8_t *)ps;
    }

    /* Add end bytes */
    sum += t;

    /* Fold 32-bit sum to 16 bits
       calling this twice is probably faster than if statements... */
    sum = FOLD_U32T(sum);
    sum = FOLD_U32T(sum);

    /* Swap if alignment was odd */
    if (odd)
    {
        sum = SWAP_BYTES_IN_WORD(sum);
    }

    return (uint16_t)(~(unsigned int)(uint16_t)sum);
}

/**
 * @brief       This function convters string to decimal number.
 *
 * @param arg   A pointer to string.
 * @param dest  A pointer to number.
 * @param len   Length of string arg.
 *
 * @return      return 0 if string arg can be convert to decimal number.
 */
static inline int get_uint(const char *arg, unsigned int *dest, unsigned int len)
{
    int i;
    unsigned int val = 0;

    for (i = 0; i < len; i++)
    {
        if (arg[i] < '0' || arg[i] > '9')
            return 1;
        val *= 10;
        val += arg[i] - '0';
    }

    *dest = val;
    return 0;
}

/**
 * @brief       This function judges if s1 and s2 are equal.
 *
 * @param s1   A pointer to string s1.
 * @param s2   A pointer to string s2.
 *
 * @return     Return 1 if s1 is equal to s2.
 */
static inline int string_equal(const char *s1, const char *s2)
{
    size_t len = strlen(s1);

    if (len == strlen(s2) && !strncmp(s1, s2, len))
        return 1;
    return 0;
}

void ncp_network_commissioning(ncp_commission_cfg_t *ncp_commission_cfg);
#endif /* __NCP_HOST_UTILS_H__ */
