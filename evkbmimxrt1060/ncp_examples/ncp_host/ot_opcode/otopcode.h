/*!\file otopcode.h
 *\brief This file provoides interface to get ot opcode.
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
* Traverse the search command list to find the command.
*
* \param[in] userinputcmd user input ot command.
* \param[in] otcmdlen ot command length.
* \return command's opcode.
*/
int8_t ot_get_opcode(uint8_t *userinputcmd, uint8_t otcmdlen);

#endif /* __OT_OPCODE_H__ */
