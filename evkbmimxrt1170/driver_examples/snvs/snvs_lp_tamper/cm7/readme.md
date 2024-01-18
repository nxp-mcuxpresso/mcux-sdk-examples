Overview
========
The SNVS LP TAMPER project is a simple demonstration program of the SDK SNVS LP driver. The test will set up pasive and active tamper pins and also enables and test voltage, temperature and clock tampers.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
No special settings are required.

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
The log below shows the output of snvs example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SNVS tamper demo
1 - passive tamper pin
2 - active tamper pin
3 - voltage tamper enable
4 - voltage tamper test
5 - temperature tamper enable
6 - temperature tamper test
7 - clock tamper enable
8 - clock tamper test
0 - exit


Select test and confirm by Enter...
1
Select passive tamper pin to be used (1~10) and confirm by Enter...7
 If tamper pin 7 is not low level, will trigger tamper violation
External Tampering 7 Detected
ZMK is cleared

press any key to disable all pins and return to test menu ...


SNVS tamper demo
1 - passive tamper pin
2 - active tamper pin
3 - voltage tamper enable
4 - voltage tamper test
5 - temperature tamper enable
6 - temperature tamper test
7 - clock tamper enable
8 - clock tamper test
0 - exit


Select test and confirm by Enter...
2
Select tamper active tx pin (1~5) and confirm by Enter...
2
Select tamper ecternal rx pin input(6-10) and confirm by Enter...
8
if tamper pin tx 2 and rx pin 8 don't connect together, will trigger tamper violation
External Tampering Detected!

press any key to continue ...
3
Voltage tamper enabled!

press any key to continue ...
4
No voltage tamper detected!

press any key to continue ...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
