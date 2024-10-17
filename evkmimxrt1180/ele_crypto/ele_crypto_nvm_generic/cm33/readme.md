Overview
========
The ELE Crypto NVM Example project is a demonstration program that uses the MCUX SDK
software to perform non volatile memory (flash) operations with EdgeLock Enclave (ELE)
and usage of its services with direct use of Messaging Unit driver.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
The log below shows the output in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EdgeLock Enclave secure Sub-System Driver Example:

****************** Load EdgeLock FW ***********************
EdgeLock FW loaded and authenticated successfully.

****************** Start RNG ******************************
EdgeLock RNG Start success.
EdgeLock RNG ready to use.

****************** Initialize ELE services ****************
ELE services initialized successfully.

****************** Open EdgeLock session ******************
Open session successfully. Session ID: 0x76f7758f

****************** Create Key Store ***********************
Open service and create Key Store successfully. Key Store ID: 0x76f7750f

****************** Open NVM Storage service ****************
Open NVM Storage service successfully. Handle ID: 0x76f772d7

****************** Key Management Open ********************
Open Key management service successfully. Key Handle ID: 0x76f772a7

****************** Open Storage service ********************
Open Storage service successfully. Handle ID: 0x76f77277

****************** Data Storage Store **********************
Data Storage successfully. Exported Chunk at address 0x20484008

****************** Key Management Export Chunks ***********
Chunks exported successfully.
         Storage Master chunk at address:        0x47ffefc , size 100 Bytes
         Key Store chunk at address:     0x20484098 , size 892 Bytes
Close data storage session successfully.

****************** Close Key Management Service ***********
Close Key Management Service successfully.

****************** Close Key Store ************************
Close Key Store successfully.

Close NVM storage session successfully.

****************** Close EdgeLock session *****************
Close session successfully.

****************** Open EdgeLock session ******************
Open session successfully. Session ID: 0x76f7758f

****************** Open NVM Storage service ****************
Open NVM Storage service successfully. Handle ID: 0x76f77567

****************** Import Master storage chunk ************
Import Master storage chunk successfully.

****************** Open Key Store ****************
Open service and create Key Store successfully. Key Store ID: 0x76f772df

****************** Key Management Open ********************
Open Key management service successfully. Key Handle ID: 0x76f772a7

****************** Open Storage service ********************
Open Storage service successfully. Handle ID: 0x76f77277

****************** Data Storage Retrieve *******************
Data Storage Retrieve successfully. Exported Chunk at address 0x4000008
Success: Retrieved data at addr 0x4000008 are same as stored plain data at 0x47fff70
Close data storage session successfully.

****************** Close Key Management Service ***********
Close Key Management Service successfully.

****************** Close Key Store ************************
Close Key Store successfully.

Close NVM storage session successfully.

****************** Close EdgeLock session *****************
Close session successfully.

End of Example with SUCCESS!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The IDs and addresses will/may be different on every run.

Note:
To download binary into qspiflash and boot from qspiflash directly, following steps are needed:
1. Compile flash target of the project, and get the binaray file "hello_world.bin".
3. Set the SW1: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J11.
4. Drop the binaray into disk "RT1180-EVK" on PC.
5. Wait for the disk disappear and appear again which will take couple of seconds.
7. Reset the board by pressing SW3 or power off and on the board. 
