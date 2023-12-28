Overview
========
This example shows how to use powerquad driver vector functions.
The powerquad driver API results are compared to the reference result.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the cpu board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the example.

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
