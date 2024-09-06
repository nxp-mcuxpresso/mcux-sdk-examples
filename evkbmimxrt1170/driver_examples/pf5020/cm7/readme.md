Overview
========
The pf5020 driver example demonstrates the usage of pf5020 SDK component driver.
The example shows the usage of pf5020 API to:
1. Set regulator output voltages;
2. Set Internal high speed clock;
3. Set PF5020 to standby mode;
4. Dump PF5020 register content.

To use this example, user need to pay attention the output voltage when update the regulator
output and make sure the output voltage to set will not cause hardware damage.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
To use EXT PMIC, Please do following Config Changes:
1. Change J41/J53/J67/J68/J69 Jumper Setting from 1-2 to 2-3
2. Change J71/J73/J19 Jumper Setting from 1-2 to OPEN
3. Change J72/J74/J77 Jumper Setting from OPEN to 1-2
4. DNP R1851,R1853 Populate R1852,R1854

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
The log below shows the output of the pf5020 driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------ PF5020 Functional Block Menu ---------------------------------

Please select the function block you want to run:
[1]. Regulators Setting.
[2]. Clock Setting.
[3]. Enter Standby State.
[4]. Dump Register Value.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

