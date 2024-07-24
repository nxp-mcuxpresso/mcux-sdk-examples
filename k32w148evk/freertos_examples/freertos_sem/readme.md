Overview
========
This document explains the freertos_sem example, what to expect when running it and a brief
introduction to the API. The freertos_sem example code shows how semaphores works. Two different
tasks are synchronized in bilateral rendezvous model.

The example uses four tasks. One producer_task and three consumer_tasks. The producer_task starts by
creating of two semaphores (xSemaphore_producer and xSemaphore_consumer). These semaphores control
access to virtual item. The synchronization is based on bilateral rendezvous pattern. Both of
consumer and producer must be prepared to enable transaction.




SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Personal Computer
- K32W148-EVK board

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Producer_task created.
Consumer_task 0 created.
Consumer_task 1 created.
Consumer_task 2 created.
Consumer number: 0
Consumer 0 accepted item.
Consumer number: 1
Consumer number: 2
Producer released item.
Consumer 0 accepted item.
Producer released item.
Consumer 1 accepted item.
Producer released item.
Consumer 2 accepted item.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

