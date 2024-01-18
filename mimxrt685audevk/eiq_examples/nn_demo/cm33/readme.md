Overview
========

The dsp_nn_demo demo application demonstrates starting DSP core with DSP image and accelerating some neural networks layers using the DSP.
This demo will just call some routines to offload Neural Networks processing on the HiFi4. The example calls are these:
ECHO: Just a round-trip
CONV DS: Convolution depthwise
CONV STD: Convolution standard
RELU: Rectified Linear Unit
MAXPOOL: Max Pooling
All the layer examples are executed in 2 flavours: synchronous and asynchronous.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- JTAG/SWD
- MIMXRT685-AUD-EVK board
- Personal Computer

Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program for CM33 core to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
5.  Detach from board.
6.  Download the program for DSP core to the target board.
7.  Launch the debugger in your IDE to begin running the demo.

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display the following:

Started NN UnitTests and benchmarks (10 iterations)
DSP Image copied to SRAM
Running ECHO SYNC
ECHO unit test succeeded
NNlib version 2.1
Avg Inference cycles: 4626 time: 0.018 ms
Throughput: 55555.6 fps
Running CONV DS SYNC
CONV DS output check succeeded
Avg Inference cycles: 921388 time: 3.684 ms
Throughput: 271.4 fps
Running CONV DS ASYNC
CONV DS output check succeeded
Avg Inference cycles: 926659 time: 3.705 ms
Throughput: 269.9 fps
Running CONV STD SYNC
CONV STD output check succeeded
Avg Inference cycles: 1755775 time: 7.020 ms
Throughput: 142.5 fps
Running CONV STD ASYNC
CONV STD output check succeeded
Avg Inference cycles: 1758458 time: 7.030 ms
Throughput: 142.2 fps
Running RELU SYNC
RELU unit test succeeded
Avg Inference cycles: 6366 time: 0.025 ms
Throughput: 40000.0 fps
Running RELU ASYNC
RELU unit test succeeded
Avg Inference cycles: 9028 time: 0.036 ms
Throughput: 27777.8 fps
Running MAXPOOL SYNC
MAXPOOL output check succeeded
Avg Inference cycles: 21362 time: 0.085 ms
Throughput: 11764.7 fps
Running MAXPOOL ASYNC
MAXPOOL output check succeeded
Avg Inference cycles: 24012 time: 0.096 ms
Throughput: 10416.7 fps
