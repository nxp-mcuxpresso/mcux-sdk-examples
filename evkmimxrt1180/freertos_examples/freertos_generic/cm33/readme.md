Overview
========

This document explains the freertos_generic example. It is based on code FreeRTOS documentation from
http://www.freertos.org/Hardware-independent-RTOS-example.html. It shows combination of several
tasks with queue, software timer, tick hook and semaphore.

The example application creates three tasks. The prvQueueSendTask periodically sending data to
xQueue queue. The prvQueueReceiveTask is waiting for incoming message and counting number of
received messages. Task prvEventSemaphoreTask is waiting for xEventSemaphore semaphore given from
vApplicationTickHook. Tick hook give semaphore every 500 ms.

Other hook types used for RTOS and resource statistics are also demonstrated in example:
* vApplicationIdleHook
* vApplicationStackOverflowHook
* vApplicationMallocFailedHook



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
- MIMXRT1180-EVK board
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
After the board is flashed the Tera Term will start periodically printing the state of generic example.

Example output:
Event task is running.
Receive message counter: 1.
Receive message counter: 2.
Receive message counter: 3.
Receive message counter: 4.
Receive message counter: 5.
Receive message counter: 6.
Receive message counter: 7.
Receive message counter: 8.
Receive message counter: 9.
Receive message counter: 10.
Receive message counter: 11.
Receive message counter: 12.
Event task is running.
Receive message counter: 13.
Receive message counter: 14.
...
