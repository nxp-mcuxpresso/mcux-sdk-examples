Overview
========
Demonstrates inference for models compiled using the GLOW AOT tool and uses
a camera to generate data for inferencing.

The network used in this is based on the CIFAR-10 example in Caffe2 [1] & [2].

[1] https://github.com/caffe2/tutorials/blob/master/CIFAR10_Part1.ipynb
[2] https://github.com/caffe2/tutorials/blob/master/CIFAR10_Part2.ipynb


The neural network consists of 3 convolution layers interspersed by
ReLU activation and max pooling layers, followed by a fully-connected layer
at the end. The input to the network is a 32x32 pixel color image extracted 
from camera data, which will be classified into one of the 10 output classes.

This project does not include the pre-trained model or the training script
since Caffe2 framework is deprecated and lately has become part of PyTorch.
This project example only includes the bundle (binary) generated after running
the Glow AOT tool and is intended to be used as-is. The Glow bundle is the same as the
"glow_cifar10" MCUXpresso SDK example in the SDK. See that project's readme.txt
 for more details on model generation. 

 If you want a step-by-step example of running the Glow AOT tool for a given model 
 take a look at the LeNet MNIST Glow example and the Glow Getting Started Lab:
 https://community.nxp.com/t5/eIQ-Machine-Learning-Software/eIQ-Glow-Lab-for-i-MX-RT/ta-p/1123119


Toolchains supported
- MCUXpresso IDE
- IAR Embedded Workbench for ARM
- Keil uVision MDK
- ArmGCC - GNU Tools ARM Embedded



SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1170 board
- Personal computer
- RK055AHD091 or RK055MHD091 display (optional)
- OV5640 camera (optional)

Board settings
==============
Connect the display to J48 (optional)
Connect the camera to J2 (optional)
Connect external 5V power to J43, set J38 to 1-2

Prepare the Demo
================
1. Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Build the project. (The project expects the RK055MHD091 panel by default. To use the RK055AHD091 panel,
    change #define DEMO_PANEL DEMO_PANEL_RK055MHD091 to #define DEMO_PANEL MIPI_PANEL_RK055AHD091
    in eiq_display_conf.h.)
4. Download the program to the target board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Use the LCD screen to point the camera at images of airplanes, ships, deer, frogs, cars, and other 
images that can be categorized by CIFAR10. Some images will work better than others, and a few example
images have been provided in the PDF in the /doc folder. For best results, the flashing selection rectangle should 
be centered on the image and nearly (but not completely) fill up the whole rectangle. The camera should be stabilized with your finger
or by some other means to prevent shaking. Also ensure the camera lens has been focused as described in the instructions
when connecting the camera and LCD 
(https://community.nxp.com/t5/i-MX-RT-Knowledge-Base/Connecting-camera-and-LCD-to-i-MX-RT-EVKs/tac-p/1122184). 

You will see the result of the inference on the LCD screen as well as the serial terminal. The result printed
on the LCD screen has a minimum threshold applied to it. 
