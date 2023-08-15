Overview
========
The usart_transfer_hardware_flow_control example project demonstrates the usage
of the hardware flow control function. This example sends data to itself(loopback),
and hardware flow control is enabled in the example. The CTS(clear-to-send)
pin is for transmiter to check if receiver is ready, if the CTS pin is asserted,
transmiter starts to send data. The RTS(request-to-send) pin is for receiver
to inform the transmiter if receiver is ready to receive data. So, please connect
RTS to CTS pin directly.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso845MAX board
- Personal Computer

Board settings
==============
USART hardware flow control on one board.
The USART sends data to itself(loopback), and hardware flow control function is 
enabled on this example. So please make sure the correct connection for example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pin Name    Board Location    connect to     Pin Name    Board Location
USART_TX    J2    Pin 4       ----------     USART_RX    J2    pin 3
USART_CTS   J2    Pin 5       ----------     USART_RTS   J2    Pin 6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J4) on the target board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is USART hardware flow control example on one board.
This example will send data to itself and will use hardware flow control to avoid the overflow.
Please make sure you make the correct line connection. Basically, the connection is:
      USART_TX    --     USART_RX
      USART_RTS   --     USART_CTS
Data matched! Transfer successfully.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
