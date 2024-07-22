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
- MIMXRT1170-EVKB board
- Personal computer
- RK055AHD091 or RK055MHD091 display
- OV5640 camera

Board settings
==============
Connect the display to J48
Connect the camera to J2
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
    in display_support.h.)
4. Download the program to the target board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
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
