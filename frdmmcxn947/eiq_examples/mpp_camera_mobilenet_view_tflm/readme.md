Overview
========

This example shows how to use the library to create a use-case for
image classification using camera as source.

The machine learning frameworks used are TensorFlow Lite Micro, GLOW or DeepViewRT
The image classification model used is quantized Mobilenet
convolutional neural network model [1] that classifies the input image into
one of 1000 output classes.

[1] https://www.tensorflow.org/lite/models

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

If this example uses TensorFLow Lite Micro (i.e. not Glow, not deepViewRT), it implements its own function MODEL_GetOpsResolver dedicated to Mobilenet.
User may provide its own implementation of MODEL_GetOpsResolver when using a different model.
