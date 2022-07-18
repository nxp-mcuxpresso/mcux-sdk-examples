Overview
========
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_ecspi_int_loopback_transfer example shows how to use CMSIS ECSPI driver in interrupt way:
 
In this example , ECSPI will do a loopback transfer in interrupt way, so, there is no need to set up any pins.
And we should set the ECSPIx->TESTREG[LBC] bit, this bit is used in Master mode only. When this bit is set, 
the ECSPI connects the transmitter and receiver sections internally, and the data shifted out from the 
most-significant bit of the shift register is looped back into the least-significant bit of the Shift register.
In this way, a self-test of the complete transmit/receive path can be made. The output pins are not affected, 
and the input pins are ignored.




Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8MM6-EVK  board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
No special settings are required.



Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================
When the demo runs successfully, the log would be seen on the debug terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is ECSPI CMSIS interrupt loopback transfer example.
The ECSPI will connect the transmitter and receiver sections internally.
Start transfer...

  This is ECSPI_MasterSignalEvent_t.

Transfer completed!
ECSPI transfer all data matched! 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
