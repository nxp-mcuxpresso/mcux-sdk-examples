Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board. 
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
When the demo runs successfully, the terminal will display similar information like the following:


mbedTLS version 2.26.0
fsys=150000000
Using following implementations:
  SHA: CSS PKC HW accelerated
  AES: CSS PKC HW accelerated
  AES GCM: CSS PKC HW accelerated
  DES: Software implementation
  Asymmetric cryptography: CSS PKC HW accelerated

  MD5                      :  4662.18 KB/s,   31.33 cycles/byte
  RIPEMD160                :  875.40 KB/s,  166.87 cycles/byte
  SHA-1                    :  2350.95 KB/s,   62.11 cycles/byte
  SHA-256                  :  41391.32 KB/s,    3.45 cycles/byte
  SHA-512                  :  37291.83 KB/s,    3.83 cycles/byte
  ARC4                     :  7274.51 KB/s,   20.04 cycles/byte
  3DES                     :  380.14 KB/s,  386.25 cycles/byte
  DES                      :  995.42 KB/s,  147.21 cycles/byte
  3DES-CMAC                :  349.99 KB/s,  419.43 cycles/byte
  AES-CBC-128              :  32889.63 KB/s,    4.36 cycles/byte
  AES-CBC-192              :  29400.76 KB/s,    4.88 cycles/byte
  AES-CBC-256              :  26880.42 KB/s,    5.35 cycles/byte
  AES-XTS-128              :  1112.54 KB/s,  131.19 cycles/byte
  AES-XTS-256              :  1102.51 KB/s,  132.41 cycles/byte
  AES-GCM-128              :  9534.20 KB/s,   15.08 cycles/byte
  AES-GCM-192              :  9001.61 KB/s,   15.74 cycles/byte
  AES-GCM-256              :  8700.90 KB/s,   16.31 cycles/byte
  AES-CCM-128              :  4607.30 KB/s,   31.14 cycles/byte
  AES-CCM-256              :  4272.91 KB/s,   33.65 cycles/byte
  ChaCha20-Poly1305        :  932.59 KB/s,  156.78 cycles/byte
  AES-CMAC-128             :  4872.27 KB/s,   29.59 cycles/byte
  AES-CMAC-192             :  1098.19 KB/s,  133.17 cycles/byte
  AES-CMAC-256             :  4642.15 KB/s,   31.22 cycles/byte
  AES-CMAC-PRF-128         :  4820.65 KB/s,   29.90 cycles/byte
  CAMELLIA-CBC-128         :  1173.54 KB/s,  124.83 cycles/byte
  CAMELLIA-CBC-192         :  921.57 KB/s,  159.03 cycles/byte
  CAMELLIA-CBC-256         :  921.57 KB/s,  159.03 cycles/byte
  ChaCha20                 :  1222.75 KB/s,  119.80 cycles/byte
  Poly1305                 :  5063.30 KB/s,   28.84 cycles/byte
  BLOWFISH-CBC-128         :  1148.30 KB/s,  127.58 cycles/byte
  BLOWFISH-CBC-192         :  1148.30 KB/s,  127.58 cycles/byte
  BLOWFISH-CBC-256         :  1148.30 KB/s,  127.58 cycles/byte
  CTR_DRBG (NOPR)          :  23935.43 KB/s,    6.03 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :  108.35 KB/s,  1363.20 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :  100.99 KB/s,  1464.84 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :  312.69 KB/s,  469.76 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :  312.68 KB/s,  469.76 cycles/byte
  RSA-1024                 :  1754.00  public/s
  RSA-1024                 :  105.67 private/s
  DHE-2048                 :    0.33 handshake/s
  DH-2048                  :    0.33 handshake/s
  ECDSA-brainpoolP512r1    :   21.33 sign/s
  ECDSA-secp384r1          :   41.67 sign/s
  ECDSA-brainpoolP384r1    :   42.33 sign/s
  ECDSA-secp256r1          :   98.00 sign/s
  ECDSA-secp256k1          :   98.33 sign/s
  ECDSA-brainpoolP256r1    :  100.00 sign/s
  ECDSA-secp224r1          :  111.67 sign/s
  ECDSA-secp224k1          :  111.67 sign/s
  ECDSA-secp192r1          :  166.67 sign/s
  ECDSA-secp192k1          :  167.00 sign/s
  ECDSA-brainpoolP512r1    :   13.33 verify/s
  ECDSA-secp384r1          :   21.33 verify/s
  ECDSA-brainpoolP384r1    :   21.67 verify/s
  ECDSA-secp256r1          :   64.33 verify/s
  ECDSA-secp256k1          :   63.67 verify/s
  ECDSA-brainpoolP256r1    :   65.33 verify/s
  ECDSA-secp224r1          :   61.00 verify/s
  ECDSA-secp224k1          :   60.33 verify/s
  ECDSA-secp192r1          :   94.00 verify/s
  ECDSA-secp192k1          :   94.33 verify/s
  ECDHE-brainpoolP512r1    :   11.00 handshake/s
  ECDHE-secp384r1          :   21.33 handshake/s
  ECDHE-brainpoolP384r1    :   22.00 handshake/s
  ECDHE-secp256r1          :   51.00 handshake/s
  ECDHE-secp256k1          :   51.00 handshake/s
  ECDHE-brainpoolP256r1    :   51.67 handshake/s
  ECDHE-secp224r1          :   58.00 handshake/s
  ECDHE-secp224k1          :   58.00 handshake/s
  ECDHE-secp192r1          :   87.00 handshake/s
  ECDHE-secp192k1          :   87.00 handshake/s
  ECDH-brainpoolP512r1     :   21.67 handshake/s
  ECDH-secp384r1           :   42.33 handshake/s
  ECDH-brainpoolP384r1     :   43.33 handshake/s
  ECDH-secp256r1           :  101.00 handshake/s
  ECDH-secp256k1           :  101.33 handshake/s
  ECDH-brainpoolP256r1     :  103.00 handshake/s
  ECDH-secp224r1           :  115.33 handshake/s
  ECDH-secp224k1           :  115.33 handshake/s
  ECDH-secp192r1           :  173.33 handshake/s
  ECDH-secp192k1           :  173.33 handshake/s
