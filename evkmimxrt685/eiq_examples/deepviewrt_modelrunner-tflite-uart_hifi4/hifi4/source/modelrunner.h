#ifndef TENSORFLOW_LITE_MICRO_MAIN_FUNCTIONS_H_
#define TENSORFLOW_LITE_MICRO_MAIN_FUNCTIONS_H_



#include "fsl_debug_console.h"
#include <inttypes.h>
#include <stdlib.h>
#include "fsl_iap.h"
struct nn_server {
    const char* model_name;
    char* params;
    size_t      model_size;
    char*       json_buffer;
    size_t      json_size;
    int*   input_dims_data;
    struct {
        int64_t run;
        int64_t decode;
        int64_t input;
        int64_t output;
    } timing;
    struct {
        int32_t num_outputs;
        char* name[256];
        char* type[256];
        int64_t timing[256];
        int32_t index[16];
        size_t bytes[16];
        char* data[16];
        char data_type[16][20];
        int32_t outputs_size;
        int* shape_data[16];
        int32_t shape_size[16];
        float scale[16];
        int32_t zero_point[16];
    } output;
    struct {
        char* name[16];
        size_t bytes[16];
        char* data[16];
        char data_type[16][20];
        const int32_t* shape_data[16];
        int32_t shape_size[16];
        int32_t inputs_size;
        float scale[16];
        int32_t zero_point[16];
        char* input_data[16];
    } input;
    uint8_t* image_upload_data;
    char* model_upload;
    int inference_count;
    bool model_flash_load;

    int64_t run_ns;
};
AT_QUICKACCESS_SECTION_CODE(status_t BOARD_FlexspiInit(uint32_t instance,
                                                       flexspi_nor_config_t *config,
                                                       serial_nor_config_option_t *option));

typedef struct nn_server NNServer;

int cmd_router(char* cmd, NNServer* server);
int modelrunner();

int64_t os_clock_now();

#endif
