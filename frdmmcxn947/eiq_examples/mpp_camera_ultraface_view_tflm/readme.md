Overview
========

This example shows how to use the library to create a use-case for
face detection using camera as source.

The machine learning framework used for this example is TensorFlow Lite Micro.
The face detection model used is quantized Ultraface slim model that detects multiple faces in an input image.

Toolchains supported
- MCUXpresso, version 11.8.0
- GCC Arm Embedded, version 12.2.Rel1


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- MCUXpresso  11.9.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- FRDM-MCXN947 board
- Mini/micro USB cable
- Personal computer
- OV7670 camera
- MikroElektronika TFT Proto 5" display (800x480)

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
- For each detected face, a labeled rectangle should be displayed on the screen.
- Logs below should be displayed on the debug console.

Logs for camera_ultraface_view example using TensorFlow Lite Micro model should look like this:

[MPP_VERSION_1.0.0]
Inference Engine: TensorFlow-Lite Micro
inference time 707 ms
ultraface : no face detected
inference time 707 ms
ultraface : no face detected
inference time 707 ms
ultraface : no face detected
inference time 706 ms
ultraface : box 0 label face score 99(%)
inference time 706 ms
ultraface : box 0 label face score 99(%)
inference time 706 ms
ultraface : box 0 label face score 99(%)
inference time 706 ms
ultraface : box 0 label face score 99(%)
inference time 710 ms
ultraface : box 0 label face score 99(%)
inference time 710 ms
ultraface : box 0 label face score 99(%)
inference time 710 ms

Important notes

TensorFLow Lite Micro is an optional engine for the ML Inference component of MPP.
This project embeds NXP's custom TensorFlow Lite Micro code by default.
TensorFLow Lite allows short-listing the "Operations" used by a specific model in order to reduce the binary image footprint.
This is done by implementing the function:

tflite::MicroOpResolver &MODEL_GetOpsResolver()

This example implements its own function MODEL_GetOpsResolver dedicated to Ultraface.
User may provide its own implementation of MODEL_GetOpsResolver when using a different model.
