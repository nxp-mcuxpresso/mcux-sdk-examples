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
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N5XX-EVK board
- Personal Computer

Software requirements
- BLHost 2.6.7 or later

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Determine the COM port number (See Appendix A in Getting started guide for description how to determine serial port number).
3.  Open a command prompt and navigate to the BLHost executables folder (<BLHost root>\bin\<OS environment>).
4.  Configure the BOOT_SRC in CMPA @0x01004000 using blhost.
    4.1  Enter ISP mode so that the blhost could program the CMPA.
              Press and hold SW3(ISP key) => Press and release SW1 (N9 RST key) => Release SW3
         Another way to enter ISP mode.
              nxpdebugmbox.exe -i pyocd ispmode -m 0
    4.2  Type the following command into the command prompt
	blhost -p COMxx write-memory 0x01004000 <SDK_root>\boards\<EVK name>\demo_apps\hello_world_qspi_xip\cm33_core0\bootfromflexspi.bin
	Note: "xx" in the command above should be replaced with the COM port number of your EVK.
5.  After successfully running the script above, press and release the reset button (SW1 - N9 RST key). 
6.  Open a serial terminal with the following settings :
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
7.  Download the program to the target board.
8.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
9.  The file bootfromflash.bin is used to reset the boot source to internal flash, follow step 4.1,4.2 to program the bootfromflash.bin to 0x01004000

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
hello world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
