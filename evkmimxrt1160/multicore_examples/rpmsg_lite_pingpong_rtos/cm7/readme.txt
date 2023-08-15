Overview
========
The Multicore RPMsg-Lite pingpong RTOS project is a simple demonstration program that uses the
MCUXpresso SDK software and the RPMsg-Lite library and shows how to implement the inter-core
communicaton between cores of the multicore system. The primary core releases the secondary core
from the reset and then the inter-core communication is established. Once the RPMsg is initialized
and endpoints are created the message exchange starts, incrementing a virtual counter that is part
of the message payload. The message pingpong finishes when the counter reaches the value of 100.
Then the RPMsg-Lite is deinitialized and the procedure of the data exchange is repeated again.

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
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
The log below shows the output of the RPMsg-Lite pingpong RTOS demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMsg demo starts

Copy CORE1 image to address: 0x20200000, size: 15548
Primary core received a msg
Message: Size=4, DATA = 1
Primary core received a msg
Message: Size=4, DATA = 3
Primary core received a msg
Message: Size=4, DATA = 5
.
.
.
Primary core received a msg
Message: Size=4, DATA = 101

RPMsg demo ends
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.

Note:
To download binary of cm7 core into qspiflash and boot from qspiflash directly, following steps are needed:
1. Compile flash target of the project, and get the binaray file "hello_world.bin".
3. Set the SW1: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J11.
4. Drop the binaray into disk "RT1160-EVK" on PC.
5. Wait for the disk disappear and appear again which will take couple of seconds.
7. Reset the board by pressing SW3 or power off and on the board. 
(If default boot core is cm4, binary of cm4 could be downloaded and boot according to steps above.)

Note:
To download and debug IAR EW project using J-Link (replacing the default CMSIS-DAP debug probe), following steps are needed:
1. Remove J6 and J7 jumpers.
2. Attach the J-Link probe (J-Link Plus / J-Trace) to the board using the J1 connector.
3. Set "J-Link / J-Trace" in CM7 project options -> Debugger -> Setup panel (replacing CMSIS-DAP option).
4. Unselect the "Use macro file(s)" in CM7 project options -> Debugger -> Setup panel.
5. Enable "Use command line options" in CM7 project options -> Debugger -> Extra Options panel 
   (--jlink_script_file=$PROJ_DIR$/../evkmimxrt1160_connect_cm4_cm7side.jlinkscript command line option is applied).
5. Click on "Download and Debug" button. During the loading process you can be asked by J-Link sw
   to select the proper device name (MIMXRT1166XXXA_M7 is unknown). Click O.K. and choose the MIMXRT1166xxxxA device.
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
