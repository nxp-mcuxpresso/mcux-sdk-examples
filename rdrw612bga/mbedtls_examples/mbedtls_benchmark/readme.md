Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- RD-RW61X-BGA board
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

mbedTLS version 2.28.5
fsys=260000000
Using following implementations:
  SHA: ELS HW accelerated SHA256 and SHA512
  AES: ELS HW accelerated
  AES GCM: ELS HW accelerated
  DES: Software implementation
  Asymmetric cryptography: PKC HW accelerated

  MD5                      :  10934.86 KiB/s,   22.96 cycles/byte
  SHA-1                    :  7257.16 KiB/s,   34.73 cycles/byte
  SHA-256                  :  77377.23 KiB/s,    3.02 cycles/byte
  SHA-512                  :  74624.56 KiB/s,    3.14 cycles/byte
  AES-CBC-128              :  60881.53 KiB/s,    3.91 cycles/byte
  AES-CBC-192              :  56899.14 KiB/s,    4.19 cycles/byte
  AES-CBC-256              :  51546.40 KiB/s,    4.66 cycles/byte
  AES-XTS-128              :  3334.33 KiB/s,   75.90 cycles/byte
  AES-XTS-256              :  3247.79 KiB/s,   77.93 cycles/byte
  AES-GCM-128              :  19704.68 KiB/s,   12.62 cycles/byte
  AES-GCM-192              :  18570.88 KiB/s,   13.40 cycles/byte
  AES-GCM-256              :  17850.00 KiB/s,   13.96 cycles/byte
  AES-CCM-128              :  12812.48 KiB/s,   19.56 cycles/byte
  AES-CCM-192              :  12236.48 KiB/s,   20.49 cycles/byte
  AES-CCM-256              :  11733.94 KiB/s,   21.38 cycles/byte
  AES-CMAC-128             :  9044.24 KiB/s,   27.81 cycles/byte
  AES-CMAC-192             :  3566.02 KiB/s,   70.95 cycles/byte
  AES-CMAC-256             :  8537.01 KiB/s,   29.48 cycles/byte
  AES-CMAC-PRF-128         :  8977.26 KiB/s,   28.02 cycles/byte
  Poly1305                 :  27043.54 KiB/s,    9.12 cycles/byte
  BLOWFISH-CBC-128         :  2891.72 KiB/s,   87.57 cycles/byte
  BLOWFISH-CBC-192         :  2891.72 KiB/s,   87.57 cycles/byte
  BLOWFISH-CBC-256         :  2891.72 KiB/s,   87.57 cycles/byte
  CTR_DRBG (NOPR)          :  45182.42 KiB/s,    5.36 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :  520.15 KiB/s,  488.82 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :  479.52 KiB/s,  530.35 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :  820.81 KiB/s,  309.45 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :  820.81 KiB/s,  309.46 cycles/byte
  RSA-1024                 :  134.67 private/s
  RSA-1024                 :  2262.00  public/s
  ECDSA-secp521r1          :    9.33 sign/s
  ECDSA-brainpoolP512r1    :   10.00 sign/s
  ECDSA-secp384r1          :   10.33 sign/s
  ECDSA-brainpoolP384r1    :   10.33 sign/s
  ECDSA-secp256r1          :  122.67 sign/s
  ECDSA-secp256k1          :  122.67 sign/s
  ECDSA-brainpoolP256r1    :  126.33 sign/s
  ECDSA-secp224r1          :  140.67 sign/s
  ECDSA-secp224k1          :  140.67 sign/s
  ECDSA-secp192r1          :  208.00 sign/s
  ECDSA-secp192k1          :  208.33 sign/s
  ECDSA-secp521r1          :   11.67 verify/s
  ECDSA-brainpoolP512r1    :   14.33 verify/s
  ECDSA-secp384r1          :   28.33 verify/s
  ECDSA-brainpoolP384r1    :   28.67 verify/s
  ECDSA-secp256r1          :   69.33 verify/s
  ECDSA-secp256k1          :   69.67 verify/s
  ECDSA-brainpoolP256r1    :   71.00 verify/s
  ECDSA-secp224r1          :   80.33 verify/s
  ECDSA-secp224k1          :   79.67 verify/s
  ECDSA-secp192r1          :  123.33 verify/s
  ECDSA-secp192k1          :  122.33 verify/s
  ECDHE-secp521r1          :   12.00 handshake/s
  ECDHE-brainpoolP512r1    :   14.33 handshake/s
  ECDHE-secp384r1          :   27.33 handshake/s
  ECDHE-brainpoolP384r1    :   28.00 handshake/s
  ECDHE-secp256r1          :   63.00 handshake/s
  ECDHE-secp256k1          :   63.00 handshake/s
  ECDHE-brainpoolP256r1    :   65.00 handshake/s
  ECDHE-secp224r1          :   72.33 handshake/s
  ECDHE-secp224k1          :   72.33 handshake/s
  ECDHE-secp192r1          :  107.33 handshake/s
  ECDHE-secp192k1          :  107.33 handshake/s
  ECDH-secp521r1           :   23.33 handshake/s
  ECDH-brainpoolP512r1     :   28.33 handshake/s
  ECDH-secp384r1           :   54.00 handshake/s
  ECDH-brainpoolP384r1     :   55.33 handshake/s
  ECDH-secp256r1           :  125.67 handshake/s
  ECDH-secp256k1           :  125.67 handshake/s
  ECDH-brainpoolP256r1     :  129.33 handshake/s
  ECDH-secp224r1           :  144.33 handshake/s
  ECDH-secp224k1           :  144.33 handshake/s
  ECDH-secp192r1           :  214.33 handshake/s
  ECDH-secp192k1           :  214.33 handshake/s
  ECDHE-secp521r1          :    6.33 full handshake/s
  ECDHE-brainpoolP512r1    :    7.33 full handshake/s
  ECDHE-secp384r1          :   13.67 full handshake/s
  ECDHE-brainpoolP384r1    :   14.33 full handshake/s
  ECDHE-secp256r1          :   31.67 full handshake/s
  ECDHE-secp256k1          :   31.67 full handshake/s
  ECDHE-brainpoolP256r1    :   32.67 full handshake/s
  ECDHE-secp224r1          :   36.33 full handshake/s
  ECDHE-secp224k1          :   36.33 full handshake/s
  ECDHE-secp192r1          :   53.67 full handshake/s
  ECDHE-secp192k1          :   54.00 full handshake/s    
