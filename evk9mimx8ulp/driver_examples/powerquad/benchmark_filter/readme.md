Overview
========
This example shows the PowerQuad based CMSIS DSP function performance.
It could be compared with the project powerquad_benckmark_sw_filter to show the PowerQuad performance improvement.

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
5.  Running the example with with imx-mkimage.(Not running it with your IDE, unless it will be failed to running).
    Note: After a successful execution, if you need to execute again, you need to completely power down the board and restart it again.

Running the demo
================
NOTE: Depending on the toolchain configuration, the benchmark result might be different.
~~~~~~~~~~~~~~~~~~~~~
BOARD_ReleaseTRDC: 75 start release trdc
BOARD_ReleaseTRDC: 78 finished release trdc, status = 0xd6
BOARD_SetTrdcGlobalConfig: 93 start setup trdc
BOARD_SetTrdcGlobalConfig: 402 finished setup trdc

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
