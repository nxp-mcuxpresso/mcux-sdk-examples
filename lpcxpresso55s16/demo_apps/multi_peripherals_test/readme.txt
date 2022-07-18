Overview
========
The multi-peripherals-test demo application does the basic peripheral test on
board. This demo will test audio, usb, led, button, sdcard, accelerometer and rtc.
To test audio function, connect an audio source, for example from a mobile phone
or PC, to Line-IN connector, connect a headphone or speaker to Line-Out connector. Then
the input audio can be heared in the headphone or speaker.
To test usb, connect a USB cable between the PC and board's high speed connector,
the cursor on the PC will move in a square pattern.
To test sdcard function, insert a uSD memory card in to the connector, then
the test result will be printed, please make sure the message is not a failed
message.
Please verify the green LED in the RGB-LED flashed with a 1Hz rate. This indicates
that the RTC crystal works.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S16 board
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)

Board settings
==============
No special is needed.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect headphones to Audio HP / Line-Out connector.
2.  Connect source of sound(from PC or Smart phone) to Audio Line-In connector.
3.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Multi-peripheral Test!

AUDIO Loopback started!
Headphones will play what is input into Audio Line-In connector.
Accelerometer: Found Accelerometer!
USB device HID mouse test
Plug-in the device, which is running mouse test, into the PC.A HID-compliant mouse is enumerated in the Device Manager.
The mouse arrow is moving on PC screen in the rectangular rotation.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Press the WAKEUP button, the red LED will flash.
Press the ISP button, the blue LED will flash.
Press the USER button, the green LED will constant on.
