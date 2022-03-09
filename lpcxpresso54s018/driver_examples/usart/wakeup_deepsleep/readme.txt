Overview
========
The usart_wakeup_deepsleep example shows how to use usart driver in 32kHz clocking mode
to wake up soc from deep sleep.

In this example, one usart instance is connected to PC, the board will enter into deep
sleep mode and wake up according to user input.

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
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
This demo is an external flash plain load demo, ROM will copy image in external flash to SRAMX to run:
1. Build the demo to generate a bin file.
   Note: If need to generate bin file using MCUXpresso IDE, below steps need to be followed:
         Set in example Properties->C/C++ Build->Settings->Build steps->Post-build steps->Edit
         enbable arm-none-eabi-objcopy -v -O binary "&{BuildArtifactFileName}" "&{BuildArtifactFileBaseName}.bin" 
         
         This plainload example linked the vector table to 0x00000000, but program to external flash 0x10000000.

2. Program the bin file to external on board flash via SEGGER J-FLASH Lite(V6.22 or higher):

   a. Open SEGGER J-FLASH Lite, select device LPC54S018.

   b. Click the 'Erase Chip' to erase the extrenal flash.(if can not success, press SW4 button and reset the board, and try to erase again)

   c. Select the bin data file, set the '.bin/Erase Start' address to 0x10000000, then click 'Program Device'
Note: Please use above way to program the binary file built by armgcc tool chain to external flash. 
      For IAR, KEIL, MCUXpresso IDE, you can use the IDE tool to program the external flash.  
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usart waking up soc from deep sleep example, please note USART can only work at 9600 baudrate in deep sleep mode

Press 1 to enter deep sleep
Press any other key to wake up soc
/* Type in '1' into UART terminal */
Received 1
Entering deep sleep mode, please change the baudrate setting of your local terminal to 9600
/* Type in '2' into UART terminal */
Received 2
Waking up from deep sleep, please change the baudrate setting of your local terminal back to 115200
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
