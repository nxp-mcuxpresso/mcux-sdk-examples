
SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- Personal Computer

Board settings
==============
No special settings are required.
Note: By default, the on chip flash cannot erase. 
The Memory Block Configuration Word in MBC for on chip flash default value is 4.
It means all the flash blocks can read and execute, but cannot erase.
To run flash erase function, please configure ACL_SEC_14(the erased sector is sector 14) in IFR table to 0x0 in ISP mode:
    blhost -p com8 flash-erase-region 0x01000000 0x100
    blhost -p com8 write-memory 0x01000000 cmpa.bin
    Note: The com8 need update based on your EVK com number
It means the sector 14 of flash could read and write.

Prepare the Demo
================
1.  Connect a Type-C USB cable between the host PC and the MCU-Link port(J15) on the target board.
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

Erase a page of flash
Calling FLASH_EraseNonBlocking() API.
Calling FLASH_VerifyErase() API.
Successfully erased page: xx -> xx

Calling FLASH_Program() API.
Calling FLASH_VerifyProgram() API..
Successfully Programmed and Verified Location: xx-> xx

End of PFlash Example!
```

