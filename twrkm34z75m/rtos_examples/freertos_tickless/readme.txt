Overview
========
This document explains the freertos_tickless example. It shows how the CPU enters the sleep mode and then
it is woken up either by expired time delay using low power timer module or by external interrupt caused by a
user defined button.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- TWR-KM34Z75M board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.
Prepare the Demo
================
Connect a serial cable from the debug UART port of the board to the PC. Start Tera Term
(http://ttssh2.osdn.jp) and make a connection to the virtual serial port.

1. Start Tera Term
2. New connection -> Serial
3. Set appropriate COMx port (x is port number) in Port context menu. Number is provided by operation
   system and could be different from computer to computer. Select COM number related to virtual
   serial port. Confirm selected port by OK button.
4. Set following connection parameters in menu Setup->Serial port...
        Baud rate:    115200
        Data:         8
        Parity:       none
        Stop:         1
        Flow control: none
5.  Confirm selected parameters by OK button.

Running the demo
================
After flashing the example to the board the console will start printing the number of tick count periodically
when the CPU is running.
To wake up the CPU by external interrupt, press the button, that is specified at the beginning of the
example (SWx where x is the number of the user defined button). When the button is pressed, the console prints
out the "CPU woken up by external interrupt" message.

Example output:

Press any key to start the example
Tickless Demo example
Press or turn on SWx to wake up the CPU

Tick count :
0
5000
10000
CPU woken up by external interrupt
15000
20000
25000

Explanation of the example
The example application prints the actual tick count number every time after the specified
delay. When the vTaskDelay() is called, the CPU enters the sleep mode for that defined period
of time.

While the CPU is in sleep mode and the user defined button is pressed, the CPU is woken up
by the external interrupt and continues to sleep after the interrupt is handled.
The period of time delay is not changed after the external interrupt occurs.
