Overview
========

The FTM / PDB demo application demonstrates how to use the FTM external trigger to start the ADC conversion using the
PDB. FTM0 is configured as a complementary combined mode, and each channel output frequency is 16 KHz. The complementary
channel dead time is 1 µs. The PDB pre-trigger works in back-to-back mode. The ADC0 and ADC1 work in single-end mode.
The ADC0 uses channel 1 and channel 5, and ADC1 uses channel 1 and channel 7.

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
- FRDM-K22F
- Personal Computer

Board settings
==============
No special settings are required.

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
The ADC PDB demo application will print the following message to the terminal:

~~~~~~~~~~~~~~~~~
Run pdb trig adc with flextimer demo.
ADC Full Range: XXXX
Input any character to start demo.
~~~~~~~~~~~~~~~~~~
Input a character to the serial console to start the ADC PDB demo.
The demo will then display 256 lines of information for the ADC conversion result.
