Overview
========
The example shows how to use i3c_bus component and create bus master to read on board sensor ICM-42688P:

The example will create i3c bus firstly, then create bus master which will do dynamic address assignment to the ICM-42688P sensor. Then reset the sensor for configuration, after that re-assgin sensor address via SETDASA CCC command because address is lost after reset. Then enable the ACCEL, GYRO function on sensor. Configure the sensor to generate IBI for TAP detect, then the application continuously read FIFO to retrieve the ACCEL and GYRO data, in the process if you tap the sensor on board, the IBI will generate master handles the IBI, print the IBI data.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
The log below shows the output of the example in the terminal window. You could tap the sensor on board(U7) and see sensor read stops, each tap will generate a new IBI data print log "Received slave IBI request.". 
~~~~~~~~~~~~~~~~~~~~~
I3C bus master read sensor example.

I3C bus master creates.

Sensor Data: ACCEL X 3, Y 29, Z 1933; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 13, Y 5, Z 2112; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 16, Y 2, Z 2133; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 10, Y 4, Z 2088; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 12, Y 2, Z 2071; GYRO X 2, Y 65532, Z 0.
Sensor Data: ACCEL X 9, Y 1, Z 2068; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 11, Y 1, Z 2072; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 13, Y 65535, Z 2072; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 11, Y 0, Z 2067; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 8, Y 0, Z 2070; GYRO X 1, Y 65534, Z 0.
Sensor Data: ACCEL X 10, Y 2, Z 2068; GYRO X 0, Y 65532, Z 2.
Sensor Data: ACCEL X 11, Y 0, Z 2071; GYRO X 3, Y 65533, Z 1.
Sensor Data: ACCEL X 9, Y 65534, Z 2071; GYRO X 3, Y 65533, Z 0.
Sensor Data: ACCEL X 10, Y 65535, Z 2070; GYRO X 2, Y 65534, Z 2.
Sensor Data: ACCEL X 9, Y 0, Z 2069; GYRO X 2, Y 65533, Z 0.
Sensor Data: ACCEL X 10, Y 65535, Z 2066; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 13, Y 65535, Z 2068; GYRO X 1, Y 65533, Z 2.
Sensor Data: ACCEL X 10, Y 65533, Z 2067; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 8, Y 65534, Z 2071; GYRO X 1, Y 65534, Z 1.
Sensor Data: ACCEL X 6, Y 2, Z 2072; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 10, Y 65531, Z 2072; GYRO X 3, Y 65532, Z 2.
Sensor Data: ACCEL X 11, Y 0, Z 2069; GYRO X 3, Y 65533, Z 1.
Sensor Data: ACCEL X 12, Y 0, Z 2071; GYRO X 1, Y 65532, Z 1.
Sensor Data: ACCEL X 10, Y 65535, Z 2069; GYRO X 2, Y 65532, Z 2.
Sensor Data: ACCEL X 11, Y 0, Z 2074; GYRO X 2, Y 65534, Z 1.
Received slave IBI request. Data 0x10.
Received slave IBI request. Data 0x10.
Received slave IBI request. Data 0x10.
Received slave IBI request. Data 0x10.
~~~~~~~~~~~~~~~~~~~~~
