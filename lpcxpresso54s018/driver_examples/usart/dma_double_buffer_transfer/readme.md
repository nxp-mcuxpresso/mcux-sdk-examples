Overview
========
This example shows how to use the DMA driver to implement a double buffer receive scheme from the USART

The example shows the double buffer constructed using two descriptors (g_pingpong_desc). These descriptors are cycled from one to the other.

Things to note :

- The descriptors of the ping pong transfer need to be aligned to size 16
- The inital transfer will perform the same job as first descriptor of ping pong, so the first linkeage is to go to g_pingpong_desc[1]
- g_pingpong_desc[1] then chains the g_pingpong_desc[0] as the next descriptor
- The properties are set up such that g_pingpong_desc[0] (and the initial configuration uses INTA to signal back to the callback)
- g_pingpong_desc[1] uses INTB to signal to the callback
- The scheduled callback uses this information to know which data was last written

A note on Performance :

The intent of this example is to illustrate how a double-buffer scheme can be implemented using the dma. The performance of this example will 
be limited to how quickly the echo printer can read-out the data from the ping pong buffer and display it. This means that the example will 
work well if characters are entered at a rate where the DMA callback to echo the string can keep up with the input stream. Connecting the USART
RX to a continuous fast speed will cause the DMA to fall behind.
 

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

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

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USART Receive Double Buffer Example. The USART will echo the double buffer after each 8 bytes :
CallBack A
CallBack B
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
