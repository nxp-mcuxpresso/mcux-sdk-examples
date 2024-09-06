Overview
========
The ROM API project is a simple demonstration program of the SDK ROM API driver.
- The ROM API supports native support for callable crypto functions in the user application.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

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
This example will run NBOOT apis. When the example runs successfully, the following message is displayed in the terminal.
```
ROM API NBOOT Driver
INFO: Starting Example test_nboot_context_init
INFO: Finished Example test_nboot_context_init
INFO: Starting Example test_romapi_rng_generate_random
 data_buf[0] random num
 data_buf[1] random num
 data_buf[2] random num
 data_buf[3] random num
INFO: Finished Example test_romapi_rng_generate_random
INFO: Starting Example test_nboot_context_deinit
INFO: Finished Example test_nboot_context_deinit
ALL nboot examples completed successfully!
```


