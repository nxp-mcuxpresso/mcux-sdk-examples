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
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- TWR-KM35Z75M board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.
Running the demo
================
After the board is flashed the Tera Term will show debug console output.

Example output:
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
