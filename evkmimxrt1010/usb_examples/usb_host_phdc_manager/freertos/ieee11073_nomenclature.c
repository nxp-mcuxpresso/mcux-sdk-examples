/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb.h"
#include "ieee11073_types.h"
#include "ieee11073_nomenclature.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(__ICCARM__)
#define _INT8_P_CASTING_
#else
#define _INT8_P_CASTING_ (int8_t *)
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Nomenclature ascii table */
const nomenclature_ascii_struct_t g_nomenclatureAsciiTable[] = {
    {MDC_ATTRIBUTE_ID_TYPE, _INT8_P_CASTING_ "ID type"},
    {MDC_ATTRIBUTE_METRIC_SPECIFICATION_SMALL, _INT8_P_CASTING_ "Small metric specification "},
    {MDC_ATTRIBUTE_UNIT_CODE, _INT8_P_CASTING_ "Unit code"},
    {MDC_ATTRIBUTE_VALUE_MAP, _INT8_P_CASTING_ "Value map"},
    {MDC_MASS_BODY_ACTUAL, _INT8_P_CASTING_ "Body Weight"},
    {MDC_LENGTH_BODY_ACTUAL, _INT8_P_CASTING_ "Body Length"},
    {MDC_RATIO_MASS_BODY_LENGTH_SQUARE, _INT8_P_CASTING_ "BMI"},
    {MDC_DIM_PERCENT, _INT8_P_CASTING_ "%"},
    {MDC_DIM_KILOGRAM, _INT8_P_CASTING_ "kg"},
    {MDC_DIM_MINUTE, _INT8_P_CASTING_ "min"},
    {MDC_DIM_HOUR, _INT8_P_CASTING_ "h"},
    {MDC_DIM_DAY, _INT8_P_CASTING_ "d"},
    {MDC_DIM_DEGREE_C, _INT8_P_CASTING_ "degrC"},
    {MDC_DIM_KG_PER_M_SQUARE, _INT8_P_CASTING_ "kg/m2"}};

/* Nomenclature partition table */
const partition_ascii_struct_t g_partitionAsciiTable[] = {
    {MDC_PARTITION_OBJECT, _INT8_P_CASTING_ "Object Infrastructure"},
    {MDC_PARTITION_SCADA, _INT8_P_CASTING_ "SCADA"},
    {MDC_PARTITION_DIMENSION, _INT8_P_CASTING_ "Dimension"},
    {MDC_PARTITION_INFRASTRUCTURE, _INT8_P_CASTING_ "Infrastructure"},
    {MDC_PARTITION_PHD_DISEASE_MANAGEMENT, _INT8_P_CASTING_ "Disease Mgmt"},
    {MDC_PARTITION_PHD_HEALTH_FITNESS, _INT8_P_CASTING_ "H&F Set"},
    {MDC_PARTITION_PHD_AGING_INDEPENDENTLY, _INT8_P_CASTING_ "Aging Independently"},
    {MDC_PARTITION_RETURN_CODE, _INT8_P_CASTING_ "Return Codes"},
    {MDC_PARTITION_EXTERNAL_NOMENCLATURE, _INT8_P_CASTING_ "Ext. Nomenclature"}};

const uint16_t g_nomAsciiCount       = sizeof(g_nomenclatureAsciiTable) / sizeof(g_nomenclatureAsciiTable[0U]);
const uint16_t g_partitionAsciiCount = sizeof(g_partitionAsciiTable) / sizeof(g_partitionAsciiTable[0U]);
