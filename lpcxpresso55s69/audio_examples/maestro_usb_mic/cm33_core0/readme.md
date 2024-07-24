Overview
========
The maestro_usb_mic application demonstrates audio processing on the ARM cortex core
utilizing the Maestro Audio Solutions library.

The application is controlled by commands from a shell interface using serial console.

Type "help" to see the command list. Similar description will be displayed on serial console:
```
    >> help

    "help": List all the registered commands

    "exit": Exit program

    "version": Display component versions

    "usb_mic": Record MIC audio and playback to the USB port as an audio 2.0
                microphone device.
        USAGE:     usb_mic <seconds>
        <seconds>  Time in seconds how long the application should run.
                When you enter a negative number the application will
                run until the board restarts.
        EXAMPLE:   The application will run for 20 seconds: usb_mic 20
    >>
```

After running the "usb_mic" command, the USB device will be enumerated on your host.
User will see the volume levels obtained from the USB host as in the example below.
This is just an example application. To leverage the values, the demo has to be modified.

### Notes
1. When device functionality is changed, please uninstall the previous PC driver to make
   sure the device with changed functionality can run normally.
2. If you're having audio problems on Windows 10 for recorder, please disable signal
   enhancement as the following if it is enabled and have a try again.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- 2x Micro USB cable
- JTAG/SWD debugger
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
1. Set the hardware jumpers (Tower system/base module) to default settings.

Prepare the Demo
================
1. Connect the first micro USB cable between the PC host and the debug USB port on the board.
2. Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Connect the second micro USB cable between the PC host and the USB port on the board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin
   running the demo.

Running the demo
================
When the example runs successfully, you should see similar output on the serial terminal as below:
```
    *************************************************
    Maestro audio USB microphone solutions demo start
    *************************************************

    Copyright  2022  NXP
    [APP_Shell_Task] start

    >> usb_mic -1

    Starting maestro usb microphone application
    The application will run until the board restarts
    [STREAMER] Message Task started
    Starting recording
    [STREAMER] start usb microphone
    Set Cur Volume : 1f00
```

