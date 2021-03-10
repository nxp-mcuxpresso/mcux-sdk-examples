Overview
========
This example shows the PowerQuad based CMSIS DSP function performance.
It could be compared with the project powerquad_benckmark_sw_matrix to show the PowerQuad performance improvement.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

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
CMSIS DSP benchmark matrix test start.
arm_mat_add_q15Test: xxx
arm_mat_add_q31Test: xxx
arm_mat_add_f32Test: xxx
arm_mat_sub_q15Test: xxx
arm_mat_sub_q31Test: xxx
arm_mat_sub_f32Test: xxx
arm_mat_mult_q15Test: xxx
arm_mat_mult_q31Test: xxx
arm_mat_mult_f32Test: xxx
arm_mat_inverse_f32Test: xxx
arm_mat_trans_q15Test: xxx
arm_mat_trans_q31Test: xxx
arm_mat_trans_f32Test: xxx
arm_mat_scale_q15Test: xxx
arm_mat_scale_q31Test: xxx
arm_mat_scale_f32Test: xxx

CMSIS DSP benchmark matrix test succeeded.
~~~~~~~~~~~~~~~~~~~~~
