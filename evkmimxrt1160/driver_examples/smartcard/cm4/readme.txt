Overview
========
This example demonstrates the SDK Peripheral drivers working with different methods.

This example demonstrates use of smartcard driver API to read GSM sim card ICC-ID (Integrated circuit card identifier,
which should be placed on the card). 
Please be aware, that this example is just simple demonstration of smartcard driver API. It may not work with all types
of GSM sim cards.


Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Personal Computer
- GSM sim card

Board settings
==============
No special settings are required.

Note:
Card insert detection is not supported since there is no card insert detection circuit on this board.
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
        Flow control: one
5.  Confirm selected parameters by OK button.

Running the demo
================

***** SMARTCARD Driver Send Receive functionality example *****

Card inserted.
Deactivating card...Done!
Resetting/Activating card...Done!
Selecting Master root file.
Getting response of selection command.
Selecting ICC-ID file.
Reading binary ICC-ID.
Received smartcard ICC-IC: 000000000000000000

Send receive functionality example finished!
~~~~~~~~~~~~~~~~~~~~~
