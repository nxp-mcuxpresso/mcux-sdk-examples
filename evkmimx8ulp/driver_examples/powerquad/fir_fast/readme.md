Overview
========
The PowerQuad fetches data from system RAM and private RAM through different
path, for the calculations which need data from two parts of memory, such as
FIR, convolve, correlate, and some matrix operations, place the input data B
in private RAM improves the performance.
PowerQuad private RAM starts from address 0xe0000000, the first 4KByte is used
by PowerQuad driver, the RAM starts from 0xe0001000 could be used by user
for optimization. In this example, the FIR taps, convolve input data B, and correlate
input data B are not changed, so they are converted to float format and saved
to private RAM at the beginning.
If the example runs successfully, the performance using optimized method is better
than the normal method.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

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
The log below shows the output of the powerquad_fir_fast example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOARD_ReleaseTRDC: 75 start release trdc
BOARD_ReleaseTRDC: 78 finished release trdc, status = 0xd6
BOARD_SetTrdcGlobalConfig: 93 start setup trdc
BOARD_SetTrdcGlobalConfig: 402 finished setup trdc
POWERQUAD FIR fast example started
PQ fir fixed 16-bit normal method: xxx
PQ fir fixed 16-bit fast method: xxx
PQ fir fixed 32-bit normal method: xxx
PQ fir fixed 32-bit fast method: xxx
PQ fir float normal method: xxx
PQ fir float fast method: xxx
PQ conv fixed 16-bit normal method: xxx
PQ conv fixed 16-bit fast method: xxx
PQ conv fixed 32-bit normal method: xxx
PQ conv fixed 32-bit fast method: xxx
PQ conv float normal method: xxx
PQ conv float fast method: xxx
PQ corr fixed 16-bit normal method: xxx
PQ corr fixed 16-bit fast method: xxx
PQ corr fixed 32-bit normal method: xxx
PQ corr fixed 32-bit fast method: xxx
PQ corr float normal method: xxx
PQ corr float fast method: xxx
POWERQUAD FIR fast example successed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
