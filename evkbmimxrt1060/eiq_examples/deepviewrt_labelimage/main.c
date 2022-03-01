/**
* Copyright 2018 Au-Zone Technologies
* All rights reserved.
*
* Software that is described herein is for illustrative purposes only which
* provides customers with programming information regarding the DeepViewRT
* library. This software is supplied "AS IS" without any warranties of any
* kind, and Au-Zone Technologies and its licensor disclaim any and all
* warranties, express or implied, including all implied warranties of
* merchantability, fitness for a particular purpose and non-infringement of
* intellectual property rights.  Au-Zone Technologies assumes no responsibility
* or liability for the use of the software, conveys no license or rights under
* any patent, copyright, mask work right, or any other intellectual property
* rights in or to any products. Au-Zone Technologies reserves the right to make
* changes in the software without notification. Au-Zone Technologies also makes
* no representation or warranty that such application will be suitable for the
* specified use without further testing or modification.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation requires authorization from Au-Zone Technologies which is
* available free of charge by visiting https://embeddedml.com/deepview-samples
*/

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "clock_config.h"
#include "board_init.h"
#include "board.h"

#include <cr_section_macros.h>

#include "deepview_rt.h"
#include "deepview_ops.h"

/**
 * If LAYER_TIMING is set to 1 the main loop will dump the
 * timing information for all non-constant layers.
 */
#define LAYER_TIMING 0

/**
 * If LOAD_MODEL_TO_SDRAM is set to 1 then the model will be
 * copied from FLASH into SDRAM before being loaded.
 */
#define LOAD_MODEL_TO_SDRAM 0

/**
 * The cache is used by DeepViewRT to optimize certain internal
 * loops.  It is optional but if used should be placed in the
 * fastest available memories, in this case we use the SRAM_DTC.
 */
#if defined(CPU_MIMXRT1176DVMAA_cm7)
#define CACHE_SIZE 256 * 1024
#else
/* 128K cache size is workable for default model */
#define CACHE_SIZE 128 * 1024
#endif

/**
 * The mempool holds the intermediate buffers for evaluating the
 * model.  This buffer can be multiple megabytes in size and therefore
 * should be stored in the SDRAM.  You may adjust this size if your
 * particular model requires more or less memory as reported by the
 * conversion tool.  If insufficient memory is provided an error will
 * be reported by nn_context_model_load().
 */
/**
 *The maximum runtime memroy size,
 *adjust it according model's runtime size and board's SDRAM size.
 */
#define MEMPOOL_SIZE 5 * 1024 * 1024

#if defined ( __ICCARM__ )    /* for iar toolchain */
extern const unsigned char model_rtm_start[];
extern const unsigned char sample_img_start[];
#else
/* DeepViewRT Model definition from model.S */
extern const unsigned char model_rtm_start;
extern const unsigned char model_rtm_end;

/* Sample image definition from model.S */
extern const unsigned char sample_img_start;
extern const unsigned char sample_img_end;
#endif

/**
 * The DeepViewRT Cache buffer stored in SRAM_DTC for maximum performance.
 */
//__BSS(SRAM_DTC_cm7) uint8_t cache[CACHE_SIZE] __attribute__((aligned(32)));
uint8_t *cache = (uint8_t*)(0x20000000); //Cache in DTCM; works for 1170 and 106x

/**
 * The DeepViewRT Memory Pool buffer holds intermediate buffers and is
 * stored in SDRAM for maximum storage space.
 */
__BSS(BOARD_SDRAM) uint8_t mempool[MEMPOOL_SIZE] __attribute__((aligned(32)));

#if LOAD_MODEL_TO_SDRAM
/**
 * MEMBLOB_SIZE needs to be at least as large as the RTM model file.
 */
#define MEMBLOB_SIZE 10 * 1024 * 1024
__BSS(BOARD_SDRAM) uint8_t memblob[MEMBLOB_SIZE] __attribute__((aligned(32)));
#endif

/**
 * SysTick_Handler triggers every millisecond and increments the
 * g_systickCounter.
 */
volatile int32_t g_systickCounter = 0;

void SysTick_Handler(void)
{
	g_systickCounter++;
}

/**
 * This symbol is required by DeepViewRT for internal time keeping and MUST
 * return a 64-bit signed integer of continuous nanoseconds.  The epoch is
 * not important but the counter should never reset during model evaluation.
 *
 * This sample has no overflow protection so after 2^31 milliseconds it will
 * wrap around.  This will generally not be a problem though production software
 * should have more intelligent tracking.
 */
int64_t os_clock_now()
{
    return ((int64_t) g_systickCounter) * (int) 1e6;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_Init();

    /**
     * Initialize the SysTick to fire every milliseconds.  If this is adjusted the os_clock_now()
     * must also be adjusted accordingly to continue reporting nanoseconds.
     */
	if(SysTick_Config(SystemCoreClock / 1000U)) {
        while(1) {}
    }

	PRINTF("==========================================================================\r\n");
	PRINTF("                    DeepviewRT  Demo\r\n");
	PRINTF("===========================================================================\r\n");
	//PRINTF("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
    //printf("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
	//PRINTF("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Semc));

    /**
     * The model and model_size will be setup at startup based on the model_rtm_start
     * and model_rtm_end variables from the model.S file.
     */
#if defined ( __ICCARM__ )   /* for iar toolchain */
    const uint8_t *model = model_rtm_start;
    int model_size = 3 * 1024 * 1024;
#else
    const uint8_t *model_end = &model_rtm_end;
    const uint8_t *model = &model_rtm_start;
    int model_size = model_end - model;
#endif
    if (model_size < 1) {
    	PRINTF("[ERROR] invalid model_size (%d) verify model.S implementation.\r\n", model_size);
    	return EXIT_FAILURE;
    }

    /**
	 * Just like model above, but from sample_img_start/sample_img_end.
	 */
#if defined ( __ICCARM__ )    /* for iar toolchain */
	const uint8_t *sample_image = sample_img_start;
	int sample_image_size = 200 * 1024;
#else
	const uint8_t *image_end = &sample_img_end;
	const uint8_t *sample_image = &sample_img_start;
	int sample_image_size = image_end - sample_image;
#endif
    if (sample_image_size < 1) {
    	PRINTF("[ERROR] invalid sample_image_size (%d) verify model.S implementation.\r\n", sample_image_size);
    	return EXIT_FAILURE;
    }
	/**
	 * The NNContext structure holds runtime model data including the memory pool
	 * and optional cache.  The first parameter is for the engine which is not used
	 * on MCU devices, but the same API is provided across all platforms.
	 *
	 * If MEMPOOL_SIZE is 0 then each layer's tensor will be allocated on the heap
	 * using malloc instead of using the optimized memory map.  If MEMPOOL_SIZE is
	 * greater than 0 but mempool is NULL then the pool will be allocated from the
	 * heap and the optimized memory map WILL be used.
	 *
	 * If CACHE_SIZE is 0 then no cache will be used, convolutions especially will
	 * take significantly longer.  If CACHE_SIZE is greater than 0 but cache is
	 * NULL then it will be allocated on the heap.  This will provide better performance
	 * but will depend on the performance of heap memory, if this is SDRAM the model
	 * will take approximately 10x longer than if the cache is located in SRAM_DTC.
	 *
	 * If the heap is too small for the configuration context will be NULL.
	 */
	NNContext *context = nn_context_init(NULL,
										 MEMPOOL_SIZE, mempool,
										 CACHE_SIZE, cache);
	if (!context) {
		PRINTF("[ERROR] insufficient memory to create context\r\n");
		return EXIT_FAILURE;
	}

#if LOAD_MODEL_TO_SDRAM
	if(model_size < MEMBLOB_SIZE){
        memcpy(memblob,model,model_size);
        model = (const uint8_t*)memblob;
        PRINTF("Model loaded to SDRAM...\r\n");
	} else {
		PRINTF("Model too large (%d) for SDRAM buffer (%d)\r\n", model_size, MEMBLOB_SIZE);
	}
#endif

	/**
	 * Loads the model into the context.  If the model is invalid because of corruption
	 * or alignment an error will be returned.  If the provided MEMPOOL_SIZE is insufficient
	 * an error will also be returned.  Many of these internal errors will also be logged
	 * through stderr.
	 */
	NNError err = nn_context_model_load(context, (size_t) model_size, model);
	if (err) {
		PRINTF("[ERROR] failed to load model: %s\r\n", nn_strerror(err));
		return EXIT_FAILURE;
    }

	/**
	 * Acquire the input tensor, will be used for loading the sample image into the model.
	 */
    size_t input_index = (size_t)(nn_model_inputs(model,NULL)[0]);
	NNTensor *input = nn_context_tensor_index(context, input_index);
	if (!input) {
		PRINTF("[ERROR] failed to retrieve input tensor\r\n");
		return EXIT_FAILURE;
	}

	/**
	 * Set imgproc mode for float input type.  For quantized models imgproc is not used.
	 */
    uint32_t proc = nn_tensor_type(input) == NNTensorType_F32
    			  ? NN_IMAGE_PROC_SIGNED_NORM
    			  : 0;

	/**
	 * Acquire the output tensor, will be used for reading out results of model evaluation.
	 */
    size_t output_index = (size_t)(nn_model_outputs(model,NULL)[0]);
	NNTensor *output = nn_context_tensor_index(context, output_index);
	if (!output) {
		PRINTF("[ERROR] failed to retrieve output tensor\r\n");
		return EXIT_FAILURE;
	}

    for(int count = 0; count < 10; count++)
    {
    	/**
    	 * The nn_tensor_load_image_ex function will load the image data and attempt to
    	 * decode it.  The function supports PNG and JPEG images and the format is discovered
    	 * by reading the buffers headers automatically.  If this operation fails an error
    	 * is returned.
    	 *
    	 * The final proc parameter of the _ex version of this function allows for preprocessing
    	 * to be applied to the image as part of loading it into the input.  This is useful for
    	 * models which were trained with specific preprocessing steps but did not include them
    	 * into the graph.  A common case is normalization (x/255) and image whitening or standardization
    	 * which are often not included in the graph but must be applied to get accurate results.
    	 *
    	 * proc==0 performs no pre-processing.  proc&1 will perform normalization (x/255) and
    	 * proc&2 will perform whitening.  It would not be common for proc&3 to be requested.
    	 */
    	int64_t start = os_clock_now();
		err = nn_tensor_load_image_ex(input, sample_image, (size_t) sample_image_size, proc);
		int64_t decode_ns = os_clock_now() - start;

		if (err) {
			PRINTF("[ERROR] failed to load image: %s\r\n", nn_strerror(err));
		}

		/**
		 * The nn_context_run function performs the actual model evaluation.  This causes all layers
		 * in the graph to be evaluated.  If any error happens on any layer this function will return
		 * an error and more details might be reported to stderr depending on the cause.
		 *
		 * A common warning can be reported when insufficient cache is provided leading to performance
		 * degradations.  These do not affect the accuracy of the results but do translate to longer
		 * inference times.
		 */
		start = os_clock_now();
		err = nn_context_run(context);
		int64_t run_ns = os_clock_now() - start;

		if (err) {
			PRINTF("[ERROR] failed to run model: %s\r\n", nn_strerror(err));
		}

		/**
		 * Classification models are typically arranged in a one-hot encoding.  This means the output is
		 * a vector representing the known labels, the largest element in this vector represents the
		 * inferred label.  This "argmax" can be used as an index into our known labels to report a text
		 * label result.  If a label is not provided we instead report the argmax value, this can also
		 * happen if labels were provided but argmax was beyond the provided labels.
		 */
		int argmax;
		float softmax;
		nn_argmax(output, &argmax, &softmax, sizeof(softmax));

		const char *label = nn_model_label(model, argmax);
		char argmax_string[12];
		snprintf(argmax_string, sizeof(argmax_string), "%d", argmax);
        int decode_ms = (int)(decode_ns/1e6);
        int run_ms = (int)(run_ns/1e6);
		PRINTF("\tResult: %s (%d%%) -- decode: %d ms runtime: %d ms \r\n",
			   label ? label : argmax_string,
			   (int) (softmax * 100),
			   decode_ms,
			   run_ms);
    }

#if LAYER_TIMING
	/**
	 * This loop iterates all the layers in the model to query the output tensor.
	 * The tensor is in turn queried for timing information.  Finally we print out
	 * the layer index, type, timing, and name.  We ignore layers with no timing
	 * information to reduce console output.
	 */
	for (size_t i = 0; i < nn_model_layer_count(model); i++) {
		NNTensor *tensor = nn_context_tensor_index(context, i);
		int64_t tensor_ns = nn_tensor_time(tensor);

		// Ignore layers with 0 time, such as constant layers.
		if (tensor_ns == 0) continue;
        int tensor_ms = (int)(tensor_ns/1e6);
		const char *name = nn_model_layer_name(model, i);
		const char *type = nn_model_layer_type(model, i);

		PRINTF("%d: %s [%d ms] %s\r\n", i, type, tensor_ms, name);
	}
#endif
	return 0;
}
