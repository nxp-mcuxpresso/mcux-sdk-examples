/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FLEXSPI_OCTAL_FLASH_OPS_H_
#define _FLEXSPI_OCTAL_FLASH_OPS_H_
#include "app.h"

status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src);
status_t flexspi_nor_get_id(FLEXSPI_Type *base, uint8_t *vendorId);
status_t flexspi_nor_enable_octal_mode(FLEXSPI_Type *base);
status_t flexspi_nor_flash_init(FLEXSPI_Type *base, uint8_t *id);
status_t flexspi_nor_exec_op(FLEXSPI_Type *base,
                             uint32_t deviceAddr,
                             flexspi_port_t port,
                             flexspi_command_type_t cmdType,
                             uint8_t seqIndex,
                             uint8_t seqNumber,
                             uint32_t *data,
                             size_t dataSize);
status_t flexspi_nor_exec_op_with_edma(FLEXSPI_Type *base,
                             uint32_t deviceAddr,
                             flexspi_port_t port,
                             flexspi_command_type_t cmdType,
                             uint8_t seqIndex,
                             uint8_t seqNumber,
                             uint32_t *data,
                             size_t dataSize);
status_t flexspi_nor_write_enable(FLEXSPI_Type *base, uint32_t baseAddr);
status_t flexspi_nor_wait_bus_busy(FLEXSPI_Type *base);

#endif
