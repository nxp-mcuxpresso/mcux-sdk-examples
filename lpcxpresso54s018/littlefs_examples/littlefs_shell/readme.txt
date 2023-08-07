Overview
========
The littlefs_shell demonstrates the capabilities of LittleFS FLASH filesystem:

After the example is loaded into the target and launched the shell prompt is printed to the console.
Type "help" to obtain list of available commands. The FLASH storage has to be formatted upon first use.
After mounting the filesystem it is possible to create/delete/list directories and read/write files using appropriate commands.
There is no concept of current directory in LittleFS, hence it is always necessary to specify full directory path.

List of supported commands:
  format       Formats the filesystem
  mount        Mounts the filesystem
  unmount      Unmounts the filesystem
  ls           Lists directory content
  rm           Removes file or directory
  mkdir        Creates a new directory
  write        Writes/appends text to a file
  cat          Prints file content

Example workflow:
To perform initial format of the storage, issue 'format yes' command.
Mount the storage by issuing 'mount' command.
Create new directory by 'mkdir mynewdir'.
Create new file in the 'mynewdir' directory by writing line of text to it using 'write mynewdir/foo.txt firstline'
Append another line to the same file using 'write mynewdir/foo.txt secondline'
Print the content of the file using 'cat mynewdir/foo.txt'. The expected output is:
  firstline
  secondline

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018 board
- Personal Computer

Board settings
==============
No special settings are required.

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
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
This demo is an external flash plain load demo, ROM will copy image in external flash to SRAMX to run:
1. Build the demo to generate a bin file.
   Note: If need to generate bin file using MCUXpresso IDE, below steps need to be followed:
         Set in example Properties->C/C++ Build->Settings->Build steps->Post-build steps->Edit
         enbable arm-none-eabi-objcopy -v -O binary "&{BuildArtifactFileName}" "&{BuildArtifactFileBaseName}.bin" 
         
         This plainload example linked the vector table to 0x00000000, but program to external flash 0x10000000.

2. Program the bin file to external on board flash via SEGGER J-FLASH Lite(V6.22 or higher):

   a. Open SEGGER J-FLASH Lite, select device LPC54S018.

   b. Click the 'Erase Chip' to erase the extrenal flash.(if can not success, press SW4 button and reset the board, and try to erase again)

   c. Select the bin data file, set the '.bin/Erase Start' address to 0x10000000, then click 'Program Device'
Note: Please use above way to program the binary file built by armgcc tool chain to external flash. 
      For IAR, KEIL, MCUXpresso IDE, you can use the IDE tool to program the external flash.  
The log below shows example output of the eeprom example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHELL (build: Feb 28 2018)
Copyright  2017  NXP
LFS>>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
