Overview
========
The MCG_Lite example shows how to use MCG_Lite driver:

 1. How to use the mode functions for MCG_Lite mode switch.
 2. How to use the frequency functions to get current MCG_Lite frequency.
 3. Work flow
  Reset mode --> LIRC8M
    LIRC8M --> HIRC
    HIRC   --> LIRC2M
    LIRC2M --> EXT
    EXT    --> HIRC
    HIRC   --> LIRC8M
    LIRC8M --> EXT
    EXT    --> LIRC2M
    LIRC2M --> HIRC
    HIRC   --> EXT
    EXT    --> LIRC8M
    LIRC8M --> LIRC2M
    LIRC2M --> LIRC8M
In this example, because the debug console's clock frequency may change,
so the example running information is not output from debug console. Here the
LED blinks to show that the example finished successfully.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-MCXC242 board
- Personal Computer

Board settings
==============
No special board settings.

Prepare the Demo
================
1. Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on FRDM-MCXC242 board.
2. Download the program to the target board.
3. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo. For detailed instructions, see Getting Started with Software Development Kit for
   MCXC242.

Running the demo
================
When the demo runs successfully, the board's LED will blink.
