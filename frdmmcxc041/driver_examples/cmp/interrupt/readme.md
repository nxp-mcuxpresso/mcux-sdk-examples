Overview
========

The CMP interrupt Example shows how to use interrupt with CMP driver.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
CMP's positive channel input. On the negative side, the internal 6-bit DAC is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user-defined channel's voltage crosses the internal DAC's value. The change of
comparator's output would generate the falling and rising edge events with their interrupts enabled. When any CMP 
interrupt happens, the CMP's ISR would turn on the LED light if detecting the output's rising edge, or turn off it when
detecting the output's falling edge.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXC041 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect a Type-C USB cable between the PC host and the OpenSDA USB port on the board.
2.  Connect the analog signal source's output to the input of used-defined comparator's channel (defined as 
"DEMO_CMP_USER_CHANNEL" in source code.)
3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~
CMP interrupt Example
~~~~~~~~~~~~~~~~~~~~~

Then change CMP analog input, and watch the change of LED.
    - CMP0_IN0 (Jump J4-6) connected to VCC=3.3V(Jump J3-4): LED GREEN on
    - CMP0_IN0 (Jump J4-6) connected to GND(Jump J3-12): LED GREEN off
