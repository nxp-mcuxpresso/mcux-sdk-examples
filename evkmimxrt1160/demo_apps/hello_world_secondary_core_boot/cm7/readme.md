Overview
========
The hello_world_secondary_core_boot demo use cm7 core to kick off cm4 core, and cm4 core will prints the "Hello World" string 
to the terminal using the SDK UART drivers and repeat what user input. This demo demonstrates the dual XIP architecture boot 
CM7 and CM4 core, XIP configuration will be included into CM7 image, so that the image of CM4 can also be booted from FLASH, 
which helps developers to have more choices on secondary core target memory.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
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
3.  Download CM4 image firstly, then download CM7 image.(Important Step)
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
After CM4 and CM7 image downloaded, the log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Prepare to wake up core1 booted from flash, console will be transferred to it.
hello world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
