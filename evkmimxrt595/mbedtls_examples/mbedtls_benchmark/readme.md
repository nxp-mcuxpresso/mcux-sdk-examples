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
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============
No special settings are required.
Prepare the Demo
================
Connect a serial cable from the debug UART port of the target board to the PC. Start TeraTerm (http://ttssh2.osdn.jp)
and make a connection to the virtual serial port.

1. Start TeraTerm.

2. New connection -> Serial.

3. Set the appropriate COMx port (where x is port number) in the port context menu. The number is provided by the operating
   system, and could be different from computer to computer. Select the COM number related to the virtual
   serial port. Confirm the selected port by clicking the "OK" button.

4. Set following connection parameters in menu Setup -> Serial port.
        Baud rate:    115200
        Data:         8
        Parity:       none
        Stop:         1
        Flow control: none

5.  Confirm selected parameters by clicking the "OK" button.

Running the demo
================
When the demo runs successfully, which takes a couple of minutes, the terminal displays similar information like the following:

mbedTLS version 2.26.0
fsys=250105263
Using following implementations:
  SHA: HASHCRYPT HW accelerated
  AES: HASHCRYPT HW accelerated
  AES GCM: Software implementation
  DES: Software implementation
  Asymmetric cryptography: CASPER HW accelerated ECC256/384/521 and RSA verify

  MD5                      :  13871.43 KB/s,   17.33 cycles/byte
  SHA-1                    :  95852.80 KB/s,    2.27 cycles/byte
  SHA-256                  :  104942.33 KB/s,    2.05 cycles/byte
  SHA-512                  :  2016.78 KB/s,  120.89 cycles/byte
  3DES                     :  871.28 KB/s,  280.37 cycles/byte
  DES                      :  2118.32 KB/s,  115.08 cycles/byte
  AES-CBC-128              :  77563.50 KB/s,    2.87 cycles/byte
  AES-CBC-192              :  66632.58 KB/s,    3.39 cycles/byte
  AES-CBC-256              :  58500.90 KB/s,    3.90 cycles/byte
  AES-GCM-128              :  2096.09 KB/s,  116.30 cycles/byte
  AES-GCM-192              :  2066.84 KB/s,  117.95 cycles/byte
  AES-GCM-256              :  2042.72 KB/s,  119.35 cycles/byte
  AES-CCM-128              :  3942.24 KB/s,   61.69 cycles/byte
  AES-CCM-192              :  3742.94 KB/s,   64.99 cycles/byte
  AES-CCM-256              :  3589.41 KB/s,   67.79 cycles/byte
  Poly1305                 :  23583.22 KB/s,   10.08 cycles/byte
  CTR_DRBG (NOPR)          :  10029.10 KB/s,   24.08 cycles/byte
  CTR_DRBG (PR)            :  6888.67 KB/s,   35.18 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :  2396.60 KB/s,  101.67 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :  2191.93 KB/s,  111.20 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :  3152.89 KB/s,   77.21 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :  3152.90 KB/s,   77.21 cycles/byte
  RSA-1024                 :  1636.33  public/s
  RSA-1024                 :   13.00 private/s
  DHE-2048                 :    0.60 handshake/s
  DH-2048                  :    1.00 handshake/s
  ECDSA-secp521r1          :   11.33 sign/s
  ECDSA-secp384r1          :   17.67 sign/s
  ECDSA-secp256r1          :   37.67 sign/s
  ECDSA-secp521r1          :    8.33 verify/s
  ECDSA-secp384r1          :   16.67 verify/s
  ECDSA-secp256r1          :   35.67 verify/s
  ECDHE-secp521r1          :    6.67 handshake/s
  ECDHE-secp384r1          :   10.33 handshake/s
  ECDHE-secp256r1          :   22.00 handshake/s
  ECDH-secp521r1           :   12.67 handshake/s
  ECDH-secp384r1           :   20.00 handshake/s
  ECDH-secp256r1           :   43.33 handshake/s
  ECDHE-secp521r1          :    3.67 full handshake/s
  ECDHE-secp384r1          :    5.33 full handshake/s
  ECDHE-secp256r1          :   11.33 full handshake/s
