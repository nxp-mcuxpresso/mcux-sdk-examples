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
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S16 board
- Personal Computer

Board settings
==============
The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J1) on the board.
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
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USART Receive Double Buffer Example. The USART will echo the double buffer after each 8 bytes :
CallBack A
CallBack B
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
