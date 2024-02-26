Overview
========
The Multicore RPMsg-Lite pingpong TZM project is a simple demonstration program that uses the
MCUXpresso SDK software and the RPMsg-Lite library and shows how to implement the inter-core
communication between cores in the TrustZone-M (TZM) environment.
The TZM is configured in the BOARD_InitTrustZone() function. The secondary core is put into
the secure domain as well as the shared memory for RPMsg messages exchange. The primary core 
boots in the secure domain, it releases the secondary core from the reset and then the inter-core 
communication is established in the secure domain. Once the RPMsg is initialized
and the secure endpoint is created the message exchange starts, incrementing a virtual counter that is part
of the message payload. The message pingpong in the secure domain finishes when the counter reaches 
the value of 50. Then the secure domain is kept running and the non-secure portion of the application
is started. The non-secure RPMsg-Lite endpoint is created in the non-secure domain and the new message 
pingpong is triggered, exchanging data between the secondary core secure endpoint and the primary core 
non-secure endpoint. Veneer functions defined in the secure project are called from the non-secure
domain during this process. This message pingpong finishes when the counter reaches the value of 1050. 
Debug prints from the non-secure portion of the application are routed into the secure domain using
the dedicated veneer function.

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjusted in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application.
The shared memory assignment to the secure domain is done in the BOARD_InitTrustZone() function.

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
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
The Multicore RPMsg-Lite pingpong TZM project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and 
configurations in default state when running this demo.


Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J17) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the primary core secure project to the target board. Note: when an IDE is used there is typically no need to care about both secure and non-secure
    portion of the application separately, the secure project debugging is triggered only, see the "TrustZone application debugging" section below.

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================
The log below shows the output of the RPMsg-Lite pingpong TZM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copy CORE1 image to address: 0x2004e000, size: 9952

RPMsg demo starts

Data exchange in secure domain
Primary core received a msg
Message: Size=4, DATA = 1
Primary core received a msg
Message: Size=4, DATA = 3
Primary core received a msg
Message: Size=4, DATA = 5
.
.
.
Primary core received a msg
Message: Size=4, DATA = 51

Entering normal world now.

Non-secure portion of the application started!

Data exchange in non-secure domain
Primary core received a msg
Message: DATA = 1001
Primary core received a msg
Message: DATA = 1003
Primary core received a msg
Message: DATA = 1005
.
.
.
Primary core received a msg
Message: DATA = 1051

RPMsg demo ends
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.


TrustZone Application Development in SDK
Every TrustZone based application consists of two independent parts - secure part/project and non-secure part/project.

The primary core secure project is stored in <application_name>\<primary core name>\<application_name>_s directory.
The primary core non-secure project is stored in <application_name>\<primary core name>\<application_name>_ns directory. 

The secure projects always contains TrustZone configuration and it is executed after device RESET. The secure project usually
ends by jump to non-secure application/project.
If IDE allows the two primary core projects in single workspace, the user can also find the project with <application_name>.
This project contains both secure and non-secure projects in one workspace (Keil MDK, IAR) and it allows to user easy transition from
one to another project.

Project Structure
The all TrustZone files related to TrustZone are located in trustzone virtual directory. The files are:

- tzm_config.c
- tzm_config.h
- veneer_table.c
- veneer_table.h

File tzm_config.c, tzm_config.h
This file is used by secure project only. It contains one function BOARD_InitTrustZone(), which configures complete TrustZone
environment. It includes SAU, MPU's, AHB secure controller and some TrustZone related registers from System Control Block.
This function is called from SystemInitHook() function, it means during system initialization.

File veneer_table.c, veneer_table.h
This file defines all secure functions (secure entry functions) exported to normal world. This file is located in secure
project only. While header file is used by both secure and non-secure projects. The secure entry functions usually contain
validation of all input parameters in order to avoid any data leak from secure world.

The files veneer_table.h and <application_name>_s_import_lib.o or <application_name>_s_CMSE_lib.o create the connection
between secure and non-secure projects. The library file is generated by linker during compilation of secure project and
it is linked to the non-secure project as any other library.

TrustZone application compilation
As first compile primary core secure project since CMSE library is needed for compilation of non-secure project. 
After successful compilation of primary core secure project, compile primary core non-secure project.

TrustZone application debugging
- Download both output file into device memory
- Start execution of secure project since secure project is going to be executed after device RESET.

If IDE (Keil MDK, IAR) allows to manage download both output files as single download, the primary core secure project
is configured to download both secure and non-secure output files so debugging can be fully managed
from primary core secure project.

Device header file and secure/non-secure access to the peripherals
Both secure and non-secure project uses identical device header file. The access to secure and non-secure aliases for all peripherals
is managed using compiler macro __ARM_FEATURE_CMSE.

For secure project using <PERIPH_BASE> means access through secure alias (address bit A28=1), 
using <PERIPH_BASE>_NS means access through non-secure alias(address bit A28=0)
For non-secure project using <PERIPH_BASE> means access through non-secure alias (address bit A28=0). 
The non-secure project doesn't have access to secure memory or peripherals regions so the secure access is not defined.

J-Link debugger usage
J-Link V6.71a and above needs to be used in case the J-Link Commander is used for loading individual parts of this application 
(using loadfile or loadbin commands).
