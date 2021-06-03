Overview
========
The multicore Low Power demo application demonstrates how to leverage the auxiliary core for periodic
data collection from sensors and passing the data to the primary core for further processing.

The data from sensors is read with frequency of about 50Hz (every 20ms) by secondary core and stored
into a buffer. When the buffer contains 50 samples from sensors, the secondary core sends an interrupt
to the primary core to process the data. The primary core computes average values from temperature and
pressure and then prints them with last values from other sensors to the console.

The reading of data from sensors takes about 3ms, the rest of the time the cores are in deep sleep
mode to reduce power consumption. In order to switch to deep sleep mode, the Power API
POWER_EnterDeepSleep() is used and to wake it up the interrupt from uTick timer is used.

When both cores can't go to the deep sleep mode (the primary core processes data or the secondary core
reads data from the sensors), the core which has nothing to do goes to sleep mode by WFI. Also, when
the primary core is switched to the sleep mode, the flash memory is powered down by Power API
POWER_PowerDownFlash() and before waking up the primary core it is powered on by POWER_PowerUpFlash().

The demo is based on:
A. Fuks, "Sensor-hub sweet-spot analysis for ultra-low-power always-on operation," 2015 Symposium on VLSI Circuits (VLSI Circuits), Kyoto, 2015, pp. C154-C155.
doi: 10.1109/VLSIC.2015.7231247
URL: http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7231247&isnumber=7231231

Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso54114 board
- Personal Computer
- NXP Sensor Shield Board (Bosch Sensortec sensors: BMI055 inertial measurement unit BMC150 digital compass, BMM150 magnetometer and BMP280 pressure/temperature sensor)

Board settings
==============
Attach the NXP Sensor Shield Board to the LPCXpresso54114 board using the expansion connectors

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J7) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the Low Power demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CORE0 is running
Copy CORE1 image to address: 0x20010000, size: 12292
Starting CORE1
CORE1 is running, CORE0 is waiting for interrupt from CORE1.
Processing...
Average values:
temperature: 26.63 C, pressure: 96638 Pa
mag: x: 280, y: 90, z: 300, R: 6685
accel: x: 16, y: 116, z: 4296
gyro: x: 76, y: 393, z: 241

Processing...
Average values:
temperature: 26.73 C, pressure: 97039 Pa
mag: x: 274, y: 90, z: 300, R: 6685
accel: x: 16, y: 108, z: 4292
gyro: x: 63, y: 384, z: 249
.
.
.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.

Power consumption
The power consumption of this demo is on picture "power_consumption.png" in same folder as this readme.
The picture shows three cycles of reading sensor data, each cycle has five phases:
    1. the cores are in deep sleep mode (consumption is about 92uA)
    2. the cores are woken up by interrupt of uTick timer (consumption peak about 3mA)
    3. primary core is in sleep mode by WFI, flash memory is powered down and secondary core reads
       data from sensors (consumption about 1.23mA)
    4. secondary core power up flash memory, wake up primary core and switch to sleep by WFI
       (consumption peak about 2.5mA)
    5. secondary core is in sleep mode by WFI, primary core is active and it calls API to switch to deep
       sleep mode (consumption about 2mA)
