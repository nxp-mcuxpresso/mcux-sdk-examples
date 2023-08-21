Overview
========
This example works as a USB audio device. It will appear as USB speaker and USB microphone device on PC.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Two Micro USB cables
- Target Board
- Personal Computer(PC)

Board settings
==============
Set USB port J3 to device mode by opening jumper JP2.

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (J1) on the target board.
2.  Connect a USB Micro cable between the host PC and the on-board USB full speed port (J3).
3.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
4.  Download the program to the target board.
5.  Press the on-board RESET button to start the demo.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX device audio loopback example...

Then, connect a USB cable between PC and the USB device port
of the board. A USB speaker and microphone will appear in the
Device Manager of Windows. The audio will output from the speaker
and loop back to the microphone.

The serial port will output:

Audio device activate
