Overview
========
The example shows how to use i3c driver as master to read on board sensor ICM-42688P:

The example will do dynamic address assignment to the ICM-42688P sensor and then reset the sensor for configuration, after that re-do address assign because of address lost. Then enable the ACCEL, GYRO function on sensor. Configure the sensor to generate IBI for TAP detect, then the application continuously read FIFO to retrieve the ACCEL and GYRO data, in the process if you tap the sensor on board, the IBI will generate master handles the IBI, print the IBI data.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

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
I3C master read sensor data example.

I3C master do dynamic address assignment to the sensor slave.

I3C master dynamic address assignment done, sensor address: 0x 9

Sensor reset is done, re-assgin dynamic address

Sensor Data: ACCEL X 6, Y 28, Z 1935; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 0, Y 65529, Z 2112; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 2, Y 65526, Z 2129; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 4, Y 65535, Z 2086; GYRO X 32768, Y 32768, Z 32768.
Sensor Data: ACCEL X 3, Y 0, Z 2066; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 8, Y 65532, Z 2068; GYRO X 3, Y 65533, Z 2.
Sensor Data: ACCEL X 1, Y 65535, Z 2063; GYRO X 2, Y 65534, Z 2.
Sensor Data: ACCEL X 5, Y 2, Z 2068; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 6, Y 2, Z 2068; GYRO X 1, Y 65534, Z 1.
Sensor Data: ACCEL X 3, Y 65534, Z 2071; GYRO X 2, Y 65534, Z 1.
Sensor Data: ACCEL X 3, Y 65533, Z 2066; GYRO X 3, Y 65533, Z 0.
Sensor Data: ACCEL X 4, Y 0, Z 2067; GYRO X 3, Y 65535, Z 0.
Sensor Data: ACCEL X 5, Y 0, Z 2069; GYRO X 2, Y 65533, Z 2.
Sensor Data: ACCEL X 4, Y 1, Z 2067; GYRO X 2, Y 65532, Z 2.
Sensor Data: ACCEL X 2, Y 1, Z 2067; GYRO X 3, Y 65533, Z 1.
Sensor Data: ACCEL X 6, Y 65534, Z 2073; GYRO X 3, Y 65533, Z 1.
Sensor Data: ACCEL X 5, Y 65535, Z 2068; GYRO X 3, Y 65534, Z 1.
Sensor Data: ACCEL X 7, Y 0, Z 2072; GYRO X 3, Y 65533, Z 1.
Sensor Data: ACCEL X 4, Y 1, Z 2062; GYRO X 2, Y 65532, Z 1.
Sensor Data: ACCEL X 7, Y 0, Z 2071; GYRO X 2, Y 65534, Z 1.
Sensor Data: ACCEL X 2, Y 65532, Z 2070; GYRO X 1, Y 65533, Z 1.
Sensor Data: ACCEL X 5, Y 65532, Z 2072; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 4, Y 65533, Z 2071; GYRO X 1, Y 65533, Z 2.
Sensor Data: ACCEL X 2, Y 65532, Z 2074; GYRO X 2, Y 65533, Z 1.
Sensor Data: ACCEL X 5, Y 65529, Z 2075; GYRO X 1, Y 65534, Z 1.
Sensor Data: ACCEL X 5, Y 65533, Z 2072; GYRO X 2, Y 65532, Z 1.
Sensor Data: ACCEL X 65531, Y 65512, Z 2237; GYRO X 65526, Y 4, Z 1.
Sensor Data: ACCEL X 65534, Y 65532, Z 2124; GYRO X 54, Y 65509, Z 65534.
Sensor Data: ACCEL X 0, Y 65534, Z 2067; GYRO X 4, Y 65532, Z 1.
Sensor Data: ACCEL X 2, Y 65533, Z 2072; GYRO X 3, Y 65534, Z 1.
Sensor Data: ACCEL X 5, Y 65531, Z 2071; GYRO X 3, Y 65534, Z 1.
Sensor Data: ACCEL X 1, Y 65532, Z 2070; GYRO X 3, Y 65533, Z 1.
Received slave IBI request. Data 0x10.
Received slave IBI request. Data 0x10.
Received slave IBI request. Data 0x10.
Received slave IBI request. Data 0x10.
~~~~~~~~~~~~~~~~~~~~~
