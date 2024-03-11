Overview
========
The flexspi_nor_dma_transfer example shows how to use flexspi driver with dma:

In this example, flexspi will send data and operate the external nor flash connected with FLEXSPI. Some simple flash command will
be executed, such as Write Enable, Erase sector, Program page.
Example will first erase the sector and program a page into the flash, at last check if the data in flash is correct.

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXSPI dma example started!
Vendor ID: 0xc2
Erasing Serial NOR over FlexSPI...
Erase data - successfully. 
Program data - successfully. 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

NOTE:
In this project, after starting DMA, the project (function flexspi_nor_flash_program in flexspi_nor_flash_ops.c)
uses a while loop to check a variable (g_completionFlag) to get the DMA transfer done status.
With RW610-RW612, each SRAM region access priority is fixed as:
 - CM33 C-AHB is the highest priority
 - CM33 S-AHB is the next priority
 - The AHB matrix is the lowest priority
When the function flexspi_nor_flash_program code, variable g_completionFlag, and the DMA
accesssed buffer s_nor_program_buffer are in the same SRAM region, CM33 core while loop accesses
the SRAM region frequently, so DMA can't get data in time, and results to FlexSPI timeout.
The solution is place the CM33 accessed function/variable and DMA accessed variable to different SRAM region.
