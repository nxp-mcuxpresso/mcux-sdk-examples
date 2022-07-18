Overview
========
The example creates a simple washing machine demonstration. It shows
a UI on a LCD panel with touch panel. It also can interact with user.

Define GUIX_PXP_ENABLE to enable Pixel Pipeline engine (PXP).
It can accelerate the image process.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- A Micro USB cable
- MIMXRT1170-EVK board
- Personal Computer
- RK055MHD091 panel or RK055AHD091 panel or RK055IQH091 panel

Board settings
==============
Connect the panel to J48
Connect 5V power to J43, set J38 to 1-2

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build the project, the project uses RK055MHD091 by default, to use other panels,
    change
    #define DEMO_PANEL DEMO_PANEL_RK055MHD091
    to
    #define DEMO_PANEL DEMO_PANEL_RK055IQH091
    or
    #define DEMO_PANEL DEMO_PANEL_RK055AHD091
    in display_support.h.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo is running, a washing machine UI will display on the LCD panel. User can
touch the panel to change the temperature, water level, ...

The serial port will output:
Start the GUIX washing machine example...

