Overview
========
The Multicore Manager example application demonstrates advanced features of the MCMgr component.
In this demo, the primary core prints the "Hello World from the Primary Core!" string to the terminal
and then releases the secondary core from the reset. The secondary
core toggles an on-board LED indicating that the secondary core is running. It is also possible to
stop/start the secondary core during the runtime by pressing on-board buttons. The
on-board LED then indicates the change, it stops/starts toggling accordingly.

This application also shows how to use the Multicore Manager for
remote core monitoring and handling of events such as application, CoreUp, CoreDown and exception events.
Application-specific callback functions for events are registered by the MCMGR_RegisterEvent() API
on the primary core. Triggering these events is done using the MCMGR_TriggerEvent() API on the
secondary core. While CoreUp, CoreDown events are triggered from startup and MCMGR code, the
application event is triggered from the application.
The exception event is triggered in the secondary application after 100 LED toggles by trying
to write to flash. This raises the exception on the secondary core and triggers the
RemoteExceptionEvent on the primary core.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K32L3A6 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect the PC host and the board
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

Starting Secondary core.
The secondary core application has been started.

Press the SW1 button to Stop Secondary core.
Press the SW2 button to Start Secondary core again.
When no action is taken the secondary core application crashes intentionally after 100 LED toggles (simulated exception), generating the RemoteExceptionEvent to this core.
Use the Stop and then the Start button to get it running again.

.
.
.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
