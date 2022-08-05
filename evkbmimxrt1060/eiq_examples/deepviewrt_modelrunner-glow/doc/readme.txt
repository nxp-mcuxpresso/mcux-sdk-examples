eIQ modelRunner for glow

Content
-------
1. Introduction
2. Directory structure
3. eIQ inference with DeepView RT Example
4. Documentation
5. Library configuration
6. Release notes
7. ModelRunner for GLOW example
8. Appendix: Glow Model Conversion

1. Introduction
---------------
DeepView RT is an implements a neural network inference engine suitable for embedded processors,
graphics processors, and microcontrollers. The DeepView RT Model (RTM) format supports in-place
interpretation which allows it to be stored directly in flash and used as-is. This allows for
resource constrained MCU (ex: Cortex-M) platforms as the actual model and its weights do not
consume any RAM and instead can be kept in-place in flash memory. A small amount of memory is
required for the network evaluation graph as well as a buffer cache for storing the volatile
input/output data at inference time, but no memory is required for the actual weights which can
remain in flash. If the cache is large enough to host the weights as well, they will be streamed
from flash on-demand as a performance optimization, and for maximum performance on parts with
adequate memory the entire model can be stored in RAM.
DeepView RT ModelRunner for GLOW is similar service as ModelRunner for RTM, but it can only stream 
test data to modelrunner service with GLOW backend, it can return inferrence result after receive
test data. 
Note: DeepView RT supports only a subset of operators available in
      TensorFlow. The eIQ toolkit can convert Tensorflow 2.x model to RTM model, please refer to
      eIQ toolkit about model conversion.
      ModelRunner for GLOW can't work with eIQ toolkit for validating model or eIQ modeltool to 
      profiling model.

2. Directory structure
--------------------------------------
<MCUXpresso-SDK-root>
|-- boards
|   -- <board>
|      -- eiq_examples                         - Example build projects
|         -- deepviewrt_modelrunner-glow       - deepviewrt example
|
|-- middleware
    -- eiq
       -- deepviewrt
          -- include                   - DeepviewRT library header files
          -- lib                       - DeepviewRT library binaries

3. eIQ Inference with DeepView RT example
-----------------------------------------
3.1 Introduction
    The package contains modelrunner example applications using
    the DeepView RT library. The build projects can be found in
    the /boards/<board>/eiq_examples/deepviewrt* folders.

3.2 Toolchains supported
    - MCUXpresso IDE
    - IAR Embedded Workbench for ARM
    - Keil uVision MDK
    - ArmGCC - GNU Tools ARM Embedded

3.3 Supported board and settings
    - evkmimxrt1170:
      No special settings are required.
    - evkmimxrt1064:
      No special settings are required.
    - evkmimxrt1060:
      No special settings are required.
    - evkbimxrt1050:
      No special settings are required.
    - evkmimxrt1160:
      No special settings are required.

4. Documentation
----------------
    https://www.nxp.com/design/software/development-software/eiq-ml-development-environment:EIQ

5. Library configuration
------------------------
5.1. Stack memory configuration
     During the library compilation, based on the stack memory configuration,
     the EIGEN_STACK_ALLOCATION_LIMIT macro definition can be set to the maximum
     size of temporary objects that can be allocated on the stack
     (they will be dynamically allocated instead). A high number may cause stack
     overflow. A low number may decrease object allocation performance.

6. Release notes
----------------
The library is based on DeepView RT version 2.4.44.

7. ModelRunner for GLOW Example
----------------
7.1 Running the demo
The ModelRunner for GLOW will run as one http service and can get data input. 
It dump the 
************************************************
 GLOW Modelrunner
************************************************
Initializing PHY...
 DHCP state       : SELECTING
 DHCP state       : REQUESTING
 DHCP state       : BOUND

 IPv4 Address     : 10.193.21.56
 IPv4 Subnet mask : 255.255.255.0
 IPv4 Gateway     : 10.193.21.254


************************************************
Model Inference time = 1067 (ms), return_status = 0
************************************************
Initialized GLOW modelrunner server at port 10818

7.2 Stream test data to ModelRunner for GLOW service
On Linux or Windows PC, prepare one image (jpg or png formate). Then use following
command to post data to ModelRunner for GLOW service on device. The last row show the
inferrence timing information, decode tming information, input layer id, label id
and softmax value. 

$ curl -D - -XPOST -H 'Content-Type: image/*' --data-binary "@./panda.jpg" 'http://10.193.21.56:10818/v1?run=1'
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 49460    0     0  100 49460      0  41148  0:00:01  0:00:01 --:--:-- 41182HTTP/1.1 200 OK
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, PUT, POST, HEAD, DELETE
Access-Control-Expose-Headers: *
Access-Control-Max-Age: 86400
Content-Type: application/json
Content-Length: 81
Connection: close

100 49541  100    81  100 49460     56  34228  0:00:01  0:00:01 --:--:-- 34308

{"timing":1071000000,"decode":331000000,"input":0,"label":389,"softmax":0.774535}

Note: Get service IP and port number from device's output. 
      The unit of above timing and decode result is ns. 
      The label id is got from model and is using imagenet labels, so the "389" means panda.


8. Appendix: Glow Model Conversion
------------------------
This is a collection of commands to convert float/uint8/int8 tflite models to GLOW bundles. Example model used is the mobilenet_v2_0.35_224.tflite in our eIQ model zoo. mobilenet_edgetpu_1.0_224_int8 is used as an example for per-channel quantized model (obtained from TensorFlow's repo)

8.1 Model Compile

   - Float models (Slow inference):
     ```shell
     $ model-compiler -model=mobilenet_v2_0.35_224.tflite
       -model-input=input,float,[1,224,224,3]
       -emit-bundle=bundle
       -backend=CPU
       -target=arm
       -mcpu=cortex-m7
       -float-abi=hard
       -network-name=model
     ```

8.2 CMSIS-NN and quantizing float models

   - Generate quantization profiling info:

      Images have to be PNG format and resolution should match the model input. Throws errors if image resolution is different from model input. 
      Tested with 1 image but more images are needed for better profile

      ```shell
      $ image-classifier.exe
        -input-image-dir=C:\validation\imagenet\png
        -image-mode=neg1to1
        -image-layout=NHWC
        -image-channel-order=RGB
        -model=mobilenet_v2_0.35_224.tflite
        -model-input=input,float,[1,224,224,3]
        -dump-profile=profile.yml
      ```

    - Generate CMSIS_NN object code (using profile from above):

     ```shell
     $ model-compiler -model=mobilenet_v2_0.35_224.tflite
       -model-input=input,float,[1,224,224,3]
       -emit-bundle=bundle
       -backend=CPU -target=arm
       -mcpu=cortex-m7 -float-abi=hard
       -network-name=model -load-profile=profile.yml
       -quantization-schema=symmetric_with_power2_scale
       -quantization-precision-bias=Int8 -use-cmsis
     ```

8.3 UINT8 per-tensor quantized models (Fastest among QASYMM8 models)

    The tflite model used below is UINT8 per-tensor quantized (legacy quant). GLOW generates model which has INT8 input and FLOAT output.
    Note that '-model-input' param does not support UINT8 format. Both commands below generate bundles without errors.

   ```shell
   $ model-compiler -model=mobilenet_v2_0.35_224_quant.tflite
     -model-input=input,int8q,0.007874015718698502,0,[1,224,224,3]
     -emit-bundle=bundle -backend=CPU -target=arm -mcpu=cortex-m7
     -float-abi=hard -tflite-uint8-to-int8 -network-name=model

   $ model-compiler -model=mobilenet_v2_0.35_224_quant.tflite
     -emit-bundle=bundle -backend=CPU
     -target=arm -mcpu=cortex-m7 -float-abi=hard
     -tflite-uint8-to-int8 -network-name=model
   ```

8.4 INT8 per-channel quantized models (Slow)

  The tflite model used below is INT8 per-channel quantized (TensorFlow V2).
  Not sure if -enable-channelwise has an impact on accuracy/time. GLOW generates model which has INT8 input and FLOAT output.

  ```shell
  $ model-compiler -model=mobilenet_edgetpu_224_1.0_int8.tflite
    -model-input=images,int8q,0.010868912562727928,-6,[1,224,224,3]
    -emit-bundle=bundle
    -backend=CPU
    -target=arm
    -mcpu=cortex-m7
    -float-abi=hard
    -network-name=model

  $ model-compiler -model=mobilenet_edgetpu_224_1.0_int8.tflite
   -model-input=images,int8q,0.010868912562727928,-6,[1,224,224,3]
   -emit-bundle=bundle
   -backend=CPU
   -target=arm
   -mcpu=cortex-m7
   -float-abi=hard
   -enable-channelwise
   -network-name=model
  ```

8.5 Input generation using python script

The tensor gen script had to be modified for image resizing. glow_process_image.py is therefore different from NXP's version.
Following commands are for INT8, FLOAT and FLOAT_NCHW input tensor generation. '-r' is the new param for resizing images

  ```shell
  $ python glow_process_image.py -i C:\validation\imagenet\bald_eagle.jpg -o C:\validation\glow\bald_eagle.inc -t int8 -r 224
  $ python glow_process_image.py -i C:\validation\imagenet\bald_eagle.jpg -o C:\validation\glow\bald_eagle.inc -m neg1to1 -r 224
  $ python glow_process_image.py -i C:\validation\imagenet\bald_eagle.jpg -o C:\validation\glow\bald_eagle.inc -m neg1to1 -l NCHW -r 224
  ```

8.6 Header modification (model.h)

The model-compiler commands generate a header, object file and weights (txt,binary) with name "model". These need to be copied to the source directory of the project before re-building.

Other modelrunners get model information from the loaded onnx/rtm/tflite but GLOW model is built at compile time, so the modelinfo is derived from headers

Sample model.h:

  ```C++
  // Bundle API auto-generated header file. Do not edit!
  // Glow Tools version: 2020-11-26

  #ifndef _GLOW_BUNDLE_MODEL_H
  #define _GLOW_BUNDLE_MODEL_H

  #include <stdint.h>

  // ---------------------------------------------------------------
  //                       Common definitions
  // ---------------------------------------------------------------
  #ifndef _GLOW_BUNDLE_COMMON_DEFS
  #define _GLOW_BUNDLE_COMMON_DEFS

  // Glow bundle error code for correct execution.
  #define GLOW_SUCCESS 0

  // Memory alignment definition with given alignment size
  // for static allocation of memory.
  #define GLOW_MEM_ALIGN(size)  __attribute__((aligned(size)))

  // Macro function to get the absolute address of a
  // placeholder using the base address of the mutable
  // weight buffer and placeholder offset definition.
  #define GLOW_GET_ADDR(mutableBaseAddr, placeholderOff)  (((uint8_t*)(mutableBaseAddr)) + placeholderOff)

  #endif

  // ---------------------------------------------------------------
  //                          Bundle API
  // ---------------------------------------------------------------
  // Model name: "model"
  // Total data size: 2704000 (bytes)
  // Placeholders:
  //
  //   Name: "input"
  //   Type: i8[S:0.007874016 O:0][-1.008,1.000]<1 x 224 x 224 x 3>
  //   Size: 150528 (elements)
  //   Size: 150528 (bytes)
  //   Offset: 0 (bytes)
  //
  //   Name: "MobilenetV2_Predictions_Softmax"
  //   Type: float<1 x 1001>
  //   Size: 1001 (elements)
  //   Size: 4004 (bytes)
  //   Offset: 150528 (bytes)
  //
  // NOTE: Placeholders are allocated within the "mutableWeight"
  // buffer and are identified using an offset relative to base.
  // ---------------------------------------------------------------
  #ifdef __cplusplus
  extern "C" {
  #endif

  // Placeholder address offsets within mutable buffer (bytes).
  #define MODEL_input                            0
  #define MODEL_MobilenetV2_Predictions_Softmax  150528

  // Memory sizes (bytes).
  #define MODEL_CONSTANT_MEM_SIZE     1696448
  #define MODEL_MUTABLE_MEM_SIZE      154560
  #define MODEL_ACTIVATIONS_MEM_SIZE  852992

  // Memory alignment (bytes).
  #define MODEL_MEM_ALIGN  64

  // Bundle entry point (inference function). Returns 0
  // for correct execution or some error code otherwise.
  int model(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

  #ifdef __cplusplus
  }
  #endif
  #endif

  ```

It would be useful for automation if the following information is either added to model.h or generated as a separate header(say modelinfo.h). Most information already exists in comments of model.h. Other information can be gleaned from the tflite/onnx model itself during bundle generation

  Care must be taken for models with multiple outputs.

  ```C++
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Find a way to add following to model.h or separate header
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  #define MODEL_NAME     "mobilenet_v2_0.35_224_quant"
  // model input information
  #ifndef MODEL_input
  #define MODEL_input    0
  #endif
  #define INPUT_NAME     "input"
  #define INPUT_NCHW     false
  #define INPUT_SIZE     150528
  #define INPUT_TYPE     0   //0-INT8; 1-FLOAT; Expand as needed
  #define INPUT_SHAPE    {1,224,224,3}
  #define NUM_INPUT_DIMS 4
  #define INPUT_SCALE    0.007874016f
  #define INPUT_ZEROP    0
  // model output information.
  #ifndef MODEL_output
  #define MODEL_output    MODEL_MobilenetV2_Predictions_Softmax
  #endif
  #define OUTPUT_NAME     "MobilenetV2_Predictions_Softmax"
  #define OUTPUT_SIZE     4004
  #define OUTPUT_TYPE     1   //0-INT8; 1-FLOAT
  #define OUTPUT_SHAPE    {1,1001}
  #define NUM_OUTPUT_DIMS 2
  #define NUM_OUTPUTS     1

  #if NUM_OUTPUTS > 1
  #ifndef MODEL_output1
  #define MODEL_output1    MODEL_MobilenetV2_Predictions_Softmax
  #endif
  #define OUTPUT1_NAME     "MobilenetV2_Predictions_Softmax"
  #define OUTPUT1_SIZE     4004
  #define OUTPUT1_TYPE     1   //0-INT8; 1-FLOAT
  #define OUTPUT1_SHAPE    {1,1001}
  #define NUM_OUTPUT1_DIMS 2
  #endif

  #if NUM_OUTPUTS > 2
  #ifndef MODEL_output2
  #define MODEL_output2    MODEL_MobilenetV2_Predictions_Softmax
  #endif
  #define OUTPUT2_NAME     "MobilenetV2_Predictions_Softmax"
  #define OUTPUT2_SIZE     4004
  #define OUTPUT2_TYPE     1   //0-INT8; 1-FLOAT
  #define OUTPUT2_SHAPE    {1,1001}
  #define NUM_OUTPUT2_DIMS 2
  #endif

  #if NUM_OUTPUTS > 3
  #ifndef MODEL_output3
  #define MODEL_output3    MODEL_MobilenetV2_Predictions_Softmax
  #endif
  #define OUTPUT3_NAME     "MobilenetV2_Predictions_Softmax"
  #define OUTPUT3_SIZE     4004
  #define OUTPUT3_TYPE     1   //0-INT8; 1-FLOAT
  #define OUTPUT3_SHAPE    {1,1001}
  #define NUM_OUTPUT3_DIMS 2
  #endif
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Find a way to automate above
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  ```

