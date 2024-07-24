Overview
========

This example shows how to use the library to create a use-case for
person detection using a camera or a static image as source.

The machine learning framework used for this example is TensorFlow Lite Micro.
The person detection model used is quantized persondetect model that detects multiple persons in an input image.

Toolchains supported
- MCUXpresso, version 11.9.0
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
- For each detected person, a rectangle should be displayed on the screen.
- Logs below should be displayed on the debug console.

Logs for camera_persondetect_view example using TensorFlow Lite Micro model should look like this:

[MPP_VERSION_2.1.e1dc37e]
Inference Engine: TensorFlow-Lite Micro 

Element stats --------------------------
Persondetect : exec_time 405 ms
Number of detections : 1
box 0 --> score 37 %
 Left=64, Top=49, Right=115, Bottom=128
-------------------------------------------

Element stats --------------------------
Persondetect : exec_time 405 ms
No person detected

Element stats --------------------------
Persondetect : exec_time 405 ms
Number of detections : 1
box 0 --> score 43 %
 Left=63, Top=55, Right=113, Bottom=128
-------------------------------------------

Element stats --------------------------
Persondetect : exec_time 403 ms
Number of detections : 1
box 0 --> score 37 %
 Left=67, Top=55, Right=112, Bottom=128
-------------------------------------------

Element stats --------------------------
Persondetect : exec_time 405 ms
No person detected

Element stats --------------------------
Persondetect : exec_time 405 ms
No person detected

Important notes

TensorFLow Lite Micro is an optional engine for the ML Inference component of MPP.
This project embeds NXP's custom TensorFlow Lite Micro code by default.
TensorFLow Lite allows short-listing the "Operations" used by a specific model in order to reduce the binary image footprint.
This is done by implementing the function:

tflite::MicroOpResolver &MODEL_GetOpsResolver()

This example implements its own function MODEL_GetOpsResolver dedicated to persondetect.
User may provide its own implementation of MODEL_GetOpsResolver when using a different model.
