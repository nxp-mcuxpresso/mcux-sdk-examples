Overview
========
The IAP Flash project is a simple demonstration program of the SDK IAP driver. It erases and programs 
a portion of on-chip flash memory. A message a printed on the UART terminal as various operations on 
flash memory are performed.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S16 board
- Personal Computer

Board settings
==============
The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)

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
Flash driver API tree Demo Application...
Initializing flash driver...
Flash init successfull!!. Halting...

 PFlash Example Start 
	kFLASH_PropertyPflashBlockBaseAddr = 0x0
	kFLASH_PropertyPflashSectorSize = 32768
	kFLASH_PropertyPflashTotalSize = 655360
	kFLASH_PropertyPflashPageSize = 0x200

Calling FLASH_Erase() API...
Done.

Done!
Calling FLASH_VerifyErase() API...
Done.

FLASH Verify Erase successful!
Calling FLASH_Program() API...
Done.

Calling FLASH_VerifyProgram() API...
Done.

FLASH Verify Program successful!

 Successfully Programmed and Verified Location 0x80000 -> 0x80200 
Done!
```

