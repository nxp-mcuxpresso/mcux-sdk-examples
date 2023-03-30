Overview
========
The usart_wakeup_deepsleep example shows how to use usart driver in 32kHz clocking mode
to wake up soc from deep sleep.

In this example, one usart instance is connected to PC, the board will enter into deep
sleep mode and wake up according to user input.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (P6) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.
Running the demo
================
When the demo runs successfully, the log would be seen on the CMSIS DAP terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usart waking up soc from deep sleep example, please note USART can only work at 9600 baudrate in deep sleep mode

Press 1 to enter deep sleep
Press any other key to wake up soc
/* Type in '1' into UART terminal */
Received 1
Entering deep sleep mode, please change the baudrate setting of your local terminal to 9600
/* Type in '2' into UART terminal */
Received 2
Waking up from deep sleep, please change the baudrate setting of your local terminal back to 115200
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
