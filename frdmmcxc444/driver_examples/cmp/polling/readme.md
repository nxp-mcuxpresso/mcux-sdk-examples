Overview
========

The CMP polling Example shows the simplest way to use CMP driver and help user with a quick start.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
CMP's positive channel input. On the negative side, the internal 6-bit DAC is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user's voltage crosses the internal DAC's value. The endless loop in main() function
would detect the logic value of comparator's output, and change the LED. The LED would be turned on when the compare
output is logic one, or turned off when zero.

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
- FRDM-MCXC444 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J13) on the target board.
2.  Connect the analog signal source's output to the input of used-defined comparator's channel (defined as 
"DEMO_CMP_USER_CHANNEL" in source code.)
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~
CMP polling Example
~~~~~~~~~~~~~~~~~~~~~

Then change CMP analog input, and watch the change of LED.
    - CMP0_IN0 (Jump J1-9) connected to VCC=3.3V (Jump J3-8): LED RED on
    - CMP0_IN0 (Jump J1-9) connected to GND (Jump J2-14): LED RED off
