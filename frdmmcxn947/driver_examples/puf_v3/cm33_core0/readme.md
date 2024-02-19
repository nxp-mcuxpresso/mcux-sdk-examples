Overview
========
The PUF Example project is a demonstration program that uses the KSDK software implement secure key storage using PUF software driver.

Usual use consists of these steps:
1. 	Enroll: The controller retrieves the Startup Data (SD) from the memory (SRAM), derives a digital fingerprint, 
	generates the corresponding Activation Code (AC) and sends it to the storage system. 
	Perform this step only once for each device. There is a control register that can block further enrollment. 
	This control register is write only and is reset on a power-on reset.

2. 	Start: The AC generated during the enroll operation and the SD are used to reconstruct the digital fingerprint. 
	It is done after every power-up and reset.

3. 	Generate Key: The controller generates an unique key and combines it with the digital fingerprint to output a key code. 
	Each time a Generate Key operation is executed a new unique key is generated.

4. 	Set Key: The digital fingerprint generated during the Enroll/Start operations and the key provided by the Client Design (CD) 
	are used to generate a Key Code. This KC can be stored externally. Perform this operation only once for each key.

5. 	Get Key: The digital fingerprint generated during the Start operation and the KC generated during a Set Key operation 
	are used to retrieve a stored key. Perform this operation every time a key is needed.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- MCUXpresso  11.9.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============

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
When the demo runs successfully, the log would be seen on the MCU-Link terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~
PUFv3 driver example
1510000:TestSelftest:PASS
1510001:TestGenerateRandomData:PASS
1510002:TestStart:PASS
1510003:TestDeriveDeviceKey:PASS
1510004:TestRandomWrapUnwrap:PASS
1510005:TestUserKeyWrapUnwrap:PASS
1510006:TestStop:PASS
1510007:TestZeroize:PASS
-----------------------
8 Tests 0 Failures 0 Ignored
OK
~~~~~~~~~~~~~~~~~~~~~~~~~~~

