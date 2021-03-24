Overview
========
The FlexIO MCU LCD demo application demonstrates how to use SDK FlexIO to drive
MCU interface LCD.

The example displays one picture and an arrow at the right bottom of the screen.
When press the arrow, a new picture is shown.

The project has provide the source code of the pictures (pictures.c), you can
modify and re-generate the pictures.bin. For example, the pictures.bin could be
generated using armgcc commands:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
arm-none-eabi-gcc -c pictures.c -DBUILD_PIC_BIN
arm-none-eabi-objcopy -O binary pictures.o pictures.bin
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDMK28FA board
- Personal Computer
- TFT Proto5 panel HW REV 1.01

Board settings
==============
1. Connect LCD panel HDR1 to FRDMK28FA board J27 (pin 1 to pin 1).
2. Short jumper J-53.

Prepare the Demo
================
1.  Download the picture.bin to QSPI flash using blhost. If the picture.bin has
    been downloaded, this step could be ingored.
    - Press and hold the NMI(SW2) button, then connect a USB cable between the host PC and
      the J-24 USB port on the target board.
    - Follow the chapter "Kinetis ROM Bootloader" in reference manual to configure
      the QSPI. The configure block file is qspi_config_block.bin.
    - Download the picture.bin file to QSPI flash address 0x68000400 using blhost.
    - Disconnect the USB cable.
    Example:
    Execute the following commands:
    "blhost -u -- write-memory 0x20000000 qspi_config_block.bin"
    "blhost -u -- configure-memory 1 0x20000000"
    "blhost -u -t 1000000 -- flash-erase-region 0x68000000 0x300000"
    "blhost -u -- write-memory 0x68000400 pictures.bin"
    if the blhost does not support command "-u -- configure-memory", please update
    to the latest blhost.

3.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.

4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
    This step is optional if you don't want to see the log details.

5.  Download the program to the target board.

6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
Running the demo
================
This demo could run without debug console. The debug console only shows some
instructions and demo status.

At the beginning, the debug console shows:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXIO MCU LCD example:
Touch the arrow to show next picture...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Press the arrow to show the next picture. If touch point is out of arrow, there
is error log from the debug console:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Not touch the arrow...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
