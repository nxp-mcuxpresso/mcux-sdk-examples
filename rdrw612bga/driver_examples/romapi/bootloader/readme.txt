Overview
========
The ROM API project is a simple demonstration program of the SDK ROM API driver.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
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
Example will print version, copyright and enter into ISP mode.
When the example runs successfully, the following message is displayed in the terminal:

```
INFO: Bootloader version [K-v3.1.1]
INFO: Bootloader Copyright [Copyright 2021 NXP]
TC INFO: Starting TC test_runBootloaderEnter boot image...
Calling the runBootloader API to force into the ISP mode: 0
The runBootloader ISP interface is choosen from the following one:
kUserAppBootPeripheral_UART     0
kUserAppBootPeripheral_I2C      1
kUserAppBootPeripheral_SPI      2
kUserAppBootPeripheral_USB_HID  3
kUserAppBootPeripheral_FLEXSPI  4
kUserAppBootPeripheral_DFU      5
kUserAppBootPeripheral_SDIO     6
Call the runBootloader API based on the arg : eb100000...
```


