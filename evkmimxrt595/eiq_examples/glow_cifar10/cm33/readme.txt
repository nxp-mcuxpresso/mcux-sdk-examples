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
- evkmimxrt595 board
- Personal Computer

Prepare the Demo
================
1. Import the SDK for mimxrt595-evk in MCUXpresso IDE. 
Depending on how you want your project to run follow one of the next steps:
   a. To run inferece on MCU:
      - Generate a Glow bundle and insert the .o, .h and .bin
       files in your MCU project in "glow_bundle" folder.
   
   <glowNN_root>\bin\model-compiler.exe \
		-model=cifarnet_quant_int8.tflite \
		-network-name=model \
		-backend=CPU -target=arm -mcpu=cortex-m33 -float-abi=hard \
		-emit-bundle=glow_bundle_test 

      ex: glowNN_root = C:\Users\MyUser\Documents\GlowNN

   b. To run inferece on MCU wand HIFI-NN optimizations:
      - Generate a Glow bundle with "-use-fusion" flags and insert the .o, .h and .bin
       files in your MCU project in "glow_bundle" folder.
      

   <glowNN_root>\bin\model-compiler.exe \
		-model=cifarnet_quant_int8.tflite \
		-network-name=model \
		-backend=CPU -target=arm -mcpu=cortex-m33 -float-abi=hard \
		-ude-fusion \
		-emit-bundle=glow_bundle_test 


   c. To run inferece on DSP standalone (with HIFI-NN optimizations): 
      - [For RT500]: Generate a Glow bundle using xt-clang compiler version with "-use-hifi" flag and insert the 
       .o and .h files in your DSP project in "glow_bundle" folder. 

   <glowNN_root>\bin\model-compiler.exe \
		-model=cifarnet_quant_int8.tflite \
		-network-name=model \
		-backend=CPU -target=arm -mcpu=cortex-m33 -float-abi=hard \
		-llvm-compiler=<path_to_xtensa>\XtDevTools\install\tools\RI-2020.5-win32\XtensaTools\bin\xt-clang.exe \
		-llvm-compiler-opt="-mlsp=<path_to_RT600_SDK>\devices\MIMXRT595S\xtensa\sim -c --xtensa-core=nxp_rt500_RI2020_5_newlib"\
		-use-fusion \
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

Running the demo on MCU:
========================

Started Cifar10 demo and benchmark
DSP Image copied to SRAM
---- Cifar10 Quantized Demo ----- 
Top1 class = 3
Ref class = 3
Confidence = 0.996093750000000000
Inference = 348.439 (ms)
Throughput = 2.9 fps

Top1 class = 8
Ref class = 8
Confidence = 0.996093750000000000
Inference = 348.374 (ms)
Throughput = 2.9 fps

Top1 class = 8
Ref class = 8
Confidence = 0.964843750000000000
Inference = 348.356 (ms)
Throughput = 2.9 fps

...

Top1 class = 3
Ref class = 0
Confidence = 0.437499999999999999
Inference = 348.332 (ms)
Throughput = 2.9 fps

Top1 class = 1
Ref class = 0
Confidence = 0.433593749999999999
Inference = 348.368 (ms)
Throughput = 2.9 fps

Top1 class = 7
Ref class = 7
Confidence = 0.953125000000000000
Inference = 348.367 (ms)
Throughput = 2.9 fps


Overall accuracy = 73.00 %


Running the demo on MCU with model optimizations:
=================================================

Started Cifar10 demo and benchmark
DSP Image copied to SRAM
---- Cifar10 Quantized Demo ----- 
Top1 class = 3
Ref class = 3
Confidence = 0.996093750000000000
Inference = 80.374 (ms)
Throughput = 12.4 fps

Top1 class = 8
Ref class = 8
Confidence = 0.996093750000000000
Inference = 80.343 (ms)
Throughput = 12.4 fps

Top1 class = 8
Ref class = 8
Confidence = 0.972656250000000000
Inference = 80.326 (ms)
Throughput = 12.4 fps

...

Top1 class = 3
Ref class = 0
Confidence = 0.472656250000000000
Inference = 80.325 (ms)
Throughput = 12.4 fps

Top1 class = 1
Ref class = 0
Confidence = 0.464843750000000000
Inference = 80.330 (ms)
Throughput = 12.4 fps

Top1 class = 7
Ref class = 7
Confidence = 0.960937500000000000
Inference = 80.328 (ms)
Throughput = 12.4 fps


Overall accuracy = 72.00 %


Running the demo on DSP:
========================

Started Cifar10 demo and benchmark
DSP Image copied to SRAM
---- Cifar10 Quantized Demo ----- 
Top1 class = 3
Ref class = 3
Confidence = 0.996093750000000000
Inference = 79.843 (ms)
Throughput = 12.5 fps

Top1 class = 8
Ref class = 8
Confidence = 0.996093750000000000
Inference = 79.803 (ms)
Throughput = 12.5 fps

Top1 class = 8
Ref class = 8
Confidence = 0.972656250000000000
Inference = 79.811 (ms)
Throughput = 12.5 fps

...

Top1 class = 3
Ref class = 0
Confidence = 0.472656250000000000
Inference = 79.804 (ms)
Throughput = 12.5 fps

Top1 class = 1
Ref class = 0
Confidence = 0.464843750000000000
Inference = 79.809 (ms)
Throughput = 12.5 fps

Top1 class = 7
Ref class = 7
Confidence = 0.960937500000000000
Inference = 79.803 (ms)
Throughput = 12.5 fps


Overall accuracy = 72.00 %
