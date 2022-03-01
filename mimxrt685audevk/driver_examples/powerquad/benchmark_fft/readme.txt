Overview
========
This example shows the PowerQuad based CMSIS DSP function performance.
It could be compared with the project powerquad_benckmark_sw_fft to show the PowerQuad performance improvement.

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
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
CMSIS DSP benchmark fft test start.
arm_rfft_q15Test: xxx
arm_rfft_q31Test: xxx
arm_cfft_q15Test: xxx
arm_cfft_q31Test: xxx
arm_ifft_q15Test: xxx
arm_ifft_q31Test: xxx
arm_dct4_q15Test: xxx
arm_dct4_q31Test: xxx

CMSIS DSP benchmark fft test succeeded.
~~~~~~~~~~~~~~~~~~~~~
