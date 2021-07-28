Overview
========
This document explains the freertos_tickless example. It shows how the CPU enters the sleep mode and then
it is woken up either by expired time delay using low power timer module or by external interrupt caused by a
user defined button.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1024-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
After flashing the example to the board the console will start printing the number of tick count periodically
when the CPU is running.
To wake up the CPU by external interrupt, press the button, that is specified at the beginning of the
example (SWx where x is the number of the user defined button). When the button is pressed, the console prints
out the "CPU woken up by external interrupt" message.

Example output:

Press any key to start the example
Tickless Demo example
Press or turn on SWx to wake up the CPU

Tick count :
0
5000
10000
CPU woken up by external interrupt
15000
20000
25000

Explanation of the example
The example application prints the actual tick count number every time after the specified
delay. When the vTaskDelay() is called, the CPU enters the sleep mode for that defined period
of time.

While the CPU is in sleep mode and the user defined button is pressed, the CPU is woken up
by the external interrupt and continues to sleep after the interrupt is handled.
The period of time delay is not changed after the external interrupt occurs.

Note:
To debug in qspiflash, following steps are needed:
1. Select the flash target and compile.
2. Set the SW8: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J23.
3. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to qspiflash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
