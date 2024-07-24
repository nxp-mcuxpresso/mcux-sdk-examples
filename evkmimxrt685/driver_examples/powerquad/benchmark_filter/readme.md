Overview
========
This example shows the PowerQuad based CMSIS DSP function performance.
It could be compared with the project powerquad_benckmark_sw_filter to show the PowerQuad performance improvement.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

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
CMSIS DSP benchmark filter test start.
arm_fir_q15Test: xxx
arm_fir_q31Test: xxx
arm_fir_f32Test: xxx
arm_conv_q15Test: xxx
arm_conv_q31Test: xxx
arm_conv_f32Test: xxx
arm_correlate_q15Test: xxx
arm_correlate_q31Test: xxx
arm_correlate_f32Test: xxx
biquad_cascade_f32Test: xxx

CMSIS DSP benchmark filter test succeeded.
~~~~~~~~~~~~~~~~~~~~~
