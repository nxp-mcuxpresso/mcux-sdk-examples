/*!\file otopcode.h
 *\brief This file provides interface to get OpenThread comands corrspoding opcode.
 */
/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __OT_OPCODE_H__
#define __OT_OPCODE_H__

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*                                  Function prototypes                       */
/* -------------------------------------------------------------------------- */

/**
* Get OpenThread commands corresponding opcode.
* This function search OT command list to find command's opcode.
*
* \param[in] userinputcmd user input OT command.
* \param[in] otcmdlen OT command length.
* \return opcode of the command.
*/
int8_t ot_get_opcode(uint8_t *userinputcmd, uint8_t otcmdlen);

#endif /* __OT_OPCODE_H__ */
