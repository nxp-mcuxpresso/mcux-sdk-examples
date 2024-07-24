Overview
========

This example shows how to use the library to create a use-case for
image classification using camera as source.

The machine learning framework used is TensorFlow Lite Micro
The image classification model used is quantized Mobilenet
convolutional neural network model [1] that classifies the input image into
one of 1000 output classes.

[1] https://www.tensorflow.org/lite/models

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
- EVK-MIMXRT1170 board
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
- Detected label should be displayed on the screen
- Pipeline tasks statistics are displayed on the console
- Logs below should be displayed on the debug console

Logs for camera_mobilenet_view example using TensorFlow Lite Micro model should look like this:

[MPP_VERSION_1.0.0]
                   Inference Engine: TensorFlow-Lite Micro
API stats ------------------------------
rc_cycle = 43 ms rc_cycle_max 99 ms
pr_slot  = 56 ms pr_rounds 1 app_slot 4 ms
MPP stats ------------------------------
mpp 80082C40 exec_time 43 ms
mpp 80082F20 exec_time 52 ms
Element stats --------------------------
mobilenet : exec_time 48 ms
mobilenet : No label detected (0%)
API stats ------------------------------
rc_cycle = 43 ms rc_cycle_max 99 ms
pr_slot  = 56 ms pr_rounds 1 app_slot 4 ms
MPP stats ------------------------------
mpp 80082C40 exec_time 43 ms
mpp 80082F20 exec_time 52 ms
Element stats --------------------------
mobilenet : exec_time 49 ms
mobilenet : mosquito net (32%)
API stats ------------------------------
rc_cycle = 43 ms rc_cycle_max 99 ms
pr_slot  = 56 ms pr_rounds 1 app_slot 4 ms
MPP stats ------------------------------
mpp 80082C40 exec_time 43 ms
mpp 80082F20 exec_time 52 ms

Important notes

TensorFLow Lite Micro is an optional engine for the ML Inference component of MPP.
This project embeds NXP's custom TensorFlow Lite Micro code by default.
TensorFLow Lite allows short-listing the "Operations" used by a specific model in order to reduce the binary image footprint.
This is done by implementing the function:

tflite::MicroOpResolver &MODEL_GetOpsResolver()

If this example uses TensorFLow Lite Micro, it implements its own function MODEL_GetOpsResolver dedicated to Mobilenet.
User may provide its own implementation of MODEL_GetOpsResolver when using a different model.
