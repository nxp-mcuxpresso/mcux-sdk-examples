Overview
========
This document explains the freertos_mutex example. It shows how mutex manage access to common
resource (terminal output).

The example application creates two identical instances of write_task. Each task will lock the mutex
before printing and unlock it after printing to ensure that the outputs from tasks are not mixed
together.

The test_task accept output message during creation as function parameter. Output message have two
parts. If xMutex is unlocked, the write_task_1 acquire xMutex and print first part of message. Then
rescheduling is performed. In this moment scheduler check if some other task could run, but second
task write_task+_2 is blocked because xMutex is already locked by first write task. The first
write_task_1 continue from last point by printing of second message part. Finaly the xMutex is
unlocked and second instance of write_task_2 is executed.




SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8MM6-EVK  board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
No special settings are required.



Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
After the board is flashed the Tera Term will start periodically printing strings synchronized by
mutex.

Example output:
"ABCD | EFGH"
"1234 | 5678"
"ABCD | EFGH"
"1234 | 5678"
