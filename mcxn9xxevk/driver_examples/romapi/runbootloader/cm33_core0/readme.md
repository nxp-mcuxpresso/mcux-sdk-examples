
SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N9XX-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
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
Calling the runBootloader API to force into the ISP mode: x...
The runBootloader ISP interface is choosen from the following one:
kIspPeripheral_Auto :     0
kIspPeripheral_UsbHid :   1
kIspPeripheral_Uart :     2
kIspPeripheral_SpiSlave : 3
kIspPeripheral_I2cSlave : 4
kIspPeripheral_Can :      5
Call the runBootloader API based on the arg : xxxxxxxx...
```

