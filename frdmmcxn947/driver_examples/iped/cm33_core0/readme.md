Overview
========
The IPED Example project is a demonstration program that uses the MCUX SDK software to set up Inline Prince Encryption Decryption.
Then tests the expected behaviour.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
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
When the demo runs successfully, the terminal displays similar information like the following:
~~~~~~~~~~~~~~~~~~
IPED Peripheral Driver Example

Calling API_Init
API_Init Successfully
Configure IPED region 0 enc/dec: start 0x8002000 end 0x8003000
Configure IPED  Successfully
External Flash memory configured successfully
*Success* read plain data from 0x8001000 to 0x8001FFF
*Success* read programmed&encrypted data from 0x08002000 to 0x8002FFF
*Success* read plain data from 0x8003000 to 0x8003FFF
Disabling IPED
*Success* read plain data from 0x8001000 to 0x8001FFF
*Success* read encrypted data from 0x8002000 to 0x8002FFF
*Success* read plain data from 0x8003000 to 0x8003FFF
Enabling IPED
*Success* read plain data from 0x8001000 to 0x8001FFF
*Success* read decrypted data from 0x8002000 to 0x8002FFF
*Success* read plain data from 0x8003000 to 0x8003FFF
Reconfiguring IPED
Reconfigure IPED  Successfully
*Success* read plain data from 0x8001000 to 0x8001FFF
*Success* read decrypted data from 0x8002000 to 0x8002FFF
*Success* read plain data from 0x8003000 to 0x8003FFF
Disabling IPED
End of example

~~~~~~~~~~~~~~~~~~

Note: Please make sure that flash is cleared and a power cycle has been performed before executing.
