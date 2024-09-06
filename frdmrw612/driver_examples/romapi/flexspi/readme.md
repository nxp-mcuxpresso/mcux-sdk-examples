Overview
========
The ROM API project is a simple demonstration program of the SDK ROM API driver.
- The ROM API supports the Serial NOR FLASH programming via the FlexSPI NOR API.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board
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
Example will print version, perform various FlexSPI flash operation (read, program, erase).
When the example runs successfully, the following message is displayed in the terminal.

```
ROM API FlexSPI Driver version [0x   10802]

FlexSpi flash Information:
FlexSpi flash size: xx KB, Hex: (xx)
FlexSpi flash sector size: xx KB, Hex: (xx)
FlexSpi flash page size: xx B, Hex: (xx)
Finished executing ROM API FlexSPI Driver examples
```


