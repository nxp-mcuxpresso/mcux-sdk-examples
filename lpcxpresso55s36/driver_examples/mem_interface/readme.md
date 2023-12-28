
SDK version
===========
- Version: 2.15.0

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

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

When the example runs successfully, the following message is displayed in the terminal:

```
Internal flash example start:

Calling API_Version!
IAP API version = x.x.x

Calling API_Init!
API_Init Successfully!

PFlash Information:
Program Flash Base Address: xx
Total Program Flash Size: xx
Program Flash Sector Size: xx

Erasing internal flash
Successfully erased internal flash xx -> xx

Calling MEM_Write to program internal flash!
Successfully Programmed and Verified Location xx -> xx

End of internal flash example!
```

