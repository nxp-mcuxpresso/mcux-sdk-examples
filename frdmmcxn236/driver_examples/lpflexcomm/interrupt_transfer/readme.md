Overview
========
	The lpflexcomm_interrupt_transfer example shows how to use lpi2c driver and lpuart driver to build a interrupt based application.In this example,
	lpi2c and lpuart use same lpflexcomm instance.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
To make LPFLEXCOMM example work, connections needed to be as follows:

        LPI2C3 SCL(J2-12)     -->          LPI2C2 SCL(J5-5)
        LPI2C3 SDA(J2-8)      -->          LPI2C2 SDA(J5-6)
		LPUART3 TX(J2-10)	  -->          LPUART2 RX(J1-2)
		LPUART3 RX(J2-6)	  -->          LPUART2 TX(J1-4)

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board. 
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 
LPFLEXCOMM example.
In this example 2 lpflexcomm instances are used.
First the lpflexcomm instances are used as I2C, I2C master of instance 1 sends data to I2C slave of instance2.
Then the lpflexcomm instances are used as UART, UART2 sends data back to UART1.
The next round UART1 sends data to UART2, then lpflexcomm instances switch back to I2C mode, 
I2C master of instance 1 reads data from I2C slave of instance 2 of the data received as UART.
Master will send data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  


Will send data using LPI2C --- MASTER TO SLAVE

Transfer all data matched! 

Slave received data by LPI2C:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

Will reply data using LPUART --- instance 2 TO instance 1

Transfer all data matched! 

Master received data by LPUART:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

Will reply data using LPUART --- instance 1 TO instance 2

Transfer all data matched! 

Slave received data by LPUART:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

Will reply data using LPI2C --- SLAVE TO MASTER

Transfer all data matched! 

Master received data by LPI2C:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  
DEMO END!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

