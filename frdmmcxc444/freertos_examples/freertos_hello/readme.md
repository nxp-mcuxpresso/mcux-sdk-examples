Overview
========
The Hello World project is a simple demonstration program that uses the SDK UART driver in
combination with FreeRTOS. The purpose of this demo is to show how to use the debug console and to
provide a simple project for debugging and further development.

The example application creates one task called hello_task. This task print "Hello world." message
via debug console utility and suspend itself.




SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXC444 board
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
