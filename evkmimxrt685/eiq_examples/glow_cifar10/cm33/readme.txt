Overview
========

Demonstrates inference for models compiled using the GLOW AOT tool.
The network used in this is based on the CIFAR-10 example in TensorFlow Lite.

The CNN model in the example was trained using the scripts available at [1]
with the CifarNet model. 
The configuration of the model was modified to match the neural 
network structure in the CMSIS-NN CIFAR-10 example.
The example source code is a modified version of the Label Image
example from the TensorFlow Lite examples [2], adjusted to run on MCUs.
The neural network consists of 3 convolution layers interspersed by
ReLU activation and max pooling layers, followed by a fully-connected layer
at the end. The input to the network is a 32x32 pixel color image, which is 
classified into one of the 10 output classes. The model size is 97 KB.

[1] https://github.com/tensorflow/models/tree/master/research/slim

In order to generate input and output, serialize_data.py is used. 
The input to the network is a 32x32 RDG image in NHWC format, which will 
be classified into one of the 10 output classes. Given the model,
the input is preprocessed and generated in io/input.bin and the 
output is generated io/output.bin.



Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- JTAG/SWD
- evkmimxrt685 board
- Personal Computer

Prepare the Demo
================
1. Import the SDK for mimxrt685-evk in MCUXpresso IDE. 
Depending on how you want your project to run follow one of the next steps:
   a. To run inferece on MCU with CMSIS-NN:
      - Generate a Glow bundle with "-use-cmsis" flag only and insert the .o, .h and .bin
       files in your MCU project in "glow_bundle" folder.
   
   <glowNN_root>\bin\model-compiler.exe \
		-model=cifarnet_quant_int8.tflite \
		-network-name=model \
		-backend=CPU -target=arm -mcpu=cortex-m33 -float-abi=hard \
		-use-cmsis \
		-tflite-float-softmax \
		-emit-bundle=glow_bundle_test 

      ex: glowNN_root = C:\Users\MyUser\Documents\GlowNN

   b. To run inferece on MCU with CMSIS-NN and HIFI-NN optimizations:
      - Generate a Glow bundle with "-use-cmsis" and "-use-hifi" flags and insert the .o, .h and .bin
       files in your MCU project in "glow_bundle" folder.

   <glowNN_root>\bin\model-compiler.exe \
		-model=cifarnet_quant_int8.tflite \
		-network-name=model \
		-backend=CPU -target=arm -mcpu=cortex-m33 -float-abi=hard \
		-ude-hifi -use-cmsis \
		-tflite-float-softmax \
		-emit-bundle=glow_bundle_test 


   c. To run inferece on DSP standalone (with HIFI-NN optimizations): 
      - Generate a Glow bundle using xt-clang compiler version with "-use-hifi" flag and insert the 
       .o and .h files in your DSP project in "glow_bundle" folder. 

   <glowNN_root>\bin\model-compiler.exe \
		-model=cifarnet_quant_int8.tflite \
		-network-name=model \
		-backend=CPU -target=arm -mcpu=cortex-m33 -float-abi=hard \
		-llvm-compiler=<path_to_xtensa>\XtDevTools\install\tools\RI-2020.5-win32\XtensaTools\bin\xt-clang.exe \
		-llvm-compiler-opt="-mlsp=<path_to_RT600_SDK>\devices\MIMXRT685S\xtensa\sim -c --xtensa-core=nxp_rt600_RI2020_5_newlib"\
		-use-hifi \
		-tflite-float-softmax \
		-emit-bundle=glow_bundle_test 

      ex:   path_to_xtensa = C:\usr\xtensa
            path_to_RT600_SDK = C:\usr\SDK_MIMXRT685

2. Check main_cm33.c for the lines:
   #define INFERENCE_CONTROL_MCU 0
   #define INFERENCE_CONTROL_DSP 1
   #define INFERENCE_CONTROL INFERENCE_CONTROL_MCU
and depending on how you want your project to run change the last line to:
   - "#define INFERENCE_CONTROL INFERENCE_CONTROL_MCU" -> if you want to run entirely on MCU
   - "#define INFERENCE_CONTROL INFERENCE_CONTROL_DSP" -> if you want to run on DSP

3. Clean the project evrey time when changing a binary file.

4. Build the project.

5. Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

6. Run the project and in the console from the IDE check the output.  The application
image includes also the DSP image which will be loaded automatically.

Running the demo with CMSIS-NN on MCU:
======================================

Started Cifar10 demo and benchmark
DSP Image copied to SRAM
---- Cifar10 Quantized Demo ----- 
Top1 class = 3
Ref class = 3
Confidence = 0.997448205947875976
Inference = 113.034 (ms)
Throughput = 8.8 fps

Top1 class = 8
Ref class = 8
Confidence = 0.999054968357086181
Inference = 112.901 (ms)
Throughput = 8.9 fps

Top1 class = 8
Ref class = 8
Confidence = 0.970263242721557617
Inference = 112.840 (ms)
Throughput = 8.9 fps

...

Top1 class = 3
Ref class = 0
Confidence = 0.426881968975067138
Inference = 112.848 (ms)
Throughput = 8.9 fps

Top1 class = 1
Ref class = 0
Confidence = 0.467271983623504638
Inference = 112.826 (ms)
Throughput = 8.9 fps

Top1 class = 7
Ref class = 7
Confidence = 0.961519420146942138
Inference = 112.818 (ms)
Throughput = 8.9 fps


Overall accuracy = 72.00 %


Running the demo with CMSIS-NN and HIFI-NN on MCU:
==================================================

Started Cifar10 demo and benchmark
DSP Image copied to SRAM
---- Cifar10 Quantized Demo ----- 
Top1 class = 3
Ref class = 3
Confidence = 0.997489213943481445
Inference = 7.953 (ms)
Throughput = 125.7 fps

Top1 class = 8
Ref class = 8
Confidence = 0.999156832695007324
Inference = 7.822 (ms)
Throughput = 127.8 fps

Top1 class = 8
Ref class = 8
Confidence = 0.973141729831695556
Inference = 7.819 (ms)
Throughput = 127.9 fps

...

Top1 class = 3
Ref class = 0
Confidence = 0.474295914173126220
Inference = 7.814 (ms)
Throughput = 128.0 fps

Top1 class = 1
Ref class = 0
Confidence = 0.467860609292984008
Inference = 7.807 (ms)
Throughput = 128.1 fps

Top1 class = 7
Ref class = 7
Confidence = 0.963658094406127929
Inference = 7.799 (ms)
Throughput = 128.2 fps


Overall accuracy = 72.00 %


Running the demo with HIFI-NN on DSP standalone:
================================================

Started Cifar10 demo and benchmark
DSP Image copied to SRAM
---- Cifar10 Quantized Demo ----- 
Top1 class = 3
Ref class = 3
Confidence = 0.997489213943481445
Inference = 7.572 (ms)
Throughput = 132.1 fps

Top1 class = 8
Ref class = 8
Confidence = 0.999156832695007324
Inference = 7.466 (ms)
Throughput = 133.9 fps

Top1 class = 8
Ref class = 8
Confidence = 0.973141729831695556
Inference = 7.459 (ms)
Throughput = 134.1 fps

...

Top1 class = 3
Ref class = 0
Confidence = 0.474295914173126220
Inference = 7.461 (ms)
Throughput = 134.0 fps

Top1 class = 1
Ref class = 0
Confidence = 0.467860609292984008
Inference = 7.462 (ms)
Throughput = 134.0 fps

Top1 class = 7
Ref class = 7
Confidence = 0.963658094406127929
Inference = 7.458 (ms)
Throughput = 134.1 fps


Overall accuracy = 72.00 %
