Overview
========
This document explains the freertos_mpu example. This demo application utilizes TrustZone,
so it demonstrates following techniques for TrustZone applications development:
1. Application separation between secure and non-secure part
2. TrustZone environment configuration
3. Exporting secure function to non-secure world
4. Calling non-secure function from secure world
5. Configuring IAR, MDK, GCC and MCUX environments for TrustZone based projects


Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
Connect a USB2COM between the PC host and the board UART pins
boards           -               USB2COM
J14-Pin26                        Tx
J14-Pin28                        Rx
J14-Pin1                         GND

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J7) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The MPU demo creates 3 unprivileged tasks - One of which has Read Only access
to a shared memory region, the second one has Read Write access to the same
region and the third one has Read Write access to a non-secure counter. The task
with Read Only access reads from the shared memory but can not write into it
which would result in memory fault (User can try to uncomment respective line
of code to test this behavior). The Read Write task writes to the shared
memory. The third task which is created calls a secure side function and passes
a pointer to a callback function. The secure side function does two things:
1. It calls the provided callback function. The callback function increments
a non-secure counter.
2. It increments a secure counter and returns the incremented value.
After the secure function call finishes, it verifies that both the counters
are incremented.

