Overview
========
Memory-Mapped Cryptographic Acceleration Unit (MMCAU)

This project is a demonstration program that uses the KSDK software for encryption/decryption sample
data using AES-CBC, DES3-CBC and Hash algorithms MD5, SHA1 and SHA256.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- TWR-KM35Z75M board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Running the demo
================
When the demo runs successfully, from the terminal you can see:
............................. MMCAU  DRIVER  EXAMPLE .............................

Testing input string:
          Once upon a midnight dreary,
           while I pondered weak and weary,
          Over many a quaint and curious volume of forgotten lore,
          While I nodded,
           nearly napping,
           suddenly there came a tapping,
          As of some one gently rapping,
           rapping at my chamber doorIts some visitor,
           I muttered,
           tapping at my chamber doorOnly this,
           and nothing more.

----------------------------------- AES-128-CBC method --------------------------------------
AES-128 CBC Encryption of 320 bytes.
AES-128 CBC encryption finished. Speed 0.952322 MB/s.

AES-128 CBC Decryption of 320 bytes.
AES-128 CBC decryption finished. Speed 0.953323 MB/s.
Decrypted string :
          Once upon a midnight dreary,
           while I pondered weak and weary,
          Over many a quaint and curious volume of forgotten lore,
          While I nodded,
           nearly napping,
           suddenly there came a tapping,
          As of some one gently rapping,
           rapping at my chamber doorIts some visitor,
           I muttered,
           tapping at my chamber doorOnly this,
           and nothing more.

AES-128 CBC encryption & decryption finished. Sucess rate 100.000000
----------------------------------- AES-192-CBC method --------------------------------------
AES-192 CBC Encryption of 320 bytes.
AES-192 CBC encryption finished. Speed 0.907594 MB/s.

AES-192 CBC Decryption of 320 bytes.
AES-192 CBC decryption finished. Speed 0.907778 MB/s.
Decrypted string :
          Once upon a midnight dreary,
           while I pondered weak and weary,
          Over many a quaint and curious volume of forgotten lore,
          While I nodded,
           nearly napping,
           suddenly there came a tapping,
          As of some one gently rapping,
           rapping at my chamber doorIts some visitor,
           I muttered,
           tapping at my chamber doorOnly this,
           and nothing more.

AES-192 CBC encryption & decryption finished. Sucess rate 100.000000
----------------------------------- AES-256-CBC method --------------------------------------
AES-256 CBC Encryption of 320 bytes.
AES-256 CBC encryption finished. Speed 0.867772 MB/s.

AES-256 CBC Decryption of 320 bytes.
AES-256 CBC decryption finished. Speed 0.868367 MB/s.
Decrypted string :
          Once upon a midnight dreary,
           while I pondered weak and weary,
          Over many a quaint and curious volume of forgotten lore,
          While I nodded,
           nearly napping,
           suddenly there came a tapping,
          As of some one gently rapping,
           rapping at my chamber doorIts some visitor,
           I muttered,
           tapping at my chamber doorOnly this,
           and nothing more.

AES-256 CBC encryption & decryption finished. Sucess rate 100.000000

----------------------------------- DES3-CBC method --------------------------------------
DES3 CBC Encryption of 320 bytes.
DES3 CBC encryption finished. Speed 0.445560 MB/s.

DES3 CBC decryption of 320 bytes.
DES3 CBC decryption finished. Speed 0.448773 MB/s.
Decrypted string :
          Once upon a midnight dreary,
           while I pondered weak and weary,
          Over many a quaint and curious volume of forgotten lore,
          While I nodded,
           nearly napping,
           suddenly there came a tapping,
          As of some one gently rapping,
           rapping at my chamber doorIts some visitor,
           I muttered,
           tapping at my chamber doorOnly this,
           and nothing more.

DES3 encryption & decryption finished. Sucess rate 100.000000

--------------------------------------- HASH ------------------------------------------
Computing hash of 64 bytes.
Input string:
          The quick brown fox jumps over the lazy dog

Computed SHA1 at speed 1.779654 MB/s:
2fd4e1c67a2d28fced849ee1bb76e7391b93eb12

SHA1 finished. Sucess rate 100.000000

Computed SHA256 at speed 1.081208 MB/s:
d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592

SHA256 finished. Sucess rate 100.000000

Computed MD5 at speed 3.271752 MB/s:
9e107d9d372bb6826bd81d3542a419d6MD5 finished. Sucess rate 100.000000



.............. THE  END  OF  THE  MMCAU  DRIVER  EXAMPLE ................................
