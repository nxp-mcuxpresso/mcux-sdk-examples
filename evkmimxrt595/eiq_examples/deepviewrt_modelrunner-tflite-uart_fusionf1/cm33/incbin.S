/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSP_IMAGE_COPY_TO_RAM
#define DSP_IMAGE_COPY_TO_RAM 1
#endif

#if DSP_IMAGE_COPY_TO_RAM

#if defined(__GNUC__) || defined(__ARMCC_VERSION)

        .section .dspcode , "ax" @progbits @preinit_array
        .global dsp_reset_image_start
        .type dsp_reset_image_start, %object
        .align 2
dsp_reset_image_start:
        .incbin "dsp_reset_release.bin"
        .global dsp_reset_image_end
        .type dsp_reset_image_end, %object
dsp_reset_image_end:
        .global dsp_text_image_start
        .type dsp_text_image_start, %object
        .align 2
dsp_text_image_start:
        .incbin "dsp_text_release.bin"
        .global dsp_text_image_end
        .type dsp_text_image_end, %object
dsp_text_image_end:
        .global dsp_data_image_start
        .type dsp_data_image_start, %object
        .align 2
dsp_data_image_start:
        .incbin "dsp_data_release.bin"
        .global dsp_data_image_end
        .type dsp_data_image_end, %object
dsp_data_image_end:
        .global dsp_reset_image_size
        .type dsp_reset_image_size, %object
        .align 2
dsp_reset_image_size:
        .int  dsp_reset_image_end - dsp_reset_image_start
        .global dsp_text_image_size
        .type dsp_text_image_size, %object
        .align 2
dsp_text_image_size:
        .int  dsp_text_image_end - dsp_text_image_start
        .global dsp_data_image_size
        .type dsp_data_image_size, %object
        .align 2
dsp_data_image_size:
        .int  dsp_data_image_end - dsp_data_image_start
        .end

#endif

#endif
