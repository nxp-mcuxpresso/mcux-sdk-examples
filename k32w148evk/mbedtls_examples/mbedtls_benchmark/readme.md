Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Personal Computer
- K32W148-EBB board

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mbedTLS version 2.28.8
fsys=96000000
Using following implementations:
  SHA: ELE200 HW accelerated
  AES: ELE200 HW accelerated ECB, CBC, CCM and CMAC
  AES GCM: Software implementation
  DES: Software implementation
  Asymmetric cryptography: ELE200 HW accelerated ECDSA and ECDH

  SHA-256                  :  6468.02 KiB/s,   14.13 cycles/byte
  SHA-512                  :  5957.50 KiB/s,   15.38 cycles/byte
  AES-CBC-128              :  2429.53 KiB/s,   38.35 cycles/byte
  AES-CBC-192              :  2414.24 KiB/s,   38.59 cycles/byte
  AES-CBC-256              :  2399.05 KiB/s,   38.84 cycles/byte
  AES-GCM-128              :   36.64 KiB/s,  2629.43 cycles/byte
  AES-GCM-192              :   36.42 KiB/s,  2644.97 cycles/byte
  AES-GCM-256              :   36.21 KiB/s,  2660.95 cycles/byte
  AES-CCM-128              :  1321.34 KiB/s,   70.70 cycles/byte
  AES-CCM-192              :  1316.70 KiB/s,   70.95 cycles/byte
  AES-CCM-256              :  1314.38 KiB/s,   71.08 cycles/byte
  AES-CMAC-128             :  1549.82 KiB/s,   60.13 cycles/byte
  AES-CMAC-192             :  1506.38 KiB/s,   61.88 cycles/byte
  AES-CMAC-256             :  1465.14 KiB/s,   63.64 cycles/byte
  AES-CMAC-PRF-128         :  1538.97 KiB/s,   60.48 cycles/byte
  CAMELLIA-CBC-128         :  635.14 KiB/s,  147.66 cycles/byte
  CAMELLIA-CBC-192         :  498.08 KiB/s,  188.42 cycles/byte
  CAMELLIA-CBC-256         :  498.08 KiB/s,  188.42 cycles/byte
  CTR_DRBG (NOPR)          :   37.41 KiB/s,  2573.51 cycles/byte
  CTR_DRBG (PR)            :   28.66 KiB/s,  3387.15 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :   51.58 KiB/s,  1852.80 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :   45.76 KiB/s,  2093.99 cycles/byte
  RSA-1024                 :    3.00 private/s
  RSA-1024                 :   84.00  public/s
  DHE-2048                 :    0.17 handshake/s
  DH-2048                  :    0.15 handshake/s
  ECDSA-secp521r1          :   24.00 sign/s
  ECDSA-secp384r1          :   47.00 sign/s
  ECDSA-secp256r1          :   82.33 sign/s
  ECDSA-secp521r1          :   12.67 verify/s
  ECDSA-secp384r1          :   30.00 verify/s
  ECDSA-secp256r1          :   71.67 verify/s
  ECDHE-secp521r1          :    0.75 full handshake/s
  ECDHE-secp384r1          :    1.00 full handshake/s
  ECDHE-secp256r1          :    2.00 full handshake/s
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

