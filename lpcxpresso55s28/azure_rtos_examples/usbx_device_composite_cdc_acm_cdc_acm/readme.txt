Overview
========
This example works as two USB CDC ACM devices. It will appear as two USB serial devices on PC.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

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
6.  Press the on-board RESET button to start the demo. A new USB serial device will appear on PC.
7.  Open two serial terminals with the following settings for two new USB serial devices.
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
Running the demo
================
When the demo is running, the serial port of the Debug Link will output:

Start USBX device composite example...

Then, connect a USB cable between PC and the USB device port of the board.
Two USB serial devices will appear in the Device Manager of Windows.
And the serial port will output:

CDC ACM1 device activate
CDC ACM2 device activate

ACM1 device serial terminal, press any key, and it will display a string, for example:

fabcdef
gabcdef
3abcdef

ACM2 device serial terminal, press any key, and it will display a string, for example:

f123456
g123456
3123456
