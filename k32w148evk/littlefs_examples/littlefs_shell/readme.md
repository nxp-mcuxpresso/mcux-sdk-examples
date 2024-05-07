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

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J14.
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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright  2022  NXP

LFS>>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
