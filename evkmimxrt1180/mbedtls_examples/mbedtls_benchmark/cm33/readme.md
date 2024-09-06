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
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
For MCUX IDE, erase the flash with SW5=0000, as noted in the Getting started guide, then set SW5=0100.

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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mbedTLS version 2.28.5
fsys=240000000
Using following implementations:
  SHA: Software implementation
  AES: ELE S40x HW accelerated
  AES GCM: Software implementation
  DES: Software implementation
  Asymmetric cryptography: ELE S40x HW accelerated RSA

  MD5                      :  474.13 KiB/s,  487.46 cycles/byte
  SHA-1                    :  607.04 KiB/s,  379.35 cycles/byte
  SHA-256                  :  1819.17 KiB/s,  121.53 cycles/byte
  SHA-512                  :  1667.46 KiB/s,  133.48 cycles/byte
  AES-CBC-128              :  2662.44 KiB/s,   80.72 cycles/byte
  AES-CBC-192              :  2654.76 KiB/s,   80.97 cycles/byte
  AES-CBC-256              :  2647.99 KiB/s,   81.19 cycles/byte
  AES-XTS-128              :   79.08 KiB/s,  2993.83 cycles/byte
  AES-XTS-256              :   78.51 KiB/s,  3016.67 cycles/byte
  AES-GCM-128              :   48.20 KiB/s,  4955.88 cycles/byte
  AES-GCM-192              :   48.09 KiB/s,  4967.43 cycles/byte
  AES-GCM-256              :   47.97 KiB/s,  4982.34 cycles/byte
  AES-CCM-128              :  3219.24 KiB/s,   65.46 cycles/byte
  AES-CCM-192              :  2899.25 KiB/s,   73.51 cycles/byte
  AES-CCM-256              :  2637.28 KiB/s,   81.51 cycles/byte
  AES-CMAC-128             :   78.23 KiB/s,  3026.06 cycles/byte
  AES-CMAC-192             :   77.92 KiB/s,  3037.69 cycles/byte
  AES-CMAC-256             :   77.57 KiB/s,  3052.74 cycles/byte
  AES-CMAC-PRF-128         :   78.13 KiB/s,  3030.14 cycles/byte
  Poly1305                 :  481.49 KiB/s,  480.18 cycles/byte
  BLOWFISH-CBC-128         :  187.46 KiB/s,  1249.64 cycles/byte
  BLOWFISH-CBC-192         :  187.46 KiB/s,  1249.63 cycles/byte
  BLOWFISH-CBC-256         :  187.46 KiB/s,  1249.66 cycles/byte
  CTR_DRBG (NOPR)          :   82.48 KiB/s,  2865.79 cycles/byte
  CTR_DRBG (PR)            :   62.09 KiB/s,  3825.00 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :   39.88 KiB/s,  6019.47 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :   36.99 KiB/s,  6503.91 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :   20.72 KiB/s,  11865.59 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :   20.72 KiB/s,  11865.89 cycles/byte
  RSA-2048                 :    7.33 private/s
  RSA-2048                 :  419.33  public/s
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
