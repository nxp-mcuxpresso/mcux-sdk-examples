Overview
========
Enable multicore support for TensorFlow Lite for Microcontrollers

The label_image application is run on Cortex-M7 core, and kws application is run
on Cortex-M4 core. M7 core starts up first and loads M4 core image into SDRAM (0x83000000),
then M4 core starts up using the image in SDRAM.

At first the Cortex-M7 core is in sleep mode, the LCD is turned off, the kws application
is detecting audio. If the valid audio is detected, the kws application will send MU
message to wakeup the Cortex-M7 core, the label_image application will start to run
object detection.

By default, M4 core wakes up the M7 core via speaking any keyword. To use specific keyword,
user can follow below steps:
Open file "source-->model-->output_postproc.cpp", comment the line "#define WAKEUP_WITH_ANY_KEYWORD".

In this application, there are two projects, one is tensorflow_lite_micro_multicore_cm4 for M4 core
(running kws), another is tensorflow_lite_micro_multicore_cm7 for M7 core(running label_image),
M7 core application depends on the M4 core, So user could build the M4 core application before M7 core.

[1] https://www.tensorflow.org/lite/models
[2] https://github.com/tensorflow/tensorflow/tree/r2.3/tensorflow/lite/examples/label_image
[3] https://github.com/ARM-software/ML-KWS-for-MCU


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
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
The log below shows the output of the demo in the terminal window (compiled with ARM GCC):

Label image object recognition example using a TensorFlow Lite Micro model.
Detection threshold: 23%
Expected category: stopwatch
Model: mobilenet_v1_0.25_128_quant_int8

Static data processing:
----------------------------------------
     Inference time: 81 ms
     Detected:  stopwatch (87%)
----------------------------------------


Camera data processing:
Data for inference are ready
----------------------------------------
     Inference time: 81 ms
     Detected: No label detected (0%)
----------------------------------------

Data for inference are ready
----------------------------------------
     Inference time: 81 ms
     Detected:     jaguar (92%)
----------------------------------------

Data for inference are ready
----------------------------------------
     Inference time: 81 ms
     Detected:  pineapple (97%)
----------------------------------------
CPU enter Sleep mode ...
