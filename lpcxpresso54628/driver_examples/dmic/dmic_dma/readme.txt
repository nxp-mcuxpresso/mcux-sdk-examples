Overview
========
This example shows how to use DMA to transfer data from DMIC to memory.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
Whole buffer before and after data transfer will be displayed. All data before transfer is 0 and data in the 
buffer after transfer is dmic data which is now different. 
The following lines are printed to the serial terminal when the demo program is executed. 

Configure DMA
Buffer Data before transfer
 <data>
Transfer completed
Buffer Data after transfer
 <data>


2. This example shows how DMA can be used with DMIC to transfer data to memory.DMIC audio data can also be seen in g_rxBuffer.
Intially this buffer is initialized to zero.
