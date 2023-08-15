Overview
========
The Multicore eRPC Two Way RPC RTOS project is a simple demonstration
program that uses the MCUXpresso SDK software and the Multicore SDK to show
how to implement the Remote Procedure Call between cores of the multicore system.
This multicore example shows how both the eRPC client and the eRPC server can be 
setup on one side/core (bidirectional communication) and how to handle callback
functions in eRPC.  
The primary core (Core0) creates client and server tasks first. The client task
releases the secondary core from the reset, initializes the RPMsg-Lite erpc transport
and once the server task is running it configures the arbitrated client. Then the
application logic is running.
The secondary core (Core1) creates client and server tasks two. The client task 
initializes the RPMsg-Lite erpc transport and once the server task is running it 
configures the arbitrated client. Then the application logic is running.
The client task logic of the Core1 is very simple, it repeatedly calls the
increaseNumber() erpc function that is implemented on the Core0 and that increments
the counter.  
The client task logic of the Core0 alternately issues either getNumberFromCore0() 
function implementation on the Core0 or getNumberFromCore1() function implementation 
on the Core1 (erpc call). Then, the nestedCallGetNumber() erpc function call is issued
that alternately triggers either getNumberFromCore1() function implementation 
on the Core1 (normal erpc call) or it triggers the getNumberFromCore0() function 
implementation on the Core0 (nested erpc call, routed through the Core1 erpc server).

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application.

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

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
The Multicore eRPC Two Way RPC RTOS project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and
configurations in default state when running this demo.

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

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================
The log below shows the output of the eRPC Two Way RPC RTOS demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Primary core started
Copy CORE1 image to address: 0x20200000, size: 15548
Get number from core1:
Got number: 4
Start normal rpc call example.
RPC call example finished.


Get number from core0:
getNumberFromCore0 function call: Actual number is 10
Got number: 10
Start nested rpc call example.
getNumberFromCore0 function call: Actual number is 10
RPC call example finished.


Get number from core1:
Got number: 15
Start normal rpc call example.
RPC call example finished.


Get number from core0:
getNumberFromCore0 function call: Actual number is 21
Got number: 21
Start nested rpc call example.
getNumberFromCore0 function call: Actual number is 21
RPC call example finished.


Get number from core1:
Got number: 27
Start normal rpc call example.
RPC call example finished.
.
.
.
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
