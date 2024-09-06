/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_PBAP_PCE_H__
#define __APP_PBAP_PCE_H__

#define CURRENT_PATH_MAX_LEN 30

struct pbap_hdr
{
    uint8_t *value;
    uint16_t length;
};

struct appl_params
{
    /**
     * Attribute Mask
     */
    uint64_t property_selector;

    /**
     * Maximum entries in the list
     */
    uint16_t max_list_count;

    /**
     * Offset of the first entry
     */
    uint16_t list_start_offset;

    /**
     * Number of indexes in phonebook
     */
    uint16_t phonebook_size;

    /**
     * Application Parameter Bit field
     */

    /**
     *  16 Bit Flag used as below
     *
     *  Usage:
     *       15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
     *      [  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  ]
     *        |  |  |  |  |  | |  |   |  |  |  |  |  |  |  |____Order
     *        |  |  |  |  |  | |  |   |  |  |  |  |  |  |_______Search Value
     *        |  |  |  |  |  | |  |   |  |  |  |  |  |__________Search Attribute
     *        |  |  |  |  |  | |  |   |  |  |  |  |_____________Max List Count
     *        |  |  |  |  |  | |  |   |  |  |  |________________List Start Offset
     *        |  |  |  |  |  | |  |   |  |  |___________________Property Selector
     *        |  |  |  |  |  | |  |   |  |______________________Format
     *        |  |  |  |  |  | |  |   |_________________________Phone book size
     *        |  |  |  |  |  | |  |_____________________________New Missed Calls
     *        |  |  |  |  |  | |________________________________PrimaryVersionCounter
     *        |  |  |  |  |  |__________________________________SecondaryVersionCounter
     *        |  |  |  |  |_____________________________________vCardSelector
     *        |  |  |  |________________________________________DatabaseIdentifier
     *        |  |  |___________________________________________vCardSelectorOperator
     *        |  |______________________________________________ResetNewMissedCalls
     *        |_________________________________________________PbapSupportedFeatures
     *
     *
     */
    uint16_t appl_param_flag;

    /**
     * Sorting Order
     */
    uint8_t order;

    /**
     * Attribute to be searched
     */
    uint8_t search_attr;

    /**
     * vCard Format 2.1 or 3.0
     */
    uint8_t format;

    /**
     * Number of new missed calls
     */
    uint8_t new_missed_calls;
    /**
     * PSE primary folder version
     */
    uint8_t primary_folder_ver[16];

    /**
     * PSE secodary folder version
     */
    uint8_t secondary_folder_ver[16];

    /**
     * PSE database identifier
     */
    uint8_t database_identifier[16];

    /**
     * vCard Selector, Attribute mask
     */
    uint64_t vcard_selector;

    /**
     * vCard selector operator when multiple
     * attributes are slector
     */
    uint8_t vcard_selector_operator;

    /**
     * To reset newmissed calls
     */
    uint8_t reset_new_missed_calls;

    /**
     * PBAP supported features
     */
    uint32_t supported_features;

    /**
     * Value of the Search attribute
     */
    struct pbap_hdr search_value;
};

typedef struct app_pbap_pse_
{
    struct bt_pbap_pse *pbap_pseHandle;
    struct bt_conn *conn;
    uint8_t peer_bd_addr[6];
    char currentpath[CURRENT_PATH_MAX_LEN];
    uint32_t remaining_rsp;
    struct appl_params req_appl_params;
    char name[42];
    uint32_t send_rsp;
    uint32_t lcl_supported_features;
    uint32_t rem_supported_features;
} app_pbap_pse_t;

void pbap_pse_task(void *pvParameters);

#endif /* __APP_PBAP_PCE_H__ */
