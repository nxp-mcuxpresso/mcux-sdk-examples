Overview
========
The ELS PKC application provides examples which demonstrate usage of most available algorithms and a crypto library lightweight testing:
- AES-128/192/256 encryption/decryption, modes ECB/CBC/CTR/GCM/CMAC
- SHA2-224/256/384/512 (including sha-direct mode)
- KDF: CKDF (NIST SP 800-108 / SP800-56C), HKDF (RFC5869)
- HMAC
- ECC Sign/Verify/Keygen/Keyexchange (P-256)
- RFC3394 key wrapping/unwrapping
- Key Import/Export/Utilities/Provisioning
- Random number Generation (DRBG/PRNG)

Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
When the demo runs successfully, the terminal will display similar information like the following:

ELS hash example

SHA224 one shot:pass
SHA256 one shot:pass
SHA256 streaming example:pass
SHA256 long message example:pass
SHA384 one shot:pass
SHA512 one shot:pass

RESULT: All 6 test PASS!!
ELS example END

