Overview
========
The DMA channel chain transfer example is a simple demonstration program about how to use channel chain feaute with the SDK software.
In this example three channel is used, 
          
          ------->channel1------->channel0
          |
channel0-->
          |
          ------->channel2

Channel0 is configured with two descriptor, first descriptor is linked to the second, then trigger the channel0 by software, after channel0 first descriptor finish, it will trigger channel1 and channel2 start transfer, after channel1 descriptor exhaust it will trigger channel0, then channel0 second descritpor will be carried out, after second descriptor exhaust, example finish.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S28 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~
DMA channel chain example begin.
Destination Buffer:
0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0
DMA channel chain example finish.
Destination Buffer:
1	2	3	4	11	22	33	44 111 222 333 444	1111 2222 3333 4444
~~~~~~~~~~~~~~~~~~~~~
