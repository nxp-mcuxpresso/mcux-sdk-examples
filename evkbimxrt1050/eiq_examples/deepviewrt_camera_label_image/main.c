/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "board_init.h"
#include "demo_config.h"
#include "image.h"
#include "image_utils.h"
#include "timer.h"
#include "eiq_video_worker.h"
#include "deepview_rt.h"
#include "deepview_ops.h"
#ifdef EIQ_GUI_PRINTF
#include "chgui.h"
#endif

#define IMAGE_WIDTH 160
#define IMAGE_HEIGHT 160
#define IMAGE_CHANNELS 3

/* DeepView mempool and cache size definitions. */
#define CACHE_SIZE 128 * 1024
/* #define MEMPOOL_SIZE 7 * 1024 * 1024 */
#define MEMPOOL_SIZE 10 * 1024 * 1024
#ifdef LOAD_MODEL_TO_SDRAM
#define MEMBLOB_SIZE 8 * 1024 * 1024
#endif

static uint8_t s_imageData[IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_CHANNELS];

///* RTM Model Objects */
//extern const unsigned char deepview_rtm_start;
//extern const unsigned char deepview_rtm_end;

#if defined ( __ICCARM__ )    /* for iar toolchain */
/* DeepViewRT Model definition from IAR raw-binary-image */
extern const unsigned char deepview_rtm_start[];
#else
/* DeepViewRT Model definition from model.S */
extern const unsigned char deepview_rtm_start;
extern const unsigned char deepview_rtm_end;
#endif


uint8_t *cache = (uint8_t*)(0x20000000); //Cache in DTCM;
//__attribute__ ((section(".bss" ".$SRAM_DTC" )))  uint8_t cache[CACHE_SIZE] __attribute__((aligned(32)));
__attribute__ ((section(".bss" ".$BOARD_SDRAM" ))) uint8_t mempool[MEMPOOL_SIZE] __attribute__((aligned(32)));
#ifdef LOAD_MODEL_TO_SDRAM
__attribute__ ((section(".bss" ".$BOARD_SDRAM" ))) uint8_t memblob[MEMBLOB_SIZE] __attribute__((aligned(32)));
#endif

void rgb_normalization(float *output, int n, const uint8_t *input)
{
    for (int i = 0; i < n; i++) {
        output[i] = ((float)input[i])/255.0;
    }
}

int64_t os_clock_now()
{
	return TIMER_GetTimeInUS();
}

int main()
{
    BOARD_Init();
    TIMER_Init();

    printf("==========================================================================\r\n");
    printf("                    DeepviewRT camera label image Demo\r\n");
    printf("===========================================================================\r\n");

    /**
     * The model and model_size will be setup at startup based on the model_rtm_start
     * and model_rtm_end variables from the model.S file.
     */
#if defined ( __ICCARM__ )   /* for iar toolchain */
    const uint8_t *model = deepview_rtm_start;
    int model_size = 3 * 1024 * 1024;
#else
    const uint8_t *model_end = &deepview_rtm_end;
    const uint8_t *model = &deepview_rtm_start;
    int model_size = model_end - model;
#endif
    if (model_size < 1) {
        printf("[ERROR] invalid model_size (%d) verify model.S implementation.\r\n", model_size);
        return EXIT_FAILURE;
    }

    NNContext *context = nn_context_init(NULL,
                                         MEMPOOL_SIZE, mempool,
                                         CACHE_SIZE, cache);
    if (!context) {
        printf("[ERROR] insufficient memory to create context\r\n");
        return EXIT_FAILURE;
    }

#ifdef LOAD_MODEL_TO_SDRAM
    if(model_size < MEMBLOB_SIZE){
        memcpy(memblob,model,model_size);
        model = (const uint8_t*)memblob;
        printf("Model loaded to SDRAM...\r\n");
    } else {
        printf("Model too large (%d) for SDRAM buffer (%d)\r\n", model_size, MEMBLOB_SIZE);
    }
#endif

    NNError err = nn_context_model_load(context, (size_t) model_size, model);
    if (err) {
        printf("[ERROR] failed to load model: %s\r\n", nn_strerror(err));
        return EXIT_FAILURE;
    }

	NNTensor *input = nn_context_tensor(context, "input");
	if (!input) {
		printf("[ERROR] failed to retrieve mobilenet input tensor\r\n");
	}

	NNTensor *output = nn_context_tensor(context, "output");
	if (!output) {
		printf("failed\n[ERROR] failed to retrieve mobilenet output tensor\r\n");
	}

    int result = 0;
    float score = 0;

    while (1)
    {
        if (IMAGE_GetImage(s_imageData, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_CHANNELS) != kStatus_Success)
        {
          printf("Failed retrieving input image\r\n");
          for (;;) {}
        }

        float *input_map = nn_tensor_maprw(input);
        if (!input_map) {
            printf("[ERROR] no memory for input tensor\n");
            for (;;) {}
        }
        rgb_normalization(input_map, IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_CHANNELS, s_imageData);
        nn_tensor_unmap(input);
		int64_t start = TIMER_GetTimeInUS();
        err = nn_context_run(context);
		int64_t run_ms = (TIMER_GetTimeInUS() - start)/1000;
        if (err) {
            printf("[ERROR] failed to run mobilenet model: %s\r\n", nn_strerror(err));
        }

        nn_argmax(output, &result, &score, sizeof(score));

        printf("Label index = %d, score = %.3f, runtime: %lld\r\n", result, score, run_ms);

        char *label="no label";

        label= (char *) nn_model_label(model, result);

        printf("\t%s - %s - %d \r\n",
               score > 0.3f ? "MATCH" : "NO MATCH",
               label,
               result);
#ifdef EIQ_GUI_PRINTF
        GUI_PrintfToBuffer(GUI_X_POS, GUI_Y_POS, "Detected: %.20s , Score: %f", score > 0.3f ? label : "NO MATCH", score);
#endif
    }
}
