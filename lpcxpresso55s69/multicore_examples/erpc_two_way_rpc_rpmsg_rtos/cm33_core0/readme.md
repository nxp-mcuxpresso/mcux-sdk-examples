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

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
The Multicore eRPC Two Way RPC RTOS project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and
configurations in default state when running this demo.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
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
Copy CORE1 image to address: 0x20033000, size: 32780
Get number from core1:
Got number: 1
Start normal rpc call example.
RPC call example finished.

Get number from core1:
Got number: 2
Start normal rpc call example.
RPC call example finished.

Get number from core1:
Got number: 3
Start normal rpc call example.
RPC call example finished.
.
.
.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.
