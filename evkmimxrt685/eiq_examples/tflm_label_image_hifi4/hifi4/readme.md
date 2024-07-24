Overview
========
TensorFlow Lite model based implementation of object detector based on
TensorFlow Lite example [2] adjusted to run on MCUs.

A 3-channel color image is set as an input to a quantized Mobilenet
convolutional neural network model [1] that classifies the input image into
one of 1000 output classes.

Firstly a static stopwatch image is set as input regardless camera is connected or not.
Secondly runtime image processing from camera in the case camera and display
is connected. Camera data are displayed on LCD.

HOW TO USE THE APPLICATION:
To classify an image, place an image in front of camera so that it fits in the
white rectangle in the middle of the display.
Note semihosting implementation causes slower or discontinuous video experience. 
Select UART in 'Project Options' during project import for using external debug console 
via UART (virtual COM port).

[1] https://www.tensorflow.org/lite/models
[2] https://github.com/tensorflow/tensorflow/tree/r2.3/tensorflow/lite/examples/label_image

Files:
  main.cpp - example main function
  image_data.h - image file converted to a C language array of RGB values
    using Python with the OpenCV and Numpy packages:
    import cv2
    import numpy as np
    img = cv2.imread('stopwatch.bmp')
    img = cv2.resize(img, (128, 128))
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    with open('image_data.h', 'w') as fout:
      print('#define STATIC_IMAGE_NAME "stopwatch"', file=fout)
      print('static const uint8_t image_data[] = {', file=fout)
      img.tofile(fout, ', ', '0x%02X')
      print('};\n', file=fout)
  labels.h - names of object classes
  mobilenet_v1_0.25_128_quant_int8.tflite - pre-trained TensorFlow Lite model quantized
    using TF Lite converter (for more details see the eIQ TensorFlow Lite User's Guide, which
    can be downloaded with the MCUXpresso SDK package)
    (source: http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_128.tgz)
  stopwatch.bmp - image file of the object to recognize
    (source: https://commons.wikimedia.org/wiki/File:Stopwatch2.jpg)
  timer.c - timer source code
  image/* - image capture and pre-processing code
  model/get_top_n.cpp - top results retrieval
  model/model_data.h - model data from the .tflite file
    converted to a C language array using the xxd tool (distributed
    with the Vim editor at www.vim.org)
  model/model.cpp - model initialization and inference code
  model/model_mobilenet_ops.cpp - model operations registration
  model/output_postproc.cpp - model output processing
  video/* - camera and display handling


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- Xtensa C Compiler  14.01

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT685 board
- Personal computer

Board settings
==============

Prepare the Demo
================
1. Build the HiFi4 project first to create the binary image.
2. Replace the DSP binary files in the Cortex-M33 project with the files
   generated into the <example_root>/hifi4/binary directory if necessary.
3. Continue with the Cortex-M33 master project.
4. Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
5. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
6. Download the program to the target board.
7. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the demo in the terminal window (compiled with ARM GCC):

Label image object recognition example using a TensorFlow Lite Micro model.
Detection threshold: 23%
Expected category: stopwatch
Model: mobilenet_v1_0.25_128_quant_int8

Static data processing:
----------------------------------------
     Inference time: 88 ms
     Detected:  stopwatch (87%)
----------------------------------------


Camera data processing:
Data for inference are ready
----------------------------------------
     Inference time: 88 ms
     Detected: No label detected (0%)
----------------------------------------

Data for inference are ready
----------------------------------------
     Inference time: 88 ms
     Detected:     jaguar (92%)
----------------------------------------

Data for inference are ready
----------------------------------------
     Inference time: 88 ms
     Detected:  pineapple (97%)
----------------------------------------
