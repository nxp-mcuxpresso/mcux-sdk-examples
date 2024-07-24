Overview
========

This example shows how to use the library to create one object detection using a file as source.

The machine learning framework used is TensorFlow Lite Micro.
The object detection model used is quantized Nanodet M model that detects multiple objects in an input image.
The model has 80 classes.

Toolchains supported
- MCUXpresso, version 11.8.0
- GCC Arm Embedded, version 12.2.Rel1


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal computer
- MT9M114 camera (optional)
- RK043FN02H-CT display (optional)

Board settings
==============
Connect the camera to J35 (optional)
Connect the display to A1-A40 and B1-B6 (optional)
Connect external 5V power supply to J2, set J1 to 1-2

Prepare the Demo
================
1. Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

EXPECTED OUTPUTS:
The expected outputs of the example are:
- For each detected object, a labeled rectangle should be displayed on the screen
- Logs below should be displayed on the debug console

Logs for static_image_nanodet_m_view example using TensorFlow Lite Micro model should look like this:

Inference Engine: TensorFlow-Lite Micro
Element stats --------------------------
nanodet : exec_time 716 ms
nanodet : box 0 label person score 82(%)
Element stats --------------------------
nanodet : exec_time 718 ms
nanodet : box 0 label person score 82(%)
Element stats --------------------------
nanodet : exec_time 716 ms
nanodet : box 0 label person score 82(%)
Element stats --------------------------
nanodet : exec_time 717 ms
nanodet : box 0 label person score 82(%)

Important notes

TensorFLow Lite Micro is an optional engine for the ML Inference component of MPP.
This project embeds NXP's custom TensorFlow Lite Micro code by default.
TensorFLow Lite allows short-listing the "Operations" used by a specific model in order to reduce the binary image footprint.
This is done by implementing the function:

tflite::MicroOpResolver &MODEL_GetOpsResolver()

This example implements its own function MODEL_GetOpsResolver dedicated to Nanodet.
User may provide its own implementation of MODEL_GetOpsResolver when using a different model.
