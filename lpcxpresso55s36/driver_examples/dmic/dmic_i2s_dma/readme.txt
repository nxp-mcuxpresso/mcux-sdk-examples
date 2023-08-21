Overview
========
Demonstrates the DMIC working with I2S. One or Stereo channel Audio data is converted to samples in the DMIC module.
Then, the data is placed into the memory buffer. Last, it is send to the I2S buffer and sent
to the CODEC, then the audio data will be output to Lineout of CODEC.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S36 boards
- Personal Computer
- headphones with 3.5 mm stereo jack

Board settings
==============
Short JP50 1-2,JP53 1-2
Remove resistor between BH20 and BH21, Short BH19 and BH20. 
Remove resistor between BH23 and BH24, Short BH22 and BH23. 

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J6).
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  Launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

Configure codec

Configure I2S

2. This example transfers data from DMIC to Codec. Connect headphone/earphone on audio out of the board.
Speak on DMIC or play song nearby the dmic (U30),  you can hear sound on the left channel of headphone/earphone.
