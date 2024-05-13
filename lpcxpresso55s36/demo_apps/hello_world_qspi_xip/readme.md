Overview
========
The Hello World demo application provides a sanity check for the new SDK build environments and board bring up. The Hello
World demo prints the "Hello World" string to the terminal using the SDK UART drivers. The purpose of this demo is to
show how to use the UART, and to provide a simple project for debugging and further development.
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.

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
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============
Connect pin 1-2 of JP41,JP42,JP43,JP44,JP45,JP62,JP64 and JP65.
Close JP63,JP66,JP67,JP68 and JP69.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Set boot mode to ISP boot with J43 to set ISP0 high, ISP1 low
      unshort J43 1-2 to make ISP0 high, short J43 3-4 to make ISP1 low
4.  Write flash config block to flash offset 0x400(0x08000400)
      blhost -p com26 fill-memory 0x2000f000 4 0xC0000001
      blhost -p com26 configure-memory 0x9 0x2000f000
      blhost -p com26 flash-erase-region 0x08000000 0x1000
      blhost -p com26 fill-memory 0x2000f004 4 0xf000000f
      blhost -p com26 configure-memory 0x9 0x2000f004
      Note: The com26 need update based on your EVK com number
5.  Configure PFR FLEXSPI_BOOT_CFG[0] @0x3E280 to reset flexspi flash use ROM with GPIO P1_14
      blhost -p com26 write-memory 0x3E200 rom_reset_flexspi.bin
      Note: For customer board, the PFR page @0x3E200 should read back first, then update @3E280 based on customer board schematic. 
            Command "blhost -p com26 read-memory 0x3E200 512 rom_reset_flexspi.bin" could be used to read PFR page @0x3E200.
6.  Set boot mode to FlexSPI boot with J43 to set ISP0 low, ISP1 high
      Short J43 1-2 to make ISP0 low, unshort J43 3-4 to make ISP1 high
7.  Download the program to the target board.
8.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Please note, when using FlexSPI boot, the PLL0 is used by FlexSPI and shall not change the Pll0.
Suggest to use FRO or PLL1 for example clock instead.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
hello world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
