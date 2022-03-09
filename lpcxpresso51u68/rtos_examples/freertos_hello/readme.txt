Overview
========
The Hello World project is a simple demonstration program that uses the SDK UART drivere in
combination with FreeRTOS. The purpose of this demo is to show how to use the debug console and to
provide a simple project for debugging and further development.

The example application creates one task called hello_task. This task print "Hello world." message
via debug console utility and suspend itself.




Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso51U68 board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.
Running the demo
================
After the board is flashed the Tera Term will print "Hello world" message on terminal.

Example output:
Hello world.
