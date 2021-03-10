Overview
========
The usart_hardware_flow_control example project demonstrates the usage
of the hardware flow control function. This example sends data to itself(loopback),
and hardware flow control is enabled in the example. The CTS(clear-to-send)
pin is for transmiter to check if receiver is ready, if the CTS pin is asserted,
transmiter starts to send data. The RTS(request-to-send) pin is for receiver
to inform the transmiter if receiver is ready to receive data. So, please connect
RTS to CTS pin directly.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
USART hardware flow control on one board.
The USART sends data to itself(loopback), and hardware flow control function is 
enabled on this example. So please make sure the correct connection for example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pin Name    Board Location    connect to     Pin Name    Board Location
USART_TX    J13   Pin 13      ----------     USART_RX    J13   pin 15
USART_CTS   J13   Pin 16      ----------     USART_RTS   J13   Pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
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
