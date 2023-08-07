Overview
========
Demonstrates inference for models compiled using the GLOW AOT tool.
The network used in this is based on the CIFAR-10 example in Caffe2 [1] & [2].

[1] https://github.com/caffe2/tutorials/blob/master/CIFAR10_Part1.ipynb
[2] https://github.com/caffe2/tutorials/blob/master/CIFAR10_Part2.ipynb

This project does not include the pre-trained model or the training script
since Caffe2 framework is deprecated and lately has become part of PyTorch.
This project example only includes the bundle (binary) generated after running
the Glow AOT tool and is intended to be used as-is. If you want a step-by-step
example of running the Glow AOT tool for a given model take a look at the
LeNet MNIST Glow example.

The neural network consists of 3 convolution layers interspersed by
ReLU activation and max pooling layers, followed by a fully-connected layer
at the end. The input to the network is a 32x32 pixel color image, which will 
be classified into one of the 10 output classes.

Files:
  main.c - example source code
  timer.c - implementation of helper functions for measuring inference time
  timer.h - declarations of helper functions for measuring inference time


Toolchains supported
====================
- MCUXpresso IDE
- IAR Embedded Workbench for ARM
- Keil uVision MDK
- ArmGCC - GNU Tools ARM Embedded

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050/EVK-IMXRT1060/EVK-IMXRT1170 board
- Personal computer (Windows)
- Python 3.x installed

Board settings
==============
No special settings are required.

Prepare the demo
================
0. Create a folder "dataset" in the root directory of the project.

1. Download the CIFAR10 dataset from https://pjreddie.com/media/files/cifar.tgz
   and copy the first 10 images from the "test" subfolder into "dataset".

2. Use the python script "scripts\glow_process_image.py" to transform an input
   image into a C array data and save it as "source\input_image.inc" which is
   included from main.cpp. As an example we will use the sample image "1_ship.png"
   which contains a "ship":

python scripts\glow_process_image.py ^
    -input-path=dataset\1_ship.png ^
    -output-path=source\input_image.inc ^
    -image-mode=0to1 -image-layout=NCHW -image-channel-order=BGR

3. Import the project in IDE and link the Glow bundle by performing the following steps:

   For MCUXpresso IDE:
     - Right click on the project -> "Properties" 
     - Select "C/C++ Build" -> "Settings" 
     - In the "Tool Setting" tab select "MCU C++ Linker" -> "Miscellaneous" 
     - Add the bundle to "Other objects" panel by clicking "Add..." and specify
       the relative path to the bundle in the project: "../source/cifar10.o"

   For IAR Embedded Workbench:
     - Right click on the "source" folder -> Add -> Add Files...
     - Choose file category "Library/Object Files (*.r;*.a;*.lib;*.o)"
     - Select the object file "cifar10.o" located in "source" folder

   For Keil uVision MDK:
     - Right click on the target -> "Options for Target ..."
     - Select "Linker" tab
     - In the "Misc controls" section add to the existing string with a 
       separating space (" ") character the string "../source/cifar10.o"

   For Armgcc:
     - Edit the "CMakeLists.txt" file from the "armgcc" directory by adding the
       following line at the end of the file:
         target_link_libraries(glow_cifar10.elf ${ProjDirPath}/../source/cifar10.o)

4. Build the project.

5. Connect a USB cable between the host PC and the OpenSDA USB port on the target board.

6. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

7. Download the program to the target board.

Running the demo
================
The log below shows the output of the Release version in the terminal window:

Top1 class = 8 (ship)
Confidence = 0.728
Inference time = 24 (ms)
