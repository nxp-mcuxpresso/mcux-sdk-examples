Overview
========
The pca9420 driver example demonstrates the usage of pca9420 SDK component driver.
The example shows the usage of PF1550 API to:
1. Dumping Mode Settings;
2. Switch Mode;
3. Dump PCA9420 register content;
4. Feed watchdog;

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
The log below shows the output of the example in the terminal window, and user can interact with the application in terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-------------- PCA9420 on board PMIC driver example --------------

Please select the PMIC example you want to run:
[1]. Dumping Mode Settings
[2]. Switch Mode
[3]. Dumping Selected Register Content
[4]. Feed watch dog
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: The example enable watchdog power in mode 3, so it will timerout after 16 seconds unless "Feed watch dog" before timeout.
The output voltage of PMIC regulators can be measured:
JP14-1 : PMIC SW1
JP3-7  : PMIC SW2
JP15-3 : PMIC LDO1
JP3-5  : PMIC LDO2
