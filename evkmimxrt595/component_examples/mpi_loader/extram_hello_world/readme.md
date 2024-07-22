Overview
========
The mpi_loader_extram_hello_world example demonstrates how to build an application running in on-board
PSRAM. Such application can be combined with mpi_loader_extram_loader by elftosb tool and boot by
mpi_loader_extram_loader.
The elftosb and blhost tool can be downloaded from nxp.com.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2. Example boards\evkmimxrt595\component_examples\mpi_loader\extram_loader should be built and we can then
   get extram_loader.bin.
3. Build this example, and we can get extram_hello_world.bin.
4. Copy all those binaries to elf2sb workspace "input_images" folder identified in
   boards\evkmimxrt595\component_examples\mpi_loader\extram_loader\mpi_loader_ram.json.
5. Copy mpi_loader_ram.json to elf2sb workspace "image_config" folder.
6. Execute elf2sb.exe with mpi_loader_<type>.json as parameter. E.g.,
   - elftosb.exe -V -d -f rt5xx -J workspace/image_config/mpi_loader_ram.json
7. In elf2sb workspace "output_images" folder, you can get the multicore packed image mpi_loader_extram_hello_world.bin.
8. Write flash config block to flash offset 0x400 (address 0x08000400) and write mpi_loader_extram_hello_world.bin to
   flash offset 0x1000 (address 0x08001000). E.g. with blhost tool, set board boot mode DIP to Serial ISP
   [1-ON, 2-OFF, 3-OFF], reset the board and use command window to communicate with it (need to identify the COM port).
   - blhost.exe -p COM29 -- fill-memory 0x1c000 4 0xc0403004
   - blhost.exe -p COM29 -- configure-memory 9 0x1c000
   - blhost.exe -p COM29 -t 100000 -- flash-erase-region 0x08000000 0x100000
   - blhost.exe -p COM29 -- fill-memory 0x1d000 4 0xf000000f
   - blhost.exe -p COM29 -- configure-memory 0x9 0x1d000
   - blhost.exe -p COM29 -- write-memory 0x08001000 workspace\output_images\mpi_loader_extram_hello_world.bin
9. Set board boot mode to flash boot [1-OFF, 2-OFF, 3-ON]
10. Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
11. Reset the board and you will see the result.

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display the following:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Loading finished, now enter external RAM application.
hello world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

