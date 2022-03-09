Overview
========
The USART example for FreeRTOS demonstrates the possibility to use the USART driver in the RTOS.
The example uses single instance of USART IP and writes string into, then reads back chars.
After every 4B received, these are sent back on USART.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S16 board
- Personal Computer

Board settings
==============
Connect a USB2COM between the PC host and the board UART pins
boards           -               USB2COM
J14-Pin26                        Tx
J14-Pin28                        Rx
J14-Pin1                         GND

The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)
    
Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

