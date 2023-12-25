Overview
========
The ROM API project is a simple demonstration program of the SDK ROM API driver.
- The ROM API supports In-application-programming functionality by the IAP API, via the unified memory interface or the
  Sbloader API.


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
Example will print version, perform various memory interface operations (program, erase, flush).
When the example runs successfully, the following message is displayed in the terminal.

```
INFO: IAP driver version [I-v1.0.0]
INFO: Starting Example test_iap_mem_operation
INFO: Finished Example test_iap_mem_operation
INFO: Starting Example test_sb_loader_examples
INFO: Finished Example test_sb_loader_examples
ALL IAP Examples completed successfully!
```


