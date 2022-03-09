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
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (J40) on the board.
2. Open a serial terminal on PC for JLink serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below. 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS USART driver example!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
