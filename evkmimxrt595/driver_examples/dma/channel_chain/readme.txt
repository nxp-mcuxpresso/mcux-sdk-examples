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
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

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

