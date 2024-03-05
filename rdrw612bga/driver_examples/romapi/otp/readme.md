Overview
========
The ROM API project is a simple demonstration program of the SDK ROM API driver.
- The ROM API supports OTP-eFuse read and programming and CRC verify operations.


SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2

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
This example demonstrate various OTP operations. When the example runs successfully, the following message is displayed in the terminal.

```
ROM API OTP Driver Version 0x8000000
INFO: Starting Example test_otp_init
INFO: Finished Example test_otp_init
INFO: Starting Example test_otp_fuse_read
Read fuseword[45] = [0x0]
INFO: Finished Example test_otp_fuse_read
INFO: Starting Example test_otp_deinit
INFO: Finished Example test_otp_deinit
ALL OTP Examples completed successfully!
```


