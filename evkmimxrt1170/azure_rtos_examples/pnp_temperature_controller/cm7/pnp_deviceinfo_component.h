/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation 
 * 
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 * 
 * SPDX-License-Identifier: MIT
 **************************************************************************/

#ifndef SAMPLE_PNP_DEVICEINFO_COMPONENT_H
#define SAMPLE_PNP_DEVICEINFO_COMPONENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_azure_iot_hub_client.h"
#include "nx_api.h"

UINT sample_pnp_deviceinfo_report_all_properties(UCHAR *component_name_ptr, UINT component_name_len,
                                                 NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);

#ifdef __cplusplus
}
#endif
#endif /* SAMPLE_PNP_DEVICEINFO_COMPONENT_H */
