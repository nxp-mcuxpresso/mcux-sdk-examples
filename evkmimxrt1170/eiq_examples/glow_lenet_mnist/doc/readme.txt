Overview
========
This example project provides an inference example using the Lenet model
compiled with the Glow AOT software tools. The model is capable to perform
hand-written digit classification. The model is using 28 x 28 grayscale
input images and provides the confidence scores for the 10 output classes:
digits "0" to "9". The application will run the inference on a sample
image and display the top1 classification results and the inference time.
The project example will walk through all the steps of downloading and
compiling the model, pre-processing a sample image, building and running
the project.

Files:
  main.c  - example source code
  timer.c - implementation of helper functions for measuring inference time
  timer.h - declarations of helper functions for measuring inference time


Toolchains supported
====================
- MCUXpresso IDE
- IAR Embedded Workbench for ARM
- ArmGCC - GNU Tools ARM Embedded

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-IMXRT1170 board
- Personal computer (Windows)
- Python 3.x installed
- Glow AOT tools installed for Windows and added to system PATH variable:
  - image-classifier.exe
  - model-compiler.exe

You can find the Glow Windows Installer on the NXP eIQ mirror:
https://www.nxp.com/design/software/development-software/eiq-ml-development-environment:EIQ

Board settings
==============
No special settings are required.

Prepare the demo
================
0. Create a folder "model" in the root directory of the project with two subfolders:
   "lenet_mnist" and "dataset".

1. Download the Lenet model files (Caffe2 format) using your browser from the
   following links into the "model\lenet_mnist" project directory:

   http://fb-glow-assets.s3.amazonaws.com/models/lenet_mnist/predict_net.pb
   http://fb-glow-assets.s3.amazonaws.com/models/lenet_mnist/init_net.pb

2. Download the Mnist image samples using your browser from the official Glow
   repository into the "model\dataset" project directory. The images contain
   one sample image for each digit from "0" to "9".

   https://github.com/pytorch/glow/blob/master/tests/images/mnist/0_1009.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/1_1008.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/2_1065.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/3_1020.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/4_1059.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/5_1087.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/6_1099.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/7_1055.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/8_1026.png?raw=true
   https://github.com/pytorch/glow/blob/master/tests/images/mnist/9_1088.png?raw=true

3. Perform profiling on the Mnist dataset using the "image-classifier" Glow tool.
   Open a Windows Command Prompt with the current path set within the project 
   directory and run the following command:

image-classifier model\dataset\*.png ^
    -image-mode=0to1 -image-layout=NCHW ^
    -model=model\lenet_mnist -model-input-name=data ^
    -dump-profile=model\profile.yml ^
    -quantization-schema=symmetric_with_power2_scale ^
    -quantization-precision-bias=Int8

4. Compile the model and generate the bundle using the "model-compiler" Glow tool.
   Run the following command:

model-compiler -model=model\lenet_mnist -model-input=data,float,[1,1,28,28] ^
    -backend=CPU -target=arm -mcpu=cortex-m7 -float-abi=hard ^
    -emit-bundle=source -load-profile=model\profile.yml ^
    -quantization-schema=symmetric_with_power2_scale ^
    -quantization-precision-bias=Int8 ^
    -use-cmsis

5. Use the python script "scripts\glow_process_image.py" to transform an input
   image into a C array data and save it as "source\input_image.inc" which is
   included in main.cpp. As an example we will use the sample image "9_1088.png"
   which contains the digit "9":

python scripts\glow_process_image.py ^
    -input-path=model\dataset\9_1088.png ^
    -output-path=source\input_image.inc ^
    -image-mode=0to1 -image-layout=NCHW -image-channel-order=RGB

6. Import the project in IDE and link the Glow bundle by performing the following steps:

   For MCUXpresso IDE:
     - Right click on the project -> "Properties" 
     - Select "C/C++ Build" -> "Settings" 
     - In the "Tool Setting" tab select "MCU C++ Linker" -> "Miscellaneous" 
     - Add the bundle to "Other objects" panel by clicking "Add..." and specify
       the relative path to the bundle in the project: "../source/lenet_mnist.o"

   For IAR Embedded Workbench:
     - Right click on the "source" folder -> Add -> Add Files...
     - Choose file category "Library/Object Files (*.r;*.a;*.lib;*.o)"
     - Select the object file "lenet_mnist.o" located in "source" folder

   For Keil uVision MDK:
     - Right click on the target -> "Options for Target ..."
     - Select "Linker" tab
     - In the "Misc controls" section add to the existing string with a 
       separating space (" ") character the string "../source/lenet_mnist.o"

   For Armgcc:
     - Edit the "CMakeLists.txt" file from the "armgcc" directory by adding the
       following line at the end of the file:
         target_link_libraries(glow_lenet_mnist.elf ${ProjDirPath}/../source/lenet_mnist.o)

7. Build the project.

8. Connect a USB cable between the host PC and the OpenSDA USB port on the target board.

9. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

10. Download the program to the target board.

Running the demo
================
The log below shows the output of the Release version in the terminal window:

Top1 class = 9
Confidence = 0.942
Inference time = 6 (ms)
