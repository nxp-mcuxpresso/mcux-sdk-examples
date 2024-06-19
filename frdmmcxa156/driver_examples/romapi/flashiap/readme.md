
SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.3
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB Type-C cable between the host PC and the LPC-Link USB port (P6) on the target board.
2.  Open a serial terminal with the following settings:
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
Flash driver API tree Demo Application.

Initializing flash driver.
Flash init successfull!.

Config flash memory access time. 

PFlash Information:
kFLASH_PropertyPflashBlockBaseAddr = x
kFLASH_PropertyPflashSectorSize = x
kFLASH_PropertyPflashTotalSize = x
kFLASH_PropertyPflashPageSize = x

Erase a sector of flash
Calling FLASH_EraseNonBlocking() API.
Calling FLASH_VerifyErase() API.
Successfully erased sector: xx -> xx

Calling FLASH_Program() API.
Calling FLASH_VerifyProgram() API..
Successfully Programmed and Verified Location: xx-> xx

End of PFlash Example!
```

