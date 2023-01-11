Overview
========
This is a small demo of the ThreadX Module. It includes a ThreadX
Module Manager thread that manages module loading, starting, stopping
and unloading.


Toolchain supported
===================
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1060 board
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
3.  Compile the demo with the configuration, "flexspi_nor_debug".
4.  Load the module image to the flash(0x60080000) of the target board.
    Please refer to readme.txt of the threadx_module project.
5.  Write the program to the flash of the target board.
6.  Press the reset button on your board to start the demo.

Running the demo
================
After preparing the demo, press the reset button on your board to start the demo.

Example output:
ThreadX Module Manager example ...
Mgr: Initializing memory from mgr_memory for modules
Mgr: Creating object pool for modules
Mgr: Loading "module" from 0x60080000
Mgr: Starting "module"
Mgr: Let modules run for 5 seconds
Start the module app(Module Test).
Start Module:Thread 0 ...
Start Module:Thread 0 sends message ==> 0
Start Module:Thread 1 ...
Start Module:Thread 1 sends message ==> 0
Start Module:Thread 1 receives message ==> 0
Start Module:Thread 0 receives message ==> 0
Start Module:Thread 1 sends message ==> 1
Start Module:Thread 0 sends message ==> 1
Start Module:Thread 0 receives message ==> 1
Start Module:Thread 1 receives message ==> 1
Start Module:Thread 0 sends message ==> 2
Start Module:Thread 1 sends message ==> 2
Start Module:Thread 1 receives message ==> 2
Start Module:Thread 0 receives message ==> 2
Start Module:Thread 1 sends message ==> 3
Start Module:Thread 0 sends message ==> 3
Start Module:Thread 0 receives message ==> 3
Start Module:Thread 1 receives message ==> 3
Start Module:Thread 0 sends message ==> 4
Start Module:Thread 1 sends message ==> 4
Start Module:Thread 1 receives message ==> 4
Start Module:Thread 0 receives message ==> 4
Mgr: Stopping "module"
Mgr: Unloading "module"
