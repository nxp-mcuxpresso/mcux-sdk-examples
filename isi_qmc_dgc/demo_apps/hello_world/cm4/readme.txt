Overview
========
The Hello World demo application provides a sanity check for the new SDK build environments and board bring up.
The Hello World demo prints the "Releasing CM4", "ISI QMC code started" and "ISI QMC code SLAVE core started" strings to the terminal using the SDK UART drivers.
The purpose of this demo is to show how to use the UART, and to provide a simple project for debugging and further development.
Note: Only Serial Download (SDP) and running from RAM are supported in the current version of the base demo. Internal boot and a new flashloader for the ISI QMC HW will be available at a later time.


Toolchain supported
===================
- MCUXpresso  11.5.0

Hardware requirements
=====================
- ISI-QMC-DGC-02 [daughter card]
- ISI-QMC-DB-02  [digital board] (if serial output is required)

Board settings
==============
- Set the boot pins to Serial Download (SDP) mode

Prepare the Demo
================
1.  If serial output is desired, insert the daughter card into the digital board.
2.  Connect a J-Link debug probe to the JTAG pins on the daughter card.
3.  Connect a USB cable between the host PC and the daughter card (without serial output) or the digital board [J48] (with serial output).
(Note: If you don't see two new devices in the COM ports in the device manager, install these drivers: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
4.  Open two serial terminals with the following settings (one for each core):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the cm7 program to the target board.
6.  Download the cm4 program to the target board.
7.  Launch the debugger for the cm7 project in MCUXpresso to begin running the cm7 demo.
(Note: Leave the cm7 debug session running. MCUXpresso can debug both cores at the same time.)
8.  Launch the debugger for the cm4 project in MCUXpresso to begin running the cm4 demo.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Releasing CM4
ISI QMC code SLAVE core started
ISI QMC code started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
