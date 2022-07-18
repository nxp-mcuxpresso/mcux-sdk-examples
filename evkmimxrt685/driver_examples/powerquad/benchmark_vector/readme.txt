Overview
========
This example shows how to use powerquad driver vector functions.
The powerquad driver API results are compared to the reference result.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
NOTE: Depending on the toolchain configuration, the benchmark result might be different.
~~~~~~~~~~~~~~~~~~~~~
CMSIS DSP benchmark vector test start.
PQ_VectorLnF32Test: xxx
PQ_VectorLnQ31Test: xxx
PQ_VectorInvF32Test: xxx
PQ_VectorInvQ31Test:  xxx
PQ_VectorSqrtF32Test: xxx
PQ_VectorSqrtQ31Test: xxx
PQ_VectorInvSqrtF32Test: xxx
PQ_VectorInvSqrtQ31Test: xxx
PQ_VectorEtoxF32Test: xxx
PQ_VectorEtoxQ31Test: xxx
PQ_VectorEtonxF32Test: xxx
PQ_VectorEtonxQ31Test: xxx
PQ_BiquadCascadedf2F32Test: xxx
PQ_BiquadCascadedf2Q31Test: xxx
PQ_VectorLnFPTest: xxx
PQ_VectorLnFXTest: xxx
PQ_VectorInvFPTest: xxx
PQ_VectorInvFXTest: xxx
PQ_VectorSqrtFPTest: xxx
PQ_VectorSqrtFXTest: xxx
PQ_VectorInvSqrtFPTest: xxx
PQ_VectorInvSqrtFXTest: xxx
PQ_VectorEtoxFPTest: xxx
PQ_VectorEtoxFXTest: xxx
PQ_VectorEtonxFPTest: xxx
PQ_VectorEtonxFXTest: xxx
PQ_16ByteBiquadCascadedf2FPTest: xxx
PQ_16ByteBiquadCascadedf2FXTest: xxx
PQ_VectorSqrtQ31Test: xxx
PQ_VectorSqrtQ15Test: xxx
PQ_VectorSinF32Test: xxx
PQ_VectorSinQ31Test: xxx
PQ_VectorSinQ15Test: xxx
PQ_VectorCosF32Test: xxx
PQ_VectorCosQ31Test: xxx
PQ_VectorCosQ15Test: xxx

CMSIS DSP benchmark vector test succeeded.
~~~~~~~~~~~~~~~~~~~~~
