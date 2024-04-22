Overview
========
The multicore trigger demo is enabled only for primary core on multicore soc, to boot other subordinative core.
Normally it needs specific linker file to prevent memory conflict with subordinative core image.

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
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

Note:
    1. This demo targets to run with CM7 image, it plays role as a trigger to boot CM7 image after POR.
       User need to flash this demo image and CM7 together into flexspi nor flash.
    2. When run below CM33+CM7 image combination, it requires to set CM33_SET_TRDC to 1U in this demo,
       also requires to set CM33_SET_TRDC to 1U in CM7 demo if it exists.
       - CM33 flexspi + CM7 flexspi, flexspi means flexspi_nor_debug/release, flexspi_nor_sdram_debug/release, flexspi_nor_hyperram_debug/release
       - CM33 SDRAM + CM7 SDRAM, SDRAM means sdram_txt_debug/release.
       - CM33 HYPERRAM + CM7 HYPERRAM, HYPERRAM means hyperram_txt_debug/release.
    3. This demo use dedicated linker file to avoid conflicting with CM7 demo linkage.
       It targets to work with those CM7 standalone demos(with same/similar linkage as hello_world_cm7 demo) in this SDK out of box.
    4. If the CM7 demo is located in RAM(debug/release, sdram_debug/release, hyperram_debug/release), it is a must to SET the CM7 TCM ECC fuse.

Running the demo
================
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Multicore trigger demo!
Core0 get core1 image info sucessfully!
  Core1 image reside addr = 0xxxxxxxxx
  Core1 image dest addr   = 0xxxxxxxxx
  Core1 image size        = 0xxxxxxxxx
  Core1 image boot addr   = 0xxxxxxxxx
Core0 is starting core1...

Core1 demo output log...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
