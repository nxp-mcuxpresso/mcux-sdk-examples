Overview
========
The pca9420 driver example demonstrates the usage of pca9420 SDK component driver.
The example shows the usage of PF1550 API to:
1. Dumping Mode Settings;
2. Switch Mode;
3. Dump PCA9420 register content;
4. Feed watchdog;

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
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
The log below shows the output of the pca9420 example in the terminal window, and user can interact with the application in terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-------------- PCA9420 on board PMIC driver example --------------

Please select the PMIC example you want to run:
[1]. Dumping Mode Settings
[2]. Switch Mode
[3]. Dumping Selected Register Content
[4]. Feed watch dog
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: Example enable watchdog power in mode 3, so it will timerout after 16 seconds unless keep call Feed watch dog example. 
