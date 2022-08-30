#include "cr_section_macros.h"
#include "lwip/sys.h"
#include "modelrunner.h"

#define TEST_IMAGE_INC 0
#define STATIC_JSON 1
#define HTTP_BUFSIZE 2 * 1024 * 1024
__BSS(BOARD_SDRAM) static char http_buffer[HTTP_BUFSIZE];

#if STATIC_JSON
#define JSON_BUFSIZE 4 * 1024 * 1024
__BSS(BOARD_SDRAM) static char nn_json_buffer[JSON_BUFSIZE];
#endif

// Statically allocate memory for constant weights (model weights) and initialize.
__DATA(BOARD_SDRAM)
GLOW_MEM_ALIGN(MODEL_MEM_ALIGN)
uint8_t constantWeight[MODEL_CONSTANT_MEM_SIZE] = {
#include "model.weights.txt"
};

// Statically allocate memory for mutable weights (model input/output data).
__BSS(BOARD_SDRAM)
GLOW_MEM_ALIGN(MODEL_MEM_ALIGN)
uint8_t mutableWeight[MODEL_MUTABLE_MEM_SIZE];

// Statically allocate memory for activations (model intermediate results).
// Use DTCM if possible
#if MODEL_ACTIVATIONS_MEM_SIZE < 262144
__BSS(BOARD_SRAM_DTC_cm7)
GLOW_MEM_ALIGN(MODEL_MEM_ALIGN)
uint8_t activations[MODEL_ACTIVATIONS_MEM_SIZE];
#else
__BSS(BOARD_SDRAM)
GLOW_MEM_ALIGN(MODEL_MEM_ALIGN)
uint8_t activations[MODEL_ACTIVATIONS_MEM_SIZE];
#endif

// Allocate buffer for input data. This buffer contains the input image
// pre-processed and serialized as text to include here.
#if TEST_IMAGE_INC
__DATA(BOARD_SDRAM)
GLOW_MEM_ALIGN(MODEL_MEM_ALIGN)
uint8_t imageData[INPUT_SIZE] = {
#include "bald_eagle.inc"
};
#define OUTPUT_CLASS 1001
#endif

//Structure that describes the model and JSON buffers
//Global so it lives in SRAM_OC
NNServer nnserver;

//Input and output shape arrays
const char* modelName = MODEL_NAME;
int32_t input_shape[NUM_INPUT_DIMS] = INPUT_SHAPE;
const char* inputName = INPUT_NAME;
int32_t output_shape[NUM_OUTPUT_DIMS] = OUTPUT_SHAPE;
const char* outputName = OUTPUT_NAME;
#if NUM_OUTPUTS > 1
    int32_t output1_shape[NUM_OUTPUT1_DIMS] = OUTPUT1_SHAPE;
    const char* output1Name = OUTPUT1_NAME;
#endif
#if NUM_OUTPUTS > 2
    int32_t output2_shape[NUM_OUTPUT2_DIMS] = OUTPUT2_SHAPE;
    const char* output2Name = OUTPUT2_NAME;
#endif
#if NUM_OUTPUTS > 3
    int32_t output3_shape[NUM_OUTPUT3_DIMS] = OUTPUT3_SHAPE;
    const char* output3Name = OUTPUT3_NAME;
#endif

//Provide weak defintions for strdup and strnlen
#if (defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__ICCARM__))
/* Copies a string to a new allocated string  */
__attribute__((weak))
char* strdup (const char *s)
{
  size_t len = strlen (s) + 1;
  void *new = malloc (len);
  if (new == NULL)
    return NULL;
  return (char *) memcpy (new, s, len);
}

/* Returns the amount of characters in a string without terminating zero.  */
__attribute__((weak))
size_t strnlen (const char *s, size_t maxlen)
{
  const char *p = s;
  /* We don't check here for NULL pointers.  */
  for (;*p != 0 && maxlen > 0; p++, maxlen--)
    ;
  return (size_t) (p - s);
}
#endif

//provide only for keil mdk
#if defined(__ARMCC_VERSION)
__attribute__((weak))
size_t __aeabi_read_tp(void)
{
  return 0;
}
#endif
//Modelrunner task that starts http server
//Inference is run once before http init
void modelrunner_task(void* arg)
{
	sys_msleep(50);
    int                 err;
    int                 listener;
    struct sockaddr_in  addr;
    void*               buf    = NULL;
    size_t              bufsiz = HTTP_BUFSIZE;

    // Timer variables.
    uint32_t start_time, stop_time;
    uint32_t duration_ms;

    //One place to change model related params - http server will use nnserver
    memset(&nnserver,0,sizeof(NNServer));
    nnserver.activation = activations;
    nnserver.weights    = constantWeight;
    nnserver.mutable    = mutableWeight;
    nnserver.input      = GLOW_GET_ADDR(mutableWeight, MODEL_input);
    nnserver.output[0]  = GLOW_GET_ADDR(mutableWeight, MODEL_output);
    nnserver.inference  = model;
    nnserver.model_size = MODEL_CONSTANT_MEM_SIZE;
    nnserver.mem_size = MODEL_ACTIVATIONS_MEM_SIZE;
    nnserver.model_name = modelName;
    //Input info
    nnserver.input_shape = input_shape;
    nnserver.input_type  = INPUT_TYPE;
    nnserver.input_dims  = NUM_INPUT_DIMS;
    nnserver.input_scale = INPUT_SCALE;
    nnserver.input_zero  = INPUT_ZEROP;
    nnserver.input_name  = inputName;
    //nnserver.input_NCHW  = INPUT_NCHW;
    nnserver.input_NCHW  = ((input_shape[1] <= 3) && (input_shape[2] > 4));
    //Output info
    nnserver.output_shape[0] = output_shape;
    nnserver.output_dims[0]  = NUM_OUTPUT_DIMS;
    nnserver.num_outputs     = NUM_OUTPUTS;
    nnserver.output_type[0]  = OUTPUT_TYPE;
    nnserver.output_name[0]  = outputName;
#if NUM_OUTPUTS > 1
    nnserver.output_shape[1] = output1_shape;
    nnserver.output_dims[1]  = NUM_OUTPUT1_DIMS;
    nnserver.output_type[1]  = OUTPUT1_TYPE;
    nnserver.output[1]       = GLOW_GET_ADDR(mutableWeight, MODEL_output1);
    nnserver.output_name[1]  = output1Name;
#endif
#if NUM_OUTPUTS > 2
    nnserver.output_shape[2] = output2_shape;
    nnserver.output_dims[2]  = NUM_OUTPUT2_DIMS;
    nnserver.output_type[2]  = OUTPUT2_TYPE;
    nnserver.output[2]       = GLOW_GET_ADDR(mutableWeight, MODEL_output2);
    nnserver.output_name[2]  = output2Name;
#endif
#if NUM_OUTPUTS > 3
    nnserver.output_shape[3] = output3_shape;
    nnserver.output_dims[3]  = NUM_OUTPUT3_DIMS;
    nnserver.output_type[3]  = OUTPUT3_TYPE;
    nnserver.output[3]       = GLOW_GET_ADDR(mutableWeight, MODEL_output3);
    nnserver.output_name[3]  = output3Name;
#endif


#if TEST_IMAGE_INC
    // Produce input data for bundle.
    // Copy the pre-processed image data into the bundle input buffer.
    memcpy(nnserver.input, imageData, sizeof(imageData));
#endif

    // Perform inference and compute inference time.
    start_time = sys_now();
    int status = nnserver.inference(nnserver.weights, nnserver.mutable, nnserver.activation);
    stop_time = sys_now();
    duration_ms = (stop_time - start_time);
    nnserver.timing.run = ((int64_t)duration_ms * (int64_t)1000000);
    PRINTF("\r\n************************************************\r\n");
    PRINTF("Model Inference time = %d (ms), return_status = %d\r\n", duration_ms, status);

#if TEST_IMAGE_INC
    // Get classification top1 result and confidence.
    float *out_data = (float*)(nnserver.output[0]);
    float max_val = 0.0;
    uint32_t max_idx = 0;
    for(int i = 0; i < OUTPUT_CLASS; i++) {
      if (out_data[i] > max_val) {
        max_val = out_data[i];
        max_idx = i;
      }
    }
    // Print classification results.
    PRINTF("Top1 class = %lu\r\n", max_idx);
    PRINTF("Confidence = 0.%03u\r\n",(int)(max_val*1000));
#endif
    PRINTF("************************************************\r\n");

#if STATIC_JSON
    nnserver.json_buffer = nn_json_buffer;
    nnserver.json_size = JSON_BUFSIZE;
#endif

    struct http_route routes[] = {{"/", 0, (void *)(&nnserver), nn_server_http_handler},
                                  {NULL, 0, NULL, NULL}};

    for (struct http_route* route = routes; route->path != NULL; ++route) {
        route->path_len = strlen(route->path);
    }

    //Static http buffer allocation - 2MB
    buf = (void *)http_buffer;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        PRINTF("failed to create listener socket: %s\r\n",
                strerror(errno));
        goto finish;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(MODELRUNNER_DEFAULT_PORT);

    err = bind(listener, (struct sockaddr*) &addr, sizeof(addr));
    if (err) {
        PRINTF("failed to bind listener socket: %s\r\n",
                strerror(errno));
        goto finish;
    }

    err = listen(listener, 16);
    if (err) {
        PRINTF("failed to listen on listener socket: %s\r\n",
                strerror(errno));
        goto finish;
    }

    PRINTF("Initialized GLOW modelrunner server at port %d\r\n",MODELRUNNER_DEFAULT_PORT);

    while (1) {
        size_t offset = 0;
        int    sock   = accept(listener, NULL, NULL);
        if (sock == -1) {
            PRINTF("Invalid socket: %s\r\n",strerror(errno));
            continue;
        }

        err = http_handler(sock, buf, bufsiz, &offset, routes);
        if (err) {
            PRINTF("http handler failed: %s\r\n", strerror(errno));
            closesocket(sock);
            continue;
        }
        //closesocket(sock);
    }

finish:
    vTaskDelete(NULL);

}
