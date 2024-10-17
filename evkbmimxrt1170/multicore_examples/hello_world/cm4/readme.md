Overview
========
The Multicore Hello World demo application demonstrates how to set up projects for individual
cores on a multicore system. In this demo, the primary core prints the "Hello World from the Primary Core!"
string to the terminal and then releases the secondary core from the reset. The secondary
core toggles an on-board LED indicating that the secondary core is running.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

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
The log below shows the output of the hello world multicore demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello World from the Primary Core!

Copy Secondary core image to address: 0x20200000, size: 3280
Starting Secondary core.
The secondary core application has been started.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy Secondary core image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.

Note:
To download and debug IAR EW project using J-Link (replacing the default CMSIS-DAP debug probe), following steps are needed:
1. Remove J6 and J7 jumpers.
2. Attach the J-Link probe (J-Link Plus / J-Trace) to the board using the J1 connector.
3. Set "J-Link / J-Trace" in CM7 project options -> Debugger -> Setup panel (replacing CMSIS-DAP option).
4. Unselect the "Use macro file(s)" in CM7 project options -> Debugger -> Setup panel.
5. Enable "Use command line options" in CM7 project options -> Debugger -> Extra Options panel 
   (--jlink_script_file=$PROJ_DIR$/../evkbmimxrt1170_connect_cm4_cm7side.jlinkscript command line option is applied).
5. Click on "Download and Debug" button. During the loading process you can be asked by J-Link sw
   to select the proper device name (MIMXRT1176XXXA_M7 is unknown). Click O.K. and choose the MIMXRT1176xxxxA device.
6. It is not possible to attach to the CM4 core when using the J-Link. Also, the multicore debugging does not work with that probe.

Note:
To download and debug Keil MDK project using J-Link (replacing the default CMSIS-DAP debug probe), following steps are needed:
1. Remove J6 and J7 jumpers.
2. Attach the J-Link probe (J-Link Plus / J-Trace) to the board using the J1 connector.
3. Set "J-LINK / J-TRACE Cortex" in CM7 project options -> Debug panel (replacing CMSIS-DAP Debugger option).
4. After the CM7 application build click on Download/F8 button (menu Flash -> Download).
5. Power off and power on the board.
6. Multicore example starts running, one can start debugging the CM7 side by clicking on Start/Stop Debug Session (Ctrl + F5). 
7. It is not possible to attach to the CM4 core when using the J-Link.
