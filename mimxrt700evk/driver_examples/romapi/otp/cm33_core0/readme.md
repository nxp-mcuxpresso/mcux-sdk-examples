Overview
========
The ROM API project is a simple demonstration program of the SDK ROM API driver.
- The ROM API supports OTP-eFuse read and programming operations.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Target board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port on the board
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
ROM API OTP Driver Version 0x45010100
INFO: Starting Example test_otp_init 
INFO: Finished Example test_otp_init
INFO: Starting Example test_otp_fuse_read 
Read fuseword[52] = [0xxx]
INFO: Finished Example test_otp_fuse_read 
INFO: Starting Example test_otp_deinit 
INFO: Finished Example test_otp_deinit 
ALL OTP Examples completed successfully!
```


