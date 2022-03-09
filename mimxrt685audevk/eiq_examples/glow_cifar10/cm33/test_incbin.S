/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(__GNUC__)

        .section .dspcode , "ax" @progbits @preinit_array

        .global input_section
        .type input_section, %object
        .align 4
input_section:
        .incbin "input.bin"

        .global weights_section
        .type weights_section, %object
        .align 6
weights_section:
        .incbin "model.weights.bin"

        .global output_section
        .type output_section, %object
        .align 4
output_section:
        .incbin "output.bin"

        .end

#endif /* defined(__GNUC__) */
