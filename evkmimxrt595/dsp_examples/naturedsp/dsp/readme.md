Overview
========
The dsp_naturedsp demo application a short self-test of selected NatureDSP functions.

This demo contains two applications:
- cm33/ is the ARM application for Cortex-M33 core
- dsp/ is the DSP application for HiFi4 core

The release configurations of the demo will combine both applications into one ARM
image.  With this, the ARM core will load and start the DSP application on
startup.  Pre-compiled DSP binary images are provided under dsp/binary/ directory.

The debug configurations will build two separate applications that need to be
loaded independently.  The ARM application will power and clock the DSP, so
it must be loaded prior to loading the DSP application.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- Xtensa C Compiler  14.01

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the debug USB port (J40) on the board
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program for CM33 core to the target board.
4. Launch the debugger in your IDE to begin running the demo.
6. If building debug configuration, download the program for DSP core to the target board.
7. If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to
begin running the demo.

### Notes:
- DSP image can only be debugged using J-Link debugger. See
'Getting Started with Xplorer for EVK-MIMXRT595.pdf' for more information.

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display the following:
```
    NatureDSP demo start
    Going to start DSP core...
```
The terminal is then deinitialized so it can be used by DSP.

Running the demo DSP
====================
When the demo runs successfully, similar output will be appended to the terminal:
```
    Running NatureDSP library on DSP core
    NatureDSP library version: x.y.z
    NatureDSP library API version: x.y.z

    /*FFT TEST START*/
    FFT 256 takes x cycles
    /*FFT TEST END with 0*/

    /*VECTOR DOT TEST START*/
    VECTOR DOT 16 takes y cycles
    /*VECTOR DOT TEST END with 0 */

    /*VECTOR ADD TEST START*/
    VECTOR ADD 32 takes z cycles
    /*VECTOR ADD TEST END with 0 */

    /*VECTOR MAX TEST START*/
    VECTOR MAX 1024f takes xx cycles
    /*VECTOR MAX TEST END with 0 */

    /*IIR TEST START*/
    IIR 32 1024 takes yy cycles
    /*IIR TEST END with 0 */

    /*FIR BLMS TEST START*/
    FIR BLMS 64 64 takes zz cycles
    /*FIR BLMS TEST END with 0 */
```

