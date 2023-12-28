
/**
 *  \file appl_utils.c
 *
 *  Main Application Utility File
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_utils.h"

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
/* Flag that Controls BD Address Input Formats */
#define APPL_BD_ADDR_STRICT_SPACED_INPUT
#define BT_BD_ADDR_STR_SIZE 12

/**
 * NOTE:
 * based on some discussions in forums typically for scanf or printf which
 * involve unsigned char input and output, the format specifier to be used is
 * - "%hhx" (half of half of int!)
 *   eg: scanf("%hhx", &uchar_val);
 * - or #include <inttypes.h> and use "%"SCNx8, which needs to be provided by
 *   the tool-chain.
 * - Or, the safer approach would be to take the inputs from Scanf into a
 *   variable of type "int" and type-cast it to "UCHAR".
 */
/* --------------------------------------------- Functions */
API_RESULT appl_get_bd_addr(UCHAR *bd_addr)
{
#ifndef APPL_BD_ADDR_STRICT_SPACED_INPUT
    int   read_val;
    UCHAR bd[BT_BD_ADDR_SIZE];
    int i;
#else  /* APPL_BD_ADDR_STRICT_SPACED_INPUT */
    UINT16 str_len;
    CHAR str[20];
#endif /* APPL_BD_ADDR_STRICT_SPACED_INPUT */

    int i, j;
    fflush(stdout);
    BT_mem_set(bd_addr, 0x00, BT_BD_ADDR_SIZE);

#ifndef APPL_BD_ADDR_STRICT_SPACED_INPUT
    BT_mem_set(bd, 0x00, sizeof(bd));
    for(i = 0; i < BT_BD_ADDR_SIZE; i ++)
    {
        scanf("%x", &read_val);
        bd[i] = (UCHAR)read_val;
    }

    /* Copy the Address into the requested parameter */
    BT_mem_copy(bd_addr, bd, BT_BD_ADDR_SIZE);
#else /* APPL_BD_ADDR_STRICT_SPACED_INPUT */


    /* Initialize */
    BT_mem_set(str, 0x00, sizeof(str));
    scanf("%s %s %s %s %s %s", &str[0U], &str[2U], &str[4U], &str[6U], &str[8U], &str[10U]);
    printf("BD_ADDR Entered %s\n", str);
    str_len = BT_str_len(str);
    if (BT_BD_ADDR_STR_SIZE != str_len)
    {
        printf("Invalid Bluetooth Address length %d \n", str_len);
        return API_FAILURE;
    }
    for (i = 0; i < BT_BD_ADDR_STR_SIZE; i++)
    {
        if ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'F') || (str[i] >= 'a' && str[i] <= 'f'))
        {
            continue;
        }
        else
        {
            printf("Invalid Bluetooth Address \n");
            return API_FAILURE;
        }
    }

    for(i = 0, j = 0; i < BT_BD_ADDR_SIZE; i++, j += 2)
    {
        bd_addr[i] = (UCHAR)EM_str_to_hexnum_pl(&(str[j]), 2);
    }
#endif /* APPL_BD_ADDR_STRICT_SPACED_INPUT */

    return API_SUCCESS;
}

API_RESULT appl_validate_params(int *read_val, UINT16 max_length, UINT16 min_option, UINT16 max_option)
{
    UCHAR str[5] = "";
    API_RESULT retval;
    UINT16 length,j;

    scanf ("%s", str);
    length = BT_str_len (str);

    for (j = 0; j < length; j++)
    {
        if ((max_length < length ) || !((str[j] >= '0') && (str[j] <= '9')))
        {
            printf ("Invalid option\n");
            retval = API_FAILURE;
            break;
        }
        else
        {
            *read_val = appl_str_to_num(str,length);
            if ((min_option > *read_val) || (max_option < *read_val))
            {
                printf ("Invalid option\n");
                retval = API_FAILURE;
                break;
            }
            else
            {
                retval = API_SUCCESS;
            }
        }
    }
    return retval;
}

API_RESULT appl_validate_strparams(UCHAR *read_val, UINT16 max_length)
{
    API_RESULT retval;
    UINT16 str_len,i;

    retval = API_SUCCESS;

    str_len = BT_str_len(read_val);

    if(str_len > max_length)
    {
        printf("Invalid  length %d \n", str_len);
        retval = API_FAILURE;
    }
    else
    {
        for (i = 0; i < str_len; i++)
        {
            if ((read_val[i] < '0' || read_val[i] > '9') && (read_val[i] < 'A' || read_val[i] > 'F') && (read_val[i] < 'a' || read_val[i] > 'f'))
            {
                printf("Invalid value \n");
                retval = API_FAILURE;
                break;
            }
        }
    }
    return retval;
}

void appl_input_octets(UCHAR *buf, UINT16 len)
{
    int i, read;

    for (i = 0; i < len; i++)
    {
        scanf("%x", &read);
        buf[i] = (UCHAR)read;
    }
}

void appl_dump_bytes (UCHAR *buffer, UINT16 length)
{
#ifndef APPL_LIMIT_LOGS
    char hex_stream[49U];
    char char_stream[17U];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;

    printf("\n");
    printf("-- Dumping %d Bytes --\n",
    (int)length);

    printf(
    "-------------------------------------------------------------------\n");

    count = 0U;
    offset = 0U;
    for(i = 0U; i < length; i ++)
    {
        c =  buffer[i];
        BT_str_print(&hex_stream[offset], "%02X ", c);

        if ( (c >= 0x20U) && (c <= 0x7EU) )
        {
            char_stream[count] = c;
        }
        else
        {
            char_stream[count] = '.';
        }

        count ++;
        offset += 3U;

        if(16U == count)
        {
            char_stream[count] = '\0';
            count = 0U;
            offset = 0U;

            printf("%s   %s\n",
            hex_stream, char_stream);

            BT_mem_set(hex_stream, 0, 49U);
            BT_mem_set(char_stream, 0, 17U);
        }
    }

    if(offset != 0U)
    {
        char_stream[count] = '\0';

        /* Maintain the alignment */
        printf("%-48s   %s\n",
        hex_stream, char_stream);
    }

    printf(
    "-------------------------------------------------------------------\n");

    printf("\n");
#endif /* APPL_LIMIT_LOGS */

    return;
}

void appl_dump_bytes_no_limit_logs (UCHAR *buffer, UINT16 length)
{
    char hex_stream[49U];
    char char_stream[17U];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;

    printf("\n");
    printf("-- Dumping %d Bytes --\n",
    (int)length);

    printf(
    "-------------------------------------------------------------------\n");

    count = 0U;
    offset = 0U;
    for(i = 0U; i < length; i ++)
    {
        c =  buffer[i];
        BT_str_print(&hex_stream[offset], "%02X ", c);

        if ( (c >= 0x20U) && (c <= 0x7EU) )
        {
            char_stream[count] = c;
        }
        else
        {
            char_stream[count] = '.';
        }

        count ++;
        offset += 3U;

        if(16U == count)
        {
            char_stream[count] = '\0';
            count = 0U;
            offset = 0U;

            printf("%s   %s\n",
            hex_stream, char_stream);

            BT_mem_set(hex_stream, 0, 49U);
            BT_mem_set(char_stream, 0, 17U);
        }
    }

    if(offset != 0U)
    {
        char_stream[count] = '\0';

        /* Maintain the alignment */
        printf("%-48s   %s\n",
        hex_stream, char_stream);
    }

    printf(
    "-------------------------------------------------------------------\n");

    printf("\n");
    return;
}


UINT32 appl_str_to_num
       (
           /* IN */  UCHAR  * str,
           /* IN */  UINT16 len
       )
{
    return EM_str_to_num_pl((CHAR *)str, len);
}

UINT32 appl_str_to_hexnum
       (
           /* IN */ UCHAR* str,
           /* IN */ UINT16 len
       )
{
    return EM_str_to_hexnum_pl((CHAR *)str, len);
}

void appl_num_to_str
     (
         /* IN  */ UINT32   num,
         /* OUT */ UCHAR  * str,
         /* OUT */ UCHAR  * len
     )
{
    int length;

    length = 0;
    EM_num_to_str_pl(num, (CHAR *)str, &length);
    *len = (UCHAR)length;
}
