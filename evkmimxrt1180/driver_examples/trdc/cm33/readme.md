Overview
========
The Multicore TRDC demo application demonstrates how to use TRDC on a multicore system
to control the access policy of the memory block checker(MBC) and memory region
checker(MRC), and how to configure the domain access control(DAC) for bus masters
to access both secure and non-secure domains.

First, the primary core example uses 1 domain for secure access demo. The primary
core which is secure first sets a MRC memory region and a MBC memory block to non-secure
access only, then trys to access the memory which triggers the hardfault. In hardfault
the access control is changed to secure access to resolve the error.
Then, the primary core releases the secondary core from the reset. The secondary core
example uses 2 domains, one is for secure access and the other is for non-secure access.
By enabling different DAC configuration, different domain is used for the access control.
The secure access process is the same as the primary core.
In the non-secure access demo, the secondary core sets a MRC memory region and a MBC memory
block to secure access only, and switch to the non-secure DAC configuration which force
the core to be non-secure. Then trys to access the memory which triggers the hardfault.
In hardfault certain flags stored in flash is set to let primary core know and update
the configuration to non-secure access to resolve the error, because the TRDC is secure
access only.

SDK version
===========
- Version: 2.16.000

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
3.  Compile the CM7 then the CM33 project to get the binary trdc_cm33.bin.
4.  Follow the instruction of SPSDK usage in SDK, to download and run the multicore image combined with edgelock FW.

Running the demo
================
The log below shows the output of the trdc multicore demo in the terminal window, if the CM33 uses TRDC2 and CM7 uses TRDC1 by default:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TRDC example starts on primary core
In primary core example we use 1 domain for secure access only.
Set the selected MBC block to non-secure for domain that is secure access only
Core0 MBC violent access at address: 0x47400000
The MBC selected block is accessiable for secure master now
Set the selected MRC region to non-secure for domain that is secure access only
Core0 MRC violent access at address: 0x 4100000
The MRC selected region is accessiable for secure master now
TRDC example succeeds on primary core
Copy Secondary core image to address: 0x303c0000, size: 16208
Starting Secondary core.
The secondary core application has been started.

In secondary core demo we use 2 domains, one is for secure access only, and the other is for non-secure access only.
Secondary core secure access demo starts.
Set the selected MBC block to non-secure for domain that is secure access only
Core1 MBC violent access at address: 0x42000000
The MBC selected block is accessiable for secure master now
Set the selected MRC region to non-secure for domain that is secure access only
Core1 MRC violent access at address: 0x60000000
The MRC selected region is accessiable for secure master now
Secondary core non-secure access demo starts.
Set the selected MBC block to non-secure for domain that is secure access only
Set the selected MRC region to non-secure for domain that is secure access only
Core1 MBC violent access at address: 0x42000000
The MBC selected block is accessiable for non-secure master now
Core1 MRC violent access at address: 0x60000000
The MRC selected region is accessiable for non-secure master now
TRDC example succeeds on secondary core

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy Secondary core image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.
