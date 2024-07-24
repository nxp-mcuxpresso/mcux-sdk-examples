Overview
========
The LIN (Local Interconnect Network) demo application is a use-case of LIN low level driver built on Low Power UART. This application demonstrates a signal transfer between the two nodes. Master node and slave node signals are represented by the character array. Both master and slave node must have implemented the timer that is used by timeout service routine every 500 us and master node use the same timer to check for switch of the frame table scheduler every 5 ms.
The application and driver use the state machines to handle various of states of the bus on different levels.
User can use the SW3 button to set master node to sleep and SW2 button to wakeup the master node which is indicated by LED state.
This application supports only unconditional frame types.

Note: Autobaudrate feature should stay disabled since the driver needs to be updated.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Two micro USB cables
- Two KW45B41Z-EVK boards
- Personal Computer
- Power adapter 12 V
- Three Dupont female-to-female wires

Board settings
==============
- Connect J11-1 of the two boards
- Connect J11-2 of the two boards
- Connect J11-4 of the two boards
- Connect 12 V adapter to J9
- Short the JP19-1 and JP19-2 with the jumper

Prepare the Demo
================
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the boards.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
6.  Download the program for master node to the target board.

Running the demo
================
Master starts communication on button SW2 press. First frame is sent from master to slave. Slave will update its frame buffer and continue to send its own frame. After that slave will repeate the response from master and master will send the new frame in the next turn. The next two frames are of diagnostic type and could be used for customer purposes (for example LIN cluster go to sleep command). This sequence is than repeated.

If communication is succesfull the master and slave node will both repeatedly print:

LIN DEMO
SLAVE
LIN DEMO
MASTER
