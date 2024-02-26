Overview
========

TFlite Model Behchmark example for Microcontrollers.

Toolchains supported
    - MCUXpresso IDE
    - IAR Embedded Workbench for ARM
    - Keil uVision MDK
    - ArmGCC - GNU Tools ARM Embedded

Supported board and settings
    - mcxn9xxevk:
      HTTP mode(default)
      UART mode: remove MODELRUNNER_HTTP from cc/cx flags.
    - mcxn9xxbrk:
      UART mode.
      


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- RJ45 Network cable
- Personal computer

Board settings
==============

Prepare the Demo
================
1. Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
2. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Insert the Ethernet Cable into the target board's RJ45 port and connect it to a router (or other DHCP server capable device).
4. Download the program to the target board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
Running the demo
================

Get benchmark results via ModelRunner http handler

HTTP Mode:
*************************************************
               TFLite Modelrunner
*************************************************
Initializing PHY...
 DHCP state       : SELECTING
 DHCP state       : REQUESTING
 DHCP state       : BOUND

 IPv4 Address     : 10.193.20.56
 IPv4 Subnet mask : 255.255.255.0
 IPv4 Gateway     : 10.193.20.254

Initialized TFLiteMicro modelrunner server at port 10818

1. Upload test model:
  $ curl -X PUT http://10.193.20.56:10818/v1 -F 'file=@"/home/x/eiq/model/mlperf_tiny_models/keyword_spotting/kws_ref_model.tflite";filename="kws_model;name=block_content"' && echo 
  {"reply": "success"}
2. Run latency benchmark:
  xiao@x:~/work/eiq$ curl -X POST http://10.193.20.56:10818/v1?run=1 && echo 
  {"timing":57202000}
3. Get Model info:
  xiao@x:~/work/eiq$ curl http://10.193.20.56:10818/v1/model && echo
{"timing":57207000,"inputs":[{"name":"input_1","scale":0.58470290899276733,"zero_points":83,"datatype":"INT8","shape":[1,49,10,1]}],"outputs":[{"name":"Identity","scale":0.00390625,"zero_points":-128,"datatype":"INT8","shape":[1,12]}],"layer_count":13,"layers":[{"name":"functional_1/activation/Relu;functional_1/batch_normalization/FusedBatchNormV3;functional_1/conv2d/BiasAdd/ReadVariableOp/resource;functional_1/conv2d/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/conv2d/Conv2D1","type":"CONV_2D","avg_timing":9275000,"tensor":{"timing":9275000}},{"name":"functional_1/activation_1/Relu;functional_1/batch_normalization_1/FusedBatchNormV3;functional_1/depthwise_conv2d/depthwise;functional_1/depthwise_conv2d/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/depthwise_conv2d/BiasAdd/ReadVariableOp/resource1","type":"DEPTHWISE_CONV_2D","avg_timing":4379000,"tensor":{"timing":4379000}},{"name":"functional_1/activation_2/Relu;functional_1/batch_normalization_2/FusedBatchNormV3;functional_1/conv2d_1/BiasAdd/ReadVariableOp/resource;functional_1/conv2d_1/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/conv2d_1/Conv2D1","type":"CONV_2D","avg_timing":7492000,"tensor":{"timing":7492000}},{"name":"functional_1/activation_3/Relu;functional_1/batch_normalization_3/FusedBatchNormV3;functional_1/depthwise_conv2d_1/depthwise;functional_1/depthwise_conv2d_1/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/depthwise_conv2d_1/BiasAdd/ReadVariableOp/resource1","type":"DEPTHWISE_CONV_2D","avg_timing":4374000,"tensor":{"timing":4374000}},{"name":"functional_1/activation_4/Relu;functional_1/batch_normalization_4/FusedBatchNormV3;functional_1/conv2d_2/BiasAdd/ReadVariableOp/resource;functional_1/conv2d_2/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/conv2d_2/Conv2D1","type":"CONV_2D","avg_timing":7494000,"tensor":{"timing":7494000}},{"name":"functional_1/activation_5/Relu;functional_1/batch_normalization_5/FusedBatchNormV3;functional_1/depthwise_conv2d_2/depthwise;functional_1/depthwise_conv2d_2/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/depthwise_conv2d_2/BiasAdd/ReadVariableOp/resource1","type":"DEPTHWISE_CONV_2D","avg_timing":4374000,"tensor":{"timing":4374000}},{"name":"functional_1/activation_6/Relu;functional_1/batch_normalization_6/FusedBatchNormV3;functional_1/conv2d_3/BiasAdd/ReadVariableOp/resource;functional_1/conv2d_3/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/conv2d_3/Conv2D1","type":"CONV_2D","avg_timing":7490000,"tensor":{"timing":7490000}},{"name":"functional_1/activation_7/Relu;functional_1/batch_normalization_7/FusedBatchNormV3;functional_1/depthwise_conv2d_3/depthwise;functional_1/depthwise_conv2d_3/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/depthwise_conv2d_3/BiasAdd/ReadVariableOp/resource1","type":"DEPTHWISE_CONV_2D","avg_timing":4380000,"tensor":{"timing":4380000}},{"name":"functional_1/activation_8/Relu;functional_1/batch_normalization_8/FusedBatchNormV3;functional_1/conv2d_4/BiasAdd/ReadVariableOp/resource;functional_1/conv2d_4/BiasAdd;functional_1/conv2d_4/Conv2D1","type":"CONV_2D","avg_timing":7491000,"tensor":{"timing":7491000}},{"name":"functional_1/average_pooling2d/AvgPool","type":"AVERAGE_POOL_2D","avg_timing":397000,"tensor":{"timing":397000}},{"name":"functional_1/flatten/Reshape","type":"RESHAPE","avg_timing":5000,"tensor":{"timing":5000}},{"name":"functional_1/dense/BiasAdd","type":"FULLY_CONNECTED","avg_timing":24000,"tensor":{"timing":24000}},{"name":"Identity","type":"SOFTMAX","avg_timing":27000,"tensor":{"timing":27000}}]}

4. Upload input tensor and get output tensor data:
  $ curl -s -X POST "http://10.193.20.56:10818/v1?run=1&output=Identity" -F'file=@"/home/x/eiq/benchmark-runner-ml/datasets/kws01/tst_000337_Yes_9.bin";filename="input_tensor;name=input_1"' |jq .outputs[0].data | xargs -i   echo  {$i} |base64 -d | hexdump -C
  00000000  80 80 80 80 80 80 80 80  80 7f 80 80              |............|

  $ curl -s -X POST "http://10.193.20.56:10818/v1?run=1&output=Identity" -F'file=@"/home/x/eiq/benchmark-runner-ml/datasets/kws01/tst_000301_Left_2.bin";filename="input_tensor;name=input_1"' |jq .outputs[0].data | xargs -i   echo  {$i} |base64 -d | hexdump -C
  00000000  80 80 7f 80 80 80 80 80  80 80 80 80              |............|

UART Mode:
----------------------------------------

 1. Download the program to the target board.
 2. Use the scripts scripts/main.py to launch the http service on x86 platform to handle the http requests and forward the data to the board via uart.
   #python3 main.py
     * Serving Flask app "main" (lazy loading)
     * Environment: production
       WARNING: This is a development server. Do not use it in a production deployment.
       Use a production WSGI server instead.
     * Debug mode: on
     * Running on http://0.0.0.0:10919/ (Press CTRL+C to quit)
     * Restarting with stat
     * Debugger is active!
     * Debugger PIN: 132-343-000
 3. Connect the uart cable to x86.

The uri should be http://localhost:10919/serial/<board-serial-id>
For linux server, the serial id can be found in /dev/serial/by-id:
  #ls /dev/serial/by-id/
   usb-NXP_Semiconductors_LPC-LINK2_CMSIS-DAP_V5.224_GRAXBQHR-if01
For Windows server, the serial id is com<x>

Do benchmark testing with deepview-validator or eIQ Model Tool:
  - DeepviewRT validator:
    #deepview-validator --input_names input --output_names MobilenetV1/Predictions/Reshape_1 --samples 1 --uri http://localhost:10919/serial/GRAXBQHR \
       --reference mobilenet_v1_1.0_224_pb.npz mobilenet_v1_1.0_224_pb_uint8.tflite
  - eIQ Model Tool:
   Add target with uri 'http://localhost:10919/serial/GRAXBQHR' and click the 'Tools->Profile Model' button to to profiling model.

Get benchmark results via Modelrunner cli
-----------------------------------------

PS C:\scripts> python .\cli.py com20

=> reset
*************************************************
               TFLite Modelrunner
*************************************************

=> model_loadb model.tflite
 model_loadb 53936
######### Ready for TFLite model download Flash
FLASH 0x100000 Erased, Program at 100000: 16384 bytes  ==
FLASH 0x104000 Erased, Program at 104000: 16384 bytes  ==
FLASH 0x108000 Erased, Program at 108000: 16384 bytes  ==
FLASH 0x10c000 Erased, Program at 10c000: 16384 bytes  ==

=> model
{
  "inputs": [
    {
      "datatype": "INT8",
      "name": "input_1",
      "scale": 0.5847029089927673,
      "shape": [
        1,
        49,
        10,
        1
      ],
      "zero_points": 83
    }
  ],
  "layer_count": 13,
  "layers": [
 {
      "avg_timing": 9827000,
      "name": "functional_1/activation/Relu;functional_1/batch_normalization/FusedBatchNormV3;functional_1/conv2d/BiasAdd/ReadVariableOp/resource;functional_1/conv2d/BiasAdd;functional_1/conv2d_4/Conv2D;functional_1/conv2d/Conv2D1",
      "tensor": {
        "timing": 9827000
      },
      "type": "CONV_2D"
    },
      ...  
      ...
  ],
  "outputs": [
    {
      "datatype": "INT8",
      "name": "Identity",
      "scale": 0.00390625,
      "shape": [
        1,
        12
      ],
      "zero_points": -128
    }
  ],
  "timing": 60370000
}

=> tensor_loadb input_1 tmp.input
tensor_loadb input_1 490
######### Ready for input_1 tensor download  MEM

######### 490 bytes received #########

=> run output=Identity
{
  "outputs": [
    {
      "data": "gICAgICAgIB/gICA",
      "datatype": "INT8",
      "name": "Identity",
      "shape": [
        1,
        12
      ],
      "type": "SOFTMAX"
    }
  ],
  "timing": 59808000
}

=>


The latency timing is 59.808 ms, the layer timings can be found in model command, for example the first layer timing is 9.827 ms.
