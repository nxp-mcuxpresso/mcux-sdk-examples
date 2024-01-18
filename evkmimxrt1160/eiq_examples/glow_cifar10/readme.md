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
- MCUXpresso IDE
- IAR Embedded Workbench for ARM
- Keil uVision MDK
- ArmGCC - GNU Tools ARM Embedded


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the Release version in the terminal window:

Top1 class = 8 (ship)
Confidence = 0.728
Inference time = 24 (ms)
