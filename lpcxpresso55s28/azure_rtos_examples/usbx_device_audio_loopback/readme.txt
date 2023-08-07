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
This example can work with the USB high speed port (P9) or the USB full speed port (P10).

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (P6) on the target board.
2.  High speed: Connect a USB Micro cable between the host PC and the on-board USB high speed port (P9).
    Full speed: Connect a USB Micro cable between the host PC and the on-board USB full speed port (P10).
3.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
4.  Compile the demo:
    - High speed: Set USB_DEVICE_CONFIG_LPCIP3511FS to 0 and USB_DEVICE_CONFIG_LPCIP3511HS to 1 in usb_device_config.h
    - Full speed: Set USB_DEVICE_CONFIG_LPCIP3511FS to 1 and USB_DEVICE_CONFIG_LPCIP3511HS to 0 in usb_device_config.h
5.  Download the program to the target board.
6.  Press the on-board RESET button to start the demo. A new USB Speaker and USB Microphone device will appear on PC.
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
