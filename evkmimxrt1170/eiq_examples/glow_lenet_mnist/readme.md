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
- MIMXRT1170-EVK board
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

Top1 class = 9
Confidence = 0.942
Inference time = 10 (ms)

Notes
The inference time depends on the board.
For example you can expect the following inference time for the following boards:

RT10xx: Inference time: 10 (ms)
RT11xx: Inference time: 6 (ms)
