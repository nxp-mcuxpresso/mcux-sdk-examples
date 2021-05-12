Overview
========
The freertos_pf1550 example demonstrates the usage of pf1550 service driver based on FreeRTOS and RPMSG_lite.

This example should use with Linux Kernel running on Application Processor Core which has a virtual PF1550 driver in it.
The example has 3 tasks running concurrent: the first one is user application on Cortex-M4 Core, here we use a hello world
application as a reference. The second one is a PF1550 Local Service task which will receive PF1550 control request from
other task and operate the PF1550 peripheral accordingly. The third one is the PF1550 Remote Service task, this task is
continuous waiting for PF1550 control request raised by Remote Application Processor Core and convert the remote PF1550
control request to PF1550 Local Service API calling and send response once PF1550 remote request is done or failed.

Toolchain supported
===================
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can support running with Linux BSP! ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the freertos pf1550 example in the terminal window.
Note
1. The ONKEY interrupt feature of PF1550 is not supported on I.MX7ULP-EVK board
2. In order to protect the i.MX 7ULP from over-voltage, it is recommended to setting
   regulator's output voltage with following restraint:
   Buck Switch1's range: 800mV ~ 1100mV;
   Buck Switch2's range: Fixed to 1200mV;
   Buck Switch3's range: Fixed to 1800mV;
   LDO1's range        : 3000mV to 3300mV;
   LDO2's range        : Fixed to 3300mV;
   LDO3's range        : Fixed to 1800mV.
   The Setting Regulator Output Voltage function of this example is used to demonstrate
   the usage of SDK PF1550 bare-bone driver, user need to adjust the regulator output
   according to specific board design.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-------------- PF1550 on board PMIC RTOS driver example --------------

Please select the PMIC example you want to run:
[1]. Setting Regulator Output Voltage
[2]. Dumping Regulator Output Voltage
[3]. Dumping Selected Register Content
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
