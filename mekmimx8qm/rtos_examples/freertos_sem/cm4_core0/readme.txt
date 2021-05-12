Overview
========
This document explains the freertos_sem example, what to expect when running it and a brief
introduction to the API. The freertos_sem example code shows how semaphores works. Two different
tasks are synchronized in bilateral rendezvous model.

The example uses four tasks. One producer_task and three consumer_tasks. The producer_task starts by
creating of two semaphores (xSemaphore_producer and xSemaphore_consumer). These semaphores control
access to virtual item. The synchronization is based on bilateral rendezvous pattern. Both of
consumer and producer must be prepared to enable transaction.




Toolchain supported
===================
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- i.MX8QM MEK CPU Board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board.
2.  Connect a USB cable between the host PC and the Debug port on the board (Refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for debug port information).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board (Please refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for how to run different targets).
5.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
Producer released item.
Consumer 0 accepted item.
Producer released item.
Consumer 1 accepted item.
Producer released item.
Consumer 2 accepted item
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
