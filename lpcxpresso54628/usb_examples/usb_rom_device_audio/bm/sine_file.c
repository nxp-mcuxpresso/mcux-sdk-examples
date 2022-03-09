/*
 * @brief RAW PCM sine wave file
 *
 * @note
 * Copyright  2016, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <stdint.h>
#include "sine_file.h"

static const int16_t sine_wave[] = {
    0,      1454,   2903,   4340,   5761,   7158,   8528,   9863,   11160,  12413,  13616,  14766,  15858,
    16887,  17849,  18741,  19559,  20300,  20961,  21539,  22032,  22438,  22755,  22983,  23120,  23166,
    23120,  22983,  22755,  22438,  22032,  21539,  20961,  20300,  19559,  18741,  17849,  16887,  15858,
    14766,  13616,  12413,  11160,  9863,   8528,   7158,   5761,   4340,   2903,   1454,   0,      -1454,
    -2903,  -4340,  -5761,  -7158,  -8528,  -9863,  -11160, -12413, -13616, -14766, -15858, -16887, -17849,
    -18741, -19559, -20300, -20961, -21539, -22032, -22438, -22755, -22983, -23120, -23166, -23120, -22983,
    -22755, -22438, -22032, -21539, -20961, -20300, -19559, -18741, -17849, -16887, -15858, -14766, -13616,
    -12413, -11160, -9863,  -8528,  -7158,  -5761,  -4340,  -2903,  -1454,
};
static const uint16_t sine_wave_ct = sizeof(sine_wave) / sizeof(int16_t);
static uint16_t idx                = 0;

uint32_t get_sine_data(void)
{
    uint32_t sample;
    int16_t l_buff, r_buff;

    l_buff = r_buff = sine_wave[idx++] / 28; //	get the sample (divider gives about -3dB)
    sample          = (uint32_t)(((uint16_t)r_buff << 16) | (uint16_t)l_buff); //	place the data in the buffer
    if (idx == sine_wave_ct)
        idx = 0; //	reset the index
    return sample;
}

//
//	End of: sine_file.c
//
