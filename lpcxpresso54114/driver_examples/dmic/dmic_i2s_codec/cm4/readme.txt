Overview
========
Demonstrates the DMIC working with I2S. Audio is converted to samples in the DMIC module.
Then, the data is placed into the I2S buffer directly without DMA or CPU intervention. Last, it is read from the I2S buffer and sent
to the CODEC, then the audio data will be output to Lineout of CODEC.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCLPCXpresso54114 board
- MAO board
- Personal Computer
- Earphone/headset
- Jumper wire
- Use an iphone headset(CTIA standard)

Board settings
==============
On MAO board
a) connect J2-7 to J1-20. Jumper wire is needed to connect both conector pins
b) JP-3 must be set on 1-2

Prepare the Demo
================
This demo requires the MAO board. It contains the WM8904 CODEC that is used for this demo.
This demo was written on a LPCXpresso 5411x board, PCBrev A and MAO REV A board.

1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J7) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

Configure WM8904 codec

Configure I2S

2. This example transfers data from DMIC to Codec. Connect headphone/earphone on audio out of the board.
   Speak on DMIC and you can listen voice on audio out.
