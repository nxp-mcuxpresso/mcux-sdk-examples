Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####

#### Please note that this application must be built with flash link file! Build with RAM link
file might fail due to shortage of RAM size. ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the mbedtls_benchmark example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mbedTLS version 2.13.1
fsys=115200000
Using following implementations:
  SHA: MMCAU HW accelerated
  AES: LTC HW accelerated
  AES GCM: MMCAU HW accelerated
  DES: MMCAU HW accelerated
  Asymmetric encryption: Software implementation


  MD5                      :  4367.04 KB/s,   15.83 cycles/byte
  SHA-1                    :  1308.00 KB/s,   77.30 cycles/byte
  SHA-256                  :  2086.48 KB/s,   48.72 cycles/byte
  SHA-512                  :  324.32 KB/s,  294.62 cycles/byte
  3DES                     :  997.42 KB/s,  112.46 cycles/byte
  DES                      :  2124.67 KB/s,   52.57 cycles/byte
  AES-CBC-128              :  5086.13 KB/s,   21.72 cycles/byte
  AES-GCM-128              :  423.85 KB/s,  252.39 cycles/byte
  AES-CCM-128              :  2539.45 KB/s,   43.89 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :   24.68 KB/s,  4716.86 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :   22.74 KB/s,  5122.51 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :   57.94 KB/s,  2031.82 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :   49.93 KB/s,  2290.57 cycles/byte
  RSA-1024                 :   81.67  public/s
  RSA-1024                 :    2.33 private/s
  DHE-2048                 :    0.18 handshake/s
  DH-2048                  :    0.17 handshake/s
  ECDSA-secp256r1          :    0.75 sign/s
  ECDSA-secp256r1          :    0.50 verify/s
  ECDHE-secp256r1          :    0.50 handshake/s
  ECDH-secp256r1           :    0.75 handshake/s
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
