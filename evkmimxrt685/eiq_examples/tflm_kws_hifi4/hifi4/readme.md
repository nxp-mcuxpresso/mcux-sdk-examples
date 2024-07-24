Overview
========
Keyword spotting example based on Keyword spotting for Microcontrollers [1].

Input data preprocessing

Raw audio data is pre-processed first - a spectrogram is calculated: A 40 ms
window slides over a one-second audio sample with a 20 ms stride. For each
window, audio frequency strengths are computed using FFT and turned into
a set of Mel-Frequency Cepstral Coefficients (MFCC). Only first 10 coefficients
are taken into account. The window slides over a sample 49 times, hence
a matrix with 49 rows and 10 columns is created. The matrix is called a spectrogram.

In the example, static audio samples ("off", "right") are evaluated first
regardless microphone is connected or not. Secondly, audio is processed directly
from microphone.

Classification

The spectrogram is fed into a neural network. The neural network is a depthwise
separable convolutional neural network based on MobileNet described in [2].
The model produces a probability vector for the following classes:
"Silence", "Unknown", "yes", "no", "up", "down", "left", "right", "on", "off",
"stop" and "go".

Quantization

The NN model is quantized to run faster on MCUs and it takes in a quantized
input and produces a quantized output. An input spectrogram needs to be scaled
from range [-247, 30] to range [0, 255] and round to integers. Values lower
than zero are set to zero and values exceeding 255 are set to 255. An output
of the softmax function is a vector with components in the interval (0, 255)
and the components will add up to 255).   

HOW TO USE THE APPLICATION:
Say different keyword so that microphone can catch them. Voice recorded from
the microphone can be heared using headphones connected to the audio jack.
Note semihosting implementation causes slower or discontinuous audio experience. 
Select UART in 'Project Options' during project import for using external debug
console via UART (virtual COM port).

[1] https://github.com/ARM-software/ML-KWS-for-MCU
[2] https://arxiv.org/abs/1704.04861

Files:
  main.cpp - example main function
  ds_cnn_s.tflite - pre-trained TensorFlow Lite model converted from DS_CNN_S.pb
    (source: https://github.com/ARM-software/ML-KWS-for-MCU/blob/master/Pretrained_models/DS_CNN/DS_CNN_S.pb)
    (for details on how to quantize and convert a model see the eIQ TensorFlow Lite
    User's Guide, which can be downloaded with the MCUXpresso SDK package)
  off.wav - waveform audio file of the word to recognize
    (source: Speech Commands Dataset available at
    https://storage.cloud.google.com/download.tensorflow.org/data/speech_commands_v0.02.tar.gz,  
    file speech_commands_test_set_v0.02/off/0ba018fc_nohash_2.wav)
  right.wav - waveform audio file of the word to recognize
    (source: Speech Commands Dataset available at
    https://storage.cloud.google.com/download.tensorflow.org/data/speech_commands_v0.02.tar.gz,
    file speech_commands_test_set_v0.02/right/0a2b400e_nohash_1.wav)
  audio_data.h - waveform audio files converted into a C language array of audio signal
    values ("off", "right") audio signal values using Python with the Scipy package:
    from scipy.io import wavfile
    rate, data = wavfile.read('yes.wav')
    with open('wav_data.h', 'w') as fout:
      print('#define WAVE_DATA {', file=fout)
      data.tofile(fout, ',', '%d')
      print('}\n', file=fout)
  train.py - model training script based on https://www.tensorflow.org/tutorials/audio/simple_audio
  timer.c - timer source code
  audio/* - audio capture and pre-processing code
  audio/mfcc.cpp - MFCC feature extraction matching the TensorFlow MFCC operation
  audio/kws_mfcc.cpp - ausio buffer handling for MFCC feature extraction
  model/get_top_n.cpp - top results retrieval
  model/model_data.h - model data from the ds_cnn_s.tflite file converted to
    a C language array using the xxd tool (distributed with the Vim editor
    at www.vim.org)
  model/model.cpp - model initialization and inference code
  model/model_ds_cnn_ops.cpp - model operations registration
  model/output_postproc.cpp - model output processing


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
The log below shows the output of the demo in the terminal window:

Keyword spotting example using a TensorFlow Lite model.
Detection threshold: 25

Static data processing:
Expected category: off
----------------------------------------
     Inference time: 32 ms
     Detected:        off (100%)
----------------------------------------

Expected category: right
----------------------------------------
     Inference time: 32 ms
     Detected:      right (98%)
----------------------------------------


Microphone data processing:
----------------------------------------
     Inference time: 32 ms
     Detected: No label detected (0%)
----------------------------------------

----------------------------------------
     Inference time: 32 ms
     Detected:         up (85%)
----------------------------------------

----------------------------------------
     Inference time: 32 ms
     Detected:       left (97%)
----------------------------------------
