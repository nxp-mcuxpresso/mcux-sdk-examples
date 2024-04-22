Overview
========
The ELE Crypto NVM Example project is a demonstration program that uses the MCUX SDK
software to perform non volatile memory (flash) operations with EdgeLock Enclave (ELE)
and usage of its services with direct use of Messaging Unit driver.


SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

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
Open session successfully. Session ID: 0x76f77347

****************** Create Key Store ***********************
Open service and create Key Store successfully. Key Store ID: 0x76f77dc7

****************** Open NVM Storage service ***************
Open NVM Storage service successfully. Handle ID: 0x76f77d8f

****************** Key Management Open ********************
Open Key management service successfully. Key Handle ID: 0x76f77d5f

****************** Key Generate (ECC NIST P256) ***********
Key generated successfully. Key Pair ID: 0x3fffffff

****************** Key Generate (AES ECB 128) *************
Key for AES-ECB generated successfully. Key ID: 0x3ffffffe

****************** Open cipher service ********************
Open Cipher service successfully. Handle ID: 0x76f746c7

****************** Encrypt Cipher AES-ECB *****************
AES-ECB encryption success, Output size returned: 0x20
****************** Key Management Export Chunks ***********
Chunks exported successfully.
         Storage Master chunk at address:        0x47ffe30 , size 100 Bytes
         Key Store chunk at address:     0x204840d8 , size 896 Bytes
         Key Group chunk at address:     0x20484008 , size 200 Bytes
****************** Open Sign service **********************
Sign service open successfully. Signature generation handle ID: 0x76f74697

****************** ECC NIST P256 sign *********************
Signature generated successfully.

****************** Open Storage service *******************
Open Storage service successfully. Handle ID: 0x76f74667

****************** Data Storage Store *********************
Data Storage successfully. Exported Chunk at address 0x20484460

****************** Data Storage Retrieve ******************
Data Storage Retrieve successfully. Exported Chunk at address 0x204c0000
Success: Retrieved data at addr 0x204c0000 are same as stored plain data at 0x47ffee8
****************** Key Delete (ECC NIST P256) *************
Key deleted successfully. Key Pair ID: 0x3fffffff

Close data storage session successfully.

****************** Close cipher service ***********************
Cipher service closed successfully.

****************** Close Sign service *********************
Sign service close successfully.

****************** Close Key Management Service ***************
Close Key Management Service successfully.

****************** Close Key Store ****************************
Close Key Store successfully.

Close NVM storage session successfully.

****************** Close EdgeLock session *********************
Close session successfully.

****************** Open EdgeLock session ******************
Open session successfully. Session ID: 0x76f77347

****************** Open NVM Storage service ***************
Open NVM Storage service successfully. Handle ID: 0x76f7731f

****************** Import Master storage chunk ************
Import Master storage chunk successfully.

****************** Open Key Store *************************
Open service and create Key Store successfully. Key Store ID: 0x76f77d97

****************** Key Management Open ********************
Open Key management service successfully. Key Handle ID: 0x76f77d5f

****************** Key Management Chunk Import ************
Key Management Key group chunk Import successfully

****************** Open cipher service ********************
Open Cipher service successfully. Handle ID: 0x76f746c7

****************** Decrypt Cipher AES-ECB *****************
AES-ECB decrypted data match the original plain text - success.

****************** Open Sign service **********************
Sign service open successfully. Signature generation handle ID: 0x76f74697

****************** ECC NIST P256 sign (second time) *******
Second Signature generated successfully.

****************** Open Verify service ********************
Verify service open successfully. Verification handle ID: 0x76f74667

****************** ECC NIST P256 verify *******************
ECC NIST p256 Signature verified successfully!

****************** ECC NIST P256 verify (second) **********
ECC NIST p256 Second Signature verified successfully!

****************** Key Management Chunk Export ************
Key Management Key group chunk Export successfully

****************** Close cipher service ***********************
Cipher service closed successfully.

****************** Close Verify service *******************
Verify service close successfully.

****************** Close Key Management Service ***************
Close Key Management Service successfully.

****************** Close Key Store ****************************
Close Key Store successfully.

Close NVM storage session successfully.

****************** Close EdgeLock session *********************
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
