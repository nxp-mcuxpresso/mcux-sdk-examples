Overview
========
This "labelimage" example is a demonstration baremetal program which uses the SDRAM to allocate
the memory pool, and it integrates a user defined model(mobilenet_v1_0.25_160) and a user
provided image(JPG) to classify, the model is trained using tensorflow, then convert to binary
runtime model (RTM) with eIQ Portal.

Documentation
-------------
  https://www.nxp.com/design/software/development-software/eiq-ml-development-environment:EIQ

Library configuration
------------------------
 Stack memory configuration
 During the library compilation, based on the stack memory configuration,
 the EIGEN_STACK_ALLOCATION_LIMIT macro definition can be set to the maximum
 size of temporary objects that can be allocated on the stack
 (they will be dynamically allocated instead). A high number may cause stack
 overflow. A low number may decrease object allocation performance.

Release notes
-------------
The library is based on DeepView RT version 2.4.44.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
2.  Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert the Ethernet Cable into the target board's RJ45 port and connect it to a router (or other DHCP server capable device).
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs, the log and inference result would be seen on the terminal as below.

   Result: giant panda (85%) -- decode: 52 ms runtime: 672 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 443 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 443 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 442 ms
   Result: giant panda (85%) -- decode: 52 ms runtime: 442 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 443 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 442 ms
