Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- TWR-KM35Z75M board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
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
These similar instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mbedTLS version 2.16.2
fsys=71991296
Using following implementations:
  SHA: MMCAU HW accelerated
  AES: MMCAU HW accelerated
  AES GCM: MMCAU HW accelerated
  DES: MMCAU HW accelerated
  Asymmetric encryption: Software implementation


  MD5                      :  3361.90 KB/s,   20.13 cycles/byte
  SHA-1                    :  1882.96 KB/s,   36.57 cycles/byte
  SHA-256                  :  1157.97 KB/s,   59.98 cycles/byte
  SHA-512                  :  212.11 KB/s,  332.23 cycles/byte
  3DES                     :  535.90 KB/s,  130.65 cycles/byte
  DES                      :  1105.68 KB/s,   62.86 cycles/byte
  AES-CBC-128              :  1197.26 KB/s,   57.98 cycles/byte
  AES-CBC-192              :  1133.40 KB/s,   61.29 cycles/byte
  AES-CBC-256              :  1090.59 KB/s,   63.73 cycles/byte
  AES-GCM-128              :  232.33 KB/s,  303.12 cycles/byte
  AES-GCM-192              :  229.61 KB/s,  306.74 cycles/byte
  AES-GCM-256              :  227.97 KB/s,  308.96 cycles/byte
  AES-CCM-128              :  438.25 KB/s,  160.00 cycles/byte
  AES-CCM-192              :  419.41 KB/s,  167.24 cycles/byte
  AES-CCM-256              :  408.63 KB/s,  171.69 cycles/byte
  CTR_DRBG (NOPR)          :  1246.48 KB/s,   55.66 cycles/byte
  CTR_DRBG (PR)            :  844.31 KB/s,   82.58 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :  130.93 KB/s,  540.31 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :  121.22 KB/s,  583.98 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :  116.25 KB/s,  609.21 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :  116.25 KB/s,  609.21 cycles/byte
  RSA-1024                 :   19.33  public/s
  RSA-1024                 :    1.00 private/s
  DHE-2048                 :    0.03 handshake/s
  DH-2048                  :    0.03 handshake/s
  ECDSA-secp256r1          :    1.67 sign/s
  ECDSA-secp256r1          :    1.00 verify/s
  ECDHE-secp256r1          :    1.00 handshake/s
  ECDH-secp256r1           :    1.67 handshake/s
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
