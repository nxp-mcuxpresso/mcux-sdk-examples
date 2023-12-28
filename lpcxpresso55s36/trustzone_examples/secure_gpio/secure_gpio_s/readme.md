Overview
========
The Secure GPIO demo application demonstrates using of secure GPIO peripheral and GPIO mask feature in
AHB secure controller. The secure GPIO is standard GPIO peripheral supporting 32 pins on port 0 only.
Having two GPIO peripherals allows user to configure one GPIO peripheral into secure world and another
one into normal world. Thus for all pins on port 0, user can select whether the pin is controlled
from secure or non-secure domain (using IOCON).
Since the every GPIO pin allows to read its state regardless of its configuration, the AHB secure
controller has build in feature GPIO mask for all GPIO pins. It is recommended to mask GPIO pins
for all peripherals assigned to secure domain in order to eliminate information leak from GPIO pins.
For example UART is assigned to secure world. But GPIO peripheral still allows to read state of
TxD and RxD pins. This information can be use to decode UART communication from normal world, which
can lead to unwanted information leak. 

This Secure GPIO demo uses one user button (let's call it LED button), which is read from both secure
(using "secure" GPIO) and normal world (using "standard" GPIO), and two LED diodes, one controlled from
secure and second one from normal world. Once LED button is pressed, both secure and non-secure
worlds detect this user action (logical zero is read) and both LEDs are switched ON (first LED is controlled
from secure world and second one from non-secure world). So both LEDs are controlled by LED button.
The second part of the demo demonstrates AHB secure controller GPIO mask feature. This feature is
controlled by second user button (let's call it MASK button). If the MASK button is not pressed, the GPIO
mask feature is disabled and normal world can read the state of LED button. Therefore LED button still
controls both LEDs as described in previous paragraph.
If the MASK button is pressed, the GPIO mask feature is enabled. In this case normal world cannot read
state of state of first button and logical zero is read only. Since logical zero corresponds to pressed LED
button, the LED, controlled from normal world, is immediately switched ON regardless of the state of LED
button. The LED, controlled from secure world, can be still controlled by LED button, because the state of
this button is still available to the secure world via GPIO assigned to secure world. Below is a table
illustrating the individual button and LED states.
For user buttons and LEDs assignment on particular board, see chapter "Running the demo".

| Input        |            | OUTPUT       |              |
|--------------|------------|--------------|--------------|
| ALLOW button | LED button | "secure" LED | "normal" LED |
| PRESSED      | RELEASED   | ON           | ON           |
| RELEASED     | RELEASED   | OFF          | OFF          |
| PRESSED      | PRESSED    | ON           | ON           |
| RELEASED     | PRESSED    | OFF          | ON           |

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55s36 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2. Download the program to the target board.
3. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Use SW3 button to control blue and green LED and SW1 to control GPIO mask feature.
If SW3 button is pressed (logical zero is read), the GREEN LED is switched on in secure world
and blue LED is switched on from normal world. So both blue and green LEDs are controlled by SW3 button.
If also SW1 button is pressed, the GPIO mask feature is activated. In this case the blue LED starts to light
regardless of SW3 button state, since normal GPIO cannot read state of SW3 button (only the logic 0 is read). 
The green LED can be still controlled by SW3 button.

TrustZone Application Development in SDK
Every TrustZone based application consists of two independent parts - secure part/project and non-secure part/project.

The secure project is stored in <application_name>\<application_name>_s directory.
The non-secure project is stored in <application_name>\<application_name>_ns directory. 

The secure projects always contains TrustZone configuration and it is executed after device RESET. The secure project usually
ends by jump to non-secure application/project.
If IDE allows the two projects in single workspace, the user can also find the project with <application_name>.
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
As first compile secure project since CMSE library is needed for compilation of non-secure project. 
After successful compilation of secure project, compile non-secure project.

TrustZone application debugging
- Download both output file into device memory
- Start execution of secure project since secure project is going to be executed after device RESET.

If IDE (Keil MDK, IAR) allows to manage download both output files as single download, the secure project
is configured to download both secure and non-secure output files so debugging can be fully managed
from secure project.

Device header file and secure/non-secure access to the peripherals
Both secure and non-secure project uses identical device header file. The access to secure and non-secure aliases for all peripherals
is managed using compiler macro __ARM_FEATURE_CMSE.

For secure project using <PERIPH_BASE> means access through secure alias (address bit A28=1), 
using <PERIPH_BASE>_NS means access through non-secure alias(address bit A28=0)
For non-secure project using <PERIPH_BASE> means access through non-secure alias (address bit A28=0). 
The non-secure project doesn't have access to secure memory or peripherals regions so the secure access is not defined.
