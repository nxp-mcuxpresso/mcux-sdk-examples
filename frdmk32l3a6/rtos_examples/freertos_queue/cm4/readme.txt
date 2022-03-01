Overview
========

This document explains the freertos_queue example. This example introduce simple logging mechanism
based on message passing.

Example could be devided in two parts. First part is logger. It contain three tasks:
log_add().....Add new message into the log. Call xQueueSend function to pass new message into
              message queue.
log_init()....Initialize logger (create logging task and message queue log_queue).
log_task()....Task responsible for printing of log output.

Second part is application of this simple logging mechanism. Each of two tasks write_task_1 and
write_task_2 print 5 messages into log.



Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini USB cable
- FRDM-K32L3A6 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for JLink serial device with these settings:
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Log 0: Task1 Message 0

Log 1: Task2 Message 0

Log 2: Task1 Message 1

Log 3: Task2 Message 1

Log 4: Task1 Message 2

Log 5: Task2 Message 2

Log 6: Task1 Message 3

Log 7: Task2 Message 3

Log 8: Task1 Message 4

Log 9: Task2 Message 4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
