Overview
========

The enet_rxtx_rxinterrupt example shows the simplest way to use ENET transactional tx/rx API for simple frame receive and transmit.

1. This example shows how to initialize the ENET.
2. How to use ENET to receive frame in interrupt irq handler and to transmit frame.

The example transmits 20 number broadcast frame, print the number of recieved frames. To avoid
the receive number overflow, the transmit/receive loop with automatically break when 20 number
are received.



SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso54S018 board
- Personal Computer
- Loopback network cable RJ45 standard (optional)

Board settings
==============
For LPCXpresso54S018 V2.0:JP11 and JP12 1-2 on.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

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
The log below shows example output of the example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 ENET example start.


Transmission start now!
The 1 frame transmitted success!
The 2 frame transmitted success!
The 3 frame transmitted success!
The 4 frame transmitted success!
The 5 frame transmitted success!
The 6 frame transmitted success!
The 7 frame transmitted success!
The 8 frame transmitted success!
The 9 frame transmitted success!
The 10 frame transmitted success!
The 11 frame transmitted success!
The 12 frame transmitted success!
The 13 frame transmitted success!
The 14 frame transmitted success!
The 15 frame transmitted success!
The 16 frame transmitted success!
The 17 frame transmitted success!
The 18 frame transmitted success!
The 19 frame transmitted success!
The 20 frame transmitted success!
1 frame has been successfuly received
2 frame has been successfuly received
3 frame has been successfuly received
4 frame has been successfuly received
5 frame has been successfuly received
6 frame has been successfuly received
8 frame has been successfuly received
9 frame has been successfuly received
10 frame has been successfuly received
12 frame has been successfuly received
13 frame has been successfuly received
14 frame has been successfuly received
15 frame has been successfuly received
17 frame has been successfuly received
18 frame has been successfuly received
20 frame has been successfuly received


...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Optionally, you may use a loop back cable
Make a loopback network cable:
      568B standard 	 Unknowed standard
J1    orange+white       green+white
J2    orange             green
J3    green+white        orange+white
J4    blue               brown+white
J5    blue+white         brown
J6    green              orange
J7    brown+white        blue
J8    brown              blue+white

Connect J1 => J3, J2 => J6, J4 => J7, J5 => J8. 10/100M transfer only requires J1, J2, J3, J6, and 1G transfer requires all 8 pins.
Check your net cable color order and refer to the 568B standard or the other standard. If your cable's color order is not showed in the list,
please connect J1~J8 based on your situation.

1.  Add #define EXAMPLE_USES_LOOPBACK_CABLE 1  into app.h and rebuild the example.
2.  Insert loopback network cable to Ethernet RJ45 port.
3.  Run the demo in the same way as described earlier.
