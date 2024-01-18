/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tf_benchmark.h"
#include "fsl_debug_console.h"
#include "app.h"

#ifdef USE_NPU
#include "tensorflow/lite/micro/kernels/neutron/neutron.h"
#endif

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_profiler.h"

#ifndef __XCC__
#include <cmsis_compiler.h>
#else
#define __ALIGNED(x) __attribute__((aligned(x)))
#endif

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
TfLiteTensor* output_t = nullptr;

static int kTensorArenaSize = 4;

#ifdef KTENSOR_ARENA_SIZE_MEM
constexpr int kTensorArenaSize_mem = KTENSOR_ARENA_SIZE_MEM;
constexpr int kTensorArenaSize_flash = KTENSOR_ARENA_SIZE_FLASH;
#else
constexpr int kTensorArenaSize_mem = 300 * 1024;
constexpr int kTensorArenaSize_flash = (300 + 1024)  * 1024;
#endif

class BProfiler : public tflite::MicroProfilerInterface {
public:
    explicit BProfiler(NNServer* server): server_(server){}

    uint32_t BeginEvent(const char* tag) {
        start_time_ = os_clock_now();
        TFLITE_DCHECK(tag != nullptr);
        event_tag_ = tag;
        return 0;
    }

    void EndEvent(uint32_t event_handle) {
        int64_t end_time = os_clock_now() - start_time_;
        server_->run_ns += end_time;
#ifdef DEBUG
        PRINTF("%s - %s took %u us\r\n", server_->output.name[server_->output.num_outputs], event_tag_, (int32_t)(end_time));
#endif
        server_->output.timing[server_->output.num_outputs] += end_time;
        server_->output.type[server_->output.num_outputs] = (char*)event_tag_;
        server_->output.num_outputs++;
	}

private:
    NNServer* server_;
    int64_t start_time_ = { 0 };
    const char* event_tag_ = { nullptr };
    TF_LITE_REMOVE_VIRTUAL_DELETE;
};

int Model_Setup(NNServer* server) {
    model = tflite::GetModel(server->model_upload);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        PRINTF("Model provided is schema version %d not equal "
               "to supported version %d.",
                model->version(), TFLITE_SCHEMA_VERSION);
        return kStatus_Fail;
    }
    auto* subgraphs = model->subgraphs();
    const tflite::SubGraph* subgraph = (*subgraphs)[0];
    auto tensors = subgraph->tensors();
    const flatbuffers::Vector<flatbuffers::Offset<tflite::Operator>>* operators = subgraph->operators();
    for (int i = 0; i < operators->size(); i++) {
        const tflite::Operator* cur_operator = (*operators)[i];
        auto* outputs = cur_operator->outputs();
        for (int j=0; j<outputs->size(); j++)
        {
            int idx = (int)(*outputs)[j];
            const tflite::Tensor* tensor = (*tensors)[idx];
            char* name = (char*)tensor->name()->c_str();
            server->output.name[i] =  name;
            break;
        }
    }

    server->input.inputs_size = subgraph->inputs()->size();
    for (int i=0; i < server->input.inputs_size; i++){
        int idx = (int)(*subgraph->inputs())[i];
        auto tensor = (*tensors)[idx];
        server->input.name[i] = (char*) tensor->name ()->c_str();
        server->input.shape_data[i] = tensor->shape()->data();
        server->input.shape_size[i] = tensor->shape()->size();
        
        switch (tensor->type()){
        case tflite::TensorType_FLOAT32:
            strcpy(server->input.data_type [i], "FLOAT32");
            server->input.bytes[i] = 4;
            break;
        case tflite::TensorType_UINT8:
            strcpy(server->input.data_type [i], "UINT8");
            server->input.bytes[i] = 1;
            break;
        case tflite::TensorType_INT8:
            strcpy(server->input.data_type [i], "INT8");
            server->input.bytes[i] = 1;
            break;
        case tflite::TensorType_INT16:
            strcpy(server->input.data_type [i], "INT16");
            server->input.bytes[i] = 2;
            break;
        default:
            break;
        }
    }

    server->output.outputs_size = subgraph->outputs()->size();
    for (int i = 0; i < server->output.outputs_size; i++)
    {
        int idx = (int)(*subgraph->outputs())[i];
        auto tensor = (*tensors)[idx];
        char* name = (char*) tensor->name ()->c_str();
        for (int j = operators->size()-1; j >= 0; j--){
            if ( strcmp (server->output.name[j], name) == 0){
                server->output.index[i] = j;
            }
        }
    }

    kTensorArenaSize = 4;
    server->inference_count = 1;
    Model_RunInference (server);
    return 0;
}

tflite::MicroOpResolver &MODEL_GetOpsResolver()
{
    static tflite::MicroMutableOpResolver<114> s_microOpResolver;
    if (s_microOpResolver.FindOp(tflite::BuiltinOperator_ZEROS_LIKE) != nullptr){
        return s_microOpResolver;
	}

    s_microOpResolver.AddAbs();
    s_microOpResolver.AddAdd();
    s_microOpResolver.AddAddN();
    s_microOpResolver.AddArgMax();
    s_microOpResolver.AddArgMin();
    s_microOpResolver.AddAssignVariable();
    s_microOpResolver.AddAveragePool2D();
    s_microOpResolver.AddBatchMatMul();
    s_microOpResolver.AddBatchToSpaceNd();
    s_microOpResolver.AddBroadcastArgs();
    s_microOpResolver.AddBroadcastTo();
    s_microOpResolver.AddCallOnce();
    s_microOpResolver.AddCast();
    s_microOpResolver.AddCeil();
    s_microOpResolver.AddCircularBuffer();
    s_microOpResolver.AddConcatenation();
    s_microOpResolver.AddConv2D();
    s_microOpResolver.AddCos();
    s_microOpResolver.AddCumSum();
    s_microOpResolver.AddDelay();
    s_microOpResolver.AddDepthToSpace();
    s_microOpResolver.AddDepthwiseConv2D();
    s_microOpResolver.AddDequantize();
    s_microOpResolver.AddDetectionPostprocess();
    s_microOpResolver.AddDiv();
    s_microOpResolver.AddEmbeddingLookup();
    s_microOpResolver.AddEnergy();
    s_microOpResolver.AddElu();
    s_microOpResolver.AddEqual();
    s_microOpResolver.AddEthosU();
    s_microOpResolver.AddExp();
    s_microOpResolver.AddExpandDims();
    s_microOpResolver.AddFftAutoScale();
    s_microOpResolver.AddFill();
    s_microOpResolver.AddFilterBank();
    s_microOpResolver.AddFilterBankLog();
    s_microOpResolver.AddFilterBankSquareRoot();
    s_microOpResolver.AddFilterBankSpectralSubtraction();
    s_microOpResolver.AddFloor();
    s_microOpResolver.AddFloorDiv();
    s_microOpResolver.AddFloorMod();
    s_microOpResolver.AddFramer();
    s_microOpResolver.AddFullyConnected();
    s_microOpResolver.AddGather();
    s_microOpResolver.AddGatherNd();
    s_microOpResolver.AddGreater();
    s_microOpResolver.AddGreaterEqual();
    s_microOpResolver.AddHardSwish();
    s_microOpResolver.AddIf();
    s_microOpResolver.AddIrfft();
    s_microOpResolver.AddL2Normalization();
    s_microOpResolver.AddL2Pool2D();
    s_microOpResolver.AddLeakyRelu();
    s_microOpResolver.AddLess();
    s_microOpResolver.AddLessEqual();
    s_microOpResolver.AddLog();
    s_microOpResolver.AddLogicalAnd();
    s_microOpResolver.AddLogicalNot();
    s_microOpResolver.AddLogicalOr();
    s_microOpResolver.AddLogistic();
    s_microOpResolver.AddLogSoftmax();
    s_microOpResolver.AddMaximum();
    s_microOpResolver.AddMaxPool2D();
    s_microOpResolver.AddMirrorPad();
    s_microOpResolver.AddMean();
    s_microOpResolver.AddMinimum();
    s_microOpResolver.AddMul();
    s_microOpResolver.AddNeg();
    s_microOpResolver.AddNotEqual();
    s_microOpResolver.AddOverlapAdd();
    s_microOpResolver.AddPack();
    s_microOpResolver.AddPad();
    s_microOpResolver.AddPadV2();
    s_microOpResolver.AddPCAN();
    s_microOpResolver.AddPrelu();
    s_microOpResolver.AddQuantize();
    s_microOpResolver.AddReadVariable();
    s_microOpResolver.AddReduceMax();
    s_microOpResolver.AddRelu();
    s_microOpResolver.AddRelu6();
    s_microOpResolver.AddReshape();
    s_microOpResolver.AddResizeBilinear();
    s_microOpResolver.AddResizeNearestNeighbor();
    s_microOpResolver.AddRfft();
    s_microOpResolver.AddRound();
    s_microOpResolver.AddRsqrt();
    s_microOpResolver.AddSelectV2();
    s_microOpResolver.AddShape();
    s_microOpResolver.AddSin();
    s_microOpResolver.AddSlice();
    s_microOpResolver.AddSoftmax();
    s_microOpResolver.AddSpaceToBatchNd();
    s_microOpResolver.AddSpaceToDepth();
    s_microOpResolver.AddSplit();
    s_microOpResolver.AddSplitV();
    s_microOpResolver.AddSqueeze();
    s_microOpResolver.AddSqrt();
    s_microOpResolver.AddSquare();
    s_microOpResolver.AddSquaredDifference();
    s_microOpResolver.AddStridedSlice();
    s_microOpResolver.AddStacker();
    s_microOpResolver.AddSub();
    s_microOpResolver.AddSum();
    s_microOpResolver.AddSvdf();
    s_microOpResolver.AddTanh();
    s_microOpResolver.AddTransposeConv();
    s_microOpResolver.AddTranspose();
    s_microOpResolver.AddUnpack();
    s_microOpResolver.AddUnidirectionalSequenceLSTM();
    s_microOpResolver.AddVarHandle();
    s_microOpResolver.AddWhile();
    s_microOpResolver.AddWindow();
    s_microOpResolver.AddZerosLike();
#ifdef USE_NPU
    s_microOpResolver.AddCustom(tflite::GetString_NEUTRON_GRAPH(),
        tflite::Register_NEUTRON_GRAPH());
#endif

    return s_microOpResolver;
}

int Model_RunInference(NNServer* server) {
    BProfiler profiler(server);

    tflite::MicroOpResolver &resolver = MODEL_GetOpsResolver();

    if(kTensorArenaSize == 4){
        if(server->model_flash_load){
        	kTensorArenaSize = kTensorArenaSize_flash;
        }else{
        	kTensorArenaSize = kTensorArenaSize_mem;
        }
    }
    if (server->rem_mem) {
        free(server->rem_mem);
        server->rem_mem = nullptr;
    }
    uint8_t* tensor_arena; 
    tensor_arena = (uint8_t*)malloc(kTensorArenaSize);
    while(!tensor_arena){
        tensor_arena = (uint8_t*)malloc(kTensorArenaSize);
        kTensorArenaSize -= 1024;
        if (kTensorArenaSize < 0){
    	    PRINTF("tensor_arena alloc failed.");
            return kStatus_Fail;
        }
    }

    // Build an interpreter to run the model with.
    tflite::MicroInterpreter static_interpreter(
					model, resolver, tensor_arena, kTensorArenaSize, nullptr, &profiler);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        PRINTF("tflite allocate tensors failed.\r\n");
        free(tensor_arena);
        return kStatus_Fail;
    }

    server->kTensorArenaSize = interpreter->arena_used_bytes();

    input = interpreter->input(0);
    output = interpreter->output(0);

    for (int i=0; i<interpreter->inputs_size(); i++){
        server->input.scale [i] = interpreter->input(i)->params.scale;
        server->input.zero_point [i] = interpreter->input(i)->params.zero_point;
    }

    for (int i=0; i<interpreter->outputs_size(); i++){
        server->output.scale [i] = interpreter->output(i)->params.scale;
        server->output.zero_point [i] = interpreter->output(i)->params.zero_point;
    }

    for (int i=0; i<interpreter->inputs_size(); i++){
        if ( server->input.input_data [i] ){
            memcpy ( interpreter->input(i)->data.raw, server->input.input_data[i], interpreter->input(i)->bytes);
            free (server->input.input_data [i]);
            server->input.input_data[i] = nullptr;
        }
    }

    server->input_dims_data = input->dims->data;

    // Obtain pointers to the model's input and output tensors.
    int64_t run_ns = 0, rem_size = 0;;
    for(int i=0; i< server->output.num_outputs; i++)
    {
        server->output.timing[i] = 0;
    }
    for(int i = 1; i <= server->inference_count; i++)
    {
        server->output.num_outputs = 0;
#ifdef DEBUG
        PRINTF("\r\nloop %d: \r\n", i);
#endif

        server->run_ns = 0;
        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            PRINTF("Invoke failed\n");
            return -1;
        }

#ifdef DEBUG
        PRINTF("run ms: %f ", (float)(server->run_ns/1e3));
#endif
 
        run_ns += server->run_ns;

        for(int i=0; i<interpreter->outputs().size(); i++){
            server->output.data [i] = interpreter->output(i)->data.raw;
            server->output.bytes [i] = interpreter->output(i)->bytes;
            server->output.shape_data [i] = interpreter->output (i)->dims->data;
            server->output.shape_size [i] = interpreter->output (i)->dims->size;
            switch (interpreter->output (i)->type){
            case kTfLiteFloat32:
                strcpy(server->output.data_type [i], "FLOAT32");
                break;
            case kTfLiteUInt8:
                strcpy(server->output.data_type [i], "UINT8");
                break;
            case kTfLiteInt8:
                strcpy(server->output.data_type [i], "INT8");
                break;
            default:
                break;
            }
            int max_size = (interpreter->output(i)->data.raw + interpreter->output(i)->bytes - (char*)tensor_arena);
            rem_size = max_size >  rem_size ? max_size : rem_size ;
        }

    }
    server->rem_mem = (char*)realloc(tensor_arena, rem_size);
    server->run_ns = (int64_t)(run_ns/(int64_t)server->inference_count);
    return 0;
}
