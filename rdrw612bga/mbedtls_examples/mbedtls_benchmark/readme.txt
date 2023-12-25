Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

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


mbedTLS version 2.28.0
fsys=259200000
Using following implementations:
  SHA: ELS PKC HW accelerated
  AES: ELS PKC HW accelerated
  AES GCM: ELS PKC HW accelerated
  DES: Software implementation
  Asymmetric cryptography: ELS PKC HW accelerated

  MD5                      :  7917.40 KB/s,   31.57 cycles/byte
  SHA-1                    :  4342.17 KB/s,   57.90 cycles/byte
  SHA-256                  :  59819.24 KB/s,    3.83 cycles/byte
  SHA-512                  :  51890.32 KB/s,    4.47 cycles/byte
  3DES                     :  636.32 KB/s,  397.97 cycles/byte
  DES                      :  1598.20 KB/s,  158.07 cycles/byte
  3DES-CMAC                :  568.22 KB/s,  443.98 cycles/byte
  AES-CBC-128              :  55653.61 KB/s,    4.15 cycles/byte
  AES-CBC-192              :  52453.76 KB/s,    4.43 cycles/byte
  AES-CBC-256              :  47762.72 KB/s,    4.90 cycles/byte
  AES-XTS-128              :  2625.80 KB/s,   96.04 cycles/byte
  AES-XTS-256              :  2576.60 KB/s,   97.88 cycles/byte
  AES-GCM-128              :  19433.10 KB/s,   12.62 cycles/byte
  AES-GCM-192              :  18332.87 KB/s,   13.41 cycles/byte
  AES-GCM-256              :  17615.37 KB/s,   13.97 cycles/byte
  AES-CCM-128              :  14530.30 KB/s,   17.02 cycles/byte
  AES-CCM-192              :  13793.04 KB/s,   17.95 cycles/byte
  AES-CCM-256              :  13156.65 KB/s,   18.84 cycles/byte
  AES-CMAC-128             :  8843.90 KB/s,   28.22 cycles/byte
  AES-CMAC-192             :  3060.97 KB/s,   82.00 cycles/byte
  AES-CMAC-256             :  8286.22 KB/s,   30.15 cycles/byte
  AES-CMAC-PRF-128         :  8789.19 KB/s,   28.40 cycles/byte
  Poly1305                 :  8313.95 KB/s,   30.05 cycles/byte
  BLOWFISH-CBC-128         :  1563.72 KB/s,  161.57 cycles/byte
  BLOWFISH-CBC-192         :  1563.72 KB/s,  161.57 cycles/byte
  BLOWFISH-CBC-256         :  1563.73 KB/s,  161.57 cycles/byte
  CTR_DRBG (NOPR)          :  43424.98 KB/s,    5.42 cycles/byte
  HMAC_DRBG SHA-1 (NOPR)   :  319.68 KB/s,  793.36 cycles/byte
  HMAC_DRBG SHA-1 (PR)     :  296.99 KB/s,  854.77 cycles/byte
  HMAC_DRBG SHA-256 (NOPR) :  488.13 KB/s,  519.22 cycles/byte
  HMAC_DRBG SHA-256 (PR)   :  488.13 KB/s,  519.22 cycles/byte
  RSA-1024                 :  2339.67  public/s
  RSA-1024                 :  145.33 private/s
  ECDSA-secp521r1          :   24.33 sign/s
  ECDSA-brainpoolP512r1    :   29.33 sign/s
  ECDSA-secp384r1          :   56.67 sign/s
  ECDSA-brainpoolP384r1    :   57.67 sign/s
  ECDSA-secp256r1          :  132.00 sign/s
  ECDSA-secp256k1          :  132.00 sign/s
  ECDSA-brainpoolP256r1    :  134.33 sign/s
  ECDSA-secp224r1          :  150.00 sign/s
  ECDSA-secp224k1          :  150.00 sign/s
  ECDSA-secp192r1          :  222.33 sign/s
  ECDSA-secp192k1          :  222.67 sign/s
  ECDSA-secp521r1          :   12.00 verify/s
  ECDSA-brainpoolP512r1    :   18.33 verify/s
  ECDSA-secp384r1          :   29.00 verify/s
  ECDSA-brainpoolP384r1    :   29.33 verify/s
  ECDSA-secp256r1          :   86.67 verify/s
  ECDSA-secp256k1          :   86.33 verify/s
  ECDSA-brainpoolP256r1    :   87.67 verify/s
  ECDSA-secp224r1          :   81.33 verify/s
  ECDSA-secp224k1          :   80.67 verify/s
  ECDSA-secp192r1          :  124.00 verify/s
  ECDSA-secp192k1          :  124.00 verify/s
  ECDHE-secp521r1          :   12.67 handshake/s
  ECDHE-brainpoolP512r1    :   15.33 handshake/s
  ECDHE-secp384r1          :   29.33 handshake/s
  ECDHE-brainpoolP384r1    :   29.67 handshake/s
  ECDHE-secp256r1          :   68.33 handshake/s
  ECDHE-secp256k1          :   68.33 handshake/s
  ECDHE-brainpoolP256r1    :   69.33 handshake/s
  ECDHE-secp224r1          :   77.67 handshake/s
  ECDHE-secp224k1          :   77.67 handshake/s
  ECDHE-secp192r1          :  116.00 handshake/s
  ECDHE-secp192k1          :  116.00 handshake/s
  ECDH-secp521r1           :   24.67 handshake/s
  ECDH-brainpoolP512r1     :   30.00 handshake/s
  ECDH-secp384r1           :   58.00 handshake/s
  ECDH-brainpoolP384r1     :   59.00 handshake/s
  ECDH-secp256r1           :  136.00 handshake/s
  ECDH-secp256k1           :  136.00 handshake/s
  ECDH-brainpoolP256r1     :  138.33 handshake/s
  ECDH-secp224r1           :  155.00 handshake/s
  ECDH-secp224k1           :  155.00 handshake/s
  ECDH-secp192r1           :  231.33 handshake/s
  ECDH-secp192k1           :  231.33 handshake/s
  ECDHE-secp521r1          :    6.67 full handshake/s
  ECDHE-brainpoolP512r1    :    8.00 full handshake/s
  ECDHE-secp384r1          :   15.00 full handshake/s
  ECDHE-brainpoolP384r1    :   15.00 full handshake/s
  ECDHE-secp256r1          :   34.33 full handshake/s
  ECDHE-secp256k1          :   34.33 full handshake/s
  ECDHE-brainpoolP256r1    :   35.00 full handshake/s
  ECDHE-secp224r1          :   39.00 full handshake/s
  ECDHE-secp224k1          :   39.00 full handshake/s
  ECDHE-secp192r1          :   58.00 full handshake/s
  ECDHE-secp192k1          :   58.00 full handshake/s
    
