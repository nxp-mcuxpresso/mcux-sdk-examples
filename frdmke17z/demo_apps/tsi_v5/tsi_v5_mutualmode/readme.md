Overview
========
The tsi_v5_mutualmode demo shows how to use TSI_V5 driver in mutual-cap mode:

In this example , available electrodes on FRDM-TOUCH board are used to show how to realize touch key.
1. Firstly, get the non-touch calibration results as baseline electrode counter;
2. Then, start the periodical Software-Trigger scan using polling method to detect finger touch;
3. Wait for the electrodes touched and deal with the event.


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
- FRDM-KE17Z board, FRDM-TOUCH board
- Personal Computer

Board settings
==============
Keep the default configuration for the FRDM board and connect the FRDM-TOUCH board with the FRDM board.

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
When running successfully, LED will be turned on when the electrode on FRDM-TOUCH board is touched
and the LED will be turned off after the electrode is released. 
The log output in terminal shall be similar as below:
~~~~~~~~~~~~~~~~~~~~~
TSI mutual mode demo.
TSI key touched.
TSI key released.
TSI key touched.
TSI key released.
~~~~~~~~~~~~~~~~~~~~~
