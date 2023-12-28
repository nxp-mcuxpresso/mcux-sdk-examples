Overview
========
The sema42 uboot example shows how to use SEMA42 driver to lock and unlock a sema gate.
This example should work together with uboot. This example runs on Cortex-M core,
the uboot runs on the Cortex-A core.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open two serial terminals (one for Cortex-A35 core, the other for Cortex-M33 core) with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example
6.  Make sure board boot in single boot mode, and A35 run into U-Boot console

Running the demo
================
The log below in the Cortex-M terminal window shows the commands to use in uboot:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**************************************************************
* Please make sure the uboot is started now.                 *
* Use the following commands in U-Boot(A35) console for SEMA42 gate access *
* - md.b 0x28037003 1 : Get SEMA42 gate status.              *
*   - 0 - Unlocked;                                          *
*   - 8 - Locked by Cortex-A35;                                *
*   - 7 - Locked by Cortex-M33;                                *
* - mw.b 0x28037003 8 1 : Lock the SEMA42 gate.              *
* - mw.b 0x28037003 0 1 : Unlock the SEMA42 gate.            *
**************************************************************

Press anykey to start the example...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Follow the example output instructions:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Now the SEMA42 gate is unlocked, checking status in uboot returns 0.
Press anykey to lock the SEMA42 gate...

Now the SEMA42 gate is locked, checking status in uboot returns 3.
Lock or unlock the SEMA42 gate in uboot, the status does not change.

Press anykey to unlock the SEMA42 gate...

Now the SEMA42 gate is unlocked, checking status in uboot returns 0.

Lock the SEMA42 gate in uboot, after locked, then press anykey...

Cortex-A has locked the SEMA42 gate in uboot, Cortex-M could not lock.

Press anykey to reset the SEMA42 gate...

Now the SEMA42 gate is unlocked, checking status in uboot returns 0.

Press anykey to finish the example...

SEMA42 uboot example successed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
