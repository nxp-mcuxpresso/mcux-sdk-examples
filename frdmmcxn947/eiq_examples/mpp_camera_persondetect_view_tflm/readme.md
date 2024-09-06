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
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- FRDM-MCXN947 board
- Mini/micro USB cable
- Personal computer
- OV7670 camera
- NXP LCD-PAR-S035 display 320x480 (default)
- MikroElektronika TFT Proto 5" display 800x480

Board settings
==============
In order to route the camera data signals to chip,
some Solder Jumpers (SJ) on the back of the board must be changed:
move SJ16, SJ26, SJ27 from A side to B side,
else the captured image would appear redish.

Connect the OV7670 module to camera pins (match 3V3 pins)
Connect the display to FLEXIO LCD pins (panel pins from GND up to D15)

Prepare the Demo
================
1. Connect a USB cable between the host PC and FRDM-MCXN947 board.
2. Debug Console:
   - available with TTY device over USB MCU-Link. 
3. For console over UART: Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
4. Build the project using the MCUX IDE (axf image) or armgcc (elf image).
5. Download the program to the target board using the MCUX IDE.
6. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
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
