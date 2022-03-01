Overview
================
DeepViewRT Camera label iamge Demo for IMXRT. This project shows the demonstration of real-time object recognition using DeepViewRT on an IMXRT platform together with a camera and LCD module.  The firmware uses a user defined model and real-time camera to classify object.

This program is a deepviewrt demonstration program which uses the SDRAM/TCM to allocate the memory pool. The cache can be allocated on SDRAM or TCM, the performance will be different then.The firmware will use PXP API's to crop the region of interest (ROI) to appropriate size for the neural network (resize by calls to PXP API's). The results of labels can be shown in the LCD, and some other processed results will be redirected to UART console and/or toolchain GUI console. 

Cache can be placed on DTC or SDRAM.

Experiments
===========
The deepview-rt-cortex-m7f library is located in 'libs' folder. By default, a pre-trained mobilenetv1 width multiplier of 0.25, pixel of 160x160 model for classification is used and located in 'models' folder.

Files:
  main.cpp - example main function
  image_data.h - image file converted to a C language array of RGB values
    using Python with the OpenCV and Numpy packages:
    import cv2
    import numpy as np
    img = cv2.imread('giant_panda.jpeg')
    img = cv2.resize(img, (160, 160))
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    with open('image_data.h', 'w') as fout:
      print('#define STATIC_IMAGE_NAME "giant panda"\n', file=fout)
      print('static const uint8_t image_data[] = {', file=fout)
      img.tofile(fout, ', ', '0x%02X')
      print('};\n', file=fout)
  mobilenet_v1_0.25_160.rtm - pre-trained DeepviewRT model
  video/* - camera and display handling

Running the demo
================
The log below shows the output of the demo in the terminal window (compiled with MDK):

==========================================================================
                    DeepviewRT camera label image Demo
===========================================================================

Static data processing:
Label index = 389, score = 0.986, runtime: 195
        MATCH - giant panda - 389

Camera data processing:

Data for inference are ready
Label index = 795, score = 0.050, runtime: 202
        NO MATCH - shower curtain - 795

Data for inference are ready
Label index = 795, score = 0.061, runtime: 201
        NO MATCH - shower curtain - 795

Data for inference are ready
Label index = 509, score = 0.403, runtime: 201
        MATCH - computer keyboard - 509

