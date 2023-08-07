Overview
========
Convolutional neural network (CNN) example with the use of
convolution, ReLU activation, pooling and fully-connected functions.

The CNN model in the example was trained using the scripts available at [1]
with the CifarNet model. 
The configuration of the model was modified to match the neural 
network structure in the CMSIS-NN CIFAR-10 example.
The example source code is a modified version of the Label Image
example from the TensorFlow Lite examples [2], adjusted to run on MCUs.
The neural network consists of 3 convolution layers interspersed by
ReLU activation and max pooling layers, followed by a fully-connected layer
at the end. The input to the network is a 32x32 pixel color image, which is 
classified into one of the 10 output classes. The model size is 91 KB.

Firstly a static ship image is used as input regardless camera is connected or not.
Secondly runtime image processing from camera in the case camera and display
is connected. Camera data are displayed on LCD. 

HOW TO USE THE APPLICATION:
To classify an image, place an image in front of the camera so that it fits in the
white rectangle in the middle of the LCD display. 
Note that semihosting implementation causes slower or discontinuous video experience. 
Select UART in 'Project Options' during project import for using external debug console 
via UART (virtual COM port).

[1] https://github.com/tensorflow/models/tree/master/research/slim
[2] https://github.com/tensorflow/tensorflow/tree/r2.3/tensorflow/lite/examples/label_image

Files:
  main.cpp - example main function
  ship.bmp - shrinked picture of the object to recognize
    (source: https://en.wikipedia.org/wiki/File:Christian_Radich_aft_foto_Ulrich_Grun.jpg)
  image_data.h - image file converted to a C language array of RGB values
    using Python with the OpenCV and Numpy packages:
    import cv2
    import numpy as np
    img = cv2.imread('ship.bmp')
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    with open('image_data.h', 'w') as fout:
      print('#define STATIC_IMAGE_NAME "ship"', file=fout)
      print('static const uint8_t image_data[] = {', file=fout)
      img.tofile(fout, ', ', '0x%02X')
      print('};\n', file=fout)
  timer.c - timer source code
  image/* - image capture and pre-processing code
  model/get_top_n.cpp - top results retrieval
  model/model_data.h - model data converted from a .tflite file to a C language
    array using the xxd tool (distributed with the Vim editor at www.vim.org)
  model/model.cpp - model initialization and inference code
  model/model_cifarnet_ops.cpp - model operations registration
  model/output_postproc.cpp - model output processing
  video/* - camera and display handling


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1060 board
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
The log below shows the output of the demo in the terminal window (compiled with ARM GCC):

CIFAR-10 object recognition example using a TensorFlow Lite model.
Detection threshold: 60%
Expected category: ship
Model: cifarnet_quant

Static data processing:
----------------------------------------
     Inference time: 65 ms
     Detected:       ship (100%)
----------------------------------------


Camera data processing:
Data for inference are ready
----------------------------------------
     Inference time: 64 ms
     Detected:        dog (95%)
----------------------------------------

Data for inference are ready
----------------------------------------
     Inference time: 65 ms
     Detected: No label detected (0%)
----------------------------------------

Data for inference are ready
----------------------------------------
     Inference time: 64 ms
     Detected:      horse (60%)
----------------------------------------
