Overview
========
This example shows how to use SDK drivers to use the Frequency Measure feature of SYSCON module.
It shows how to measure a target frequency using a (faster) reference frequency. The example uses the internal main clock as the reference frequency to measure the frequencies of the RTC, watchdog oscillator, and internal RC oscillator.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
The log below shows example output of the frequency measure demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Capture source: External clock (clk_in) (main system clock reference), reference frequency = 198000000 Hz
Computed frequency value = 23999410 Hz
Expected frequency value = 24000000 Hz

Capture source: LPOSC clock (main system clock reference), reference frequency = 198000000 Hz
Computed frequency value = 813468 Hz
Expected frequency value = 1000000 Hz

Capture source: FRO192M clock (main system clock reference), reference frequency = 198000000 Hz
Computed frequency value = 152082435 Hz
Expected frequency value = 192000000 Hz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
