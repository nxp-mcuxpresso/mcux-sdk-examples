Overview
========
Memory-Mapped Cryptographic Acceleration Unit (MMCAU)

This project is a demonstration program that uses the KSDK software for encryption/decryption sample
data using AES-CBC, DES3-CBC and Hash algorithms MD5, SHA1 and SHA256.


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
AES-128 CBC encryption finished. Speed 1.412340 MB/s.

AES-128 CBC Decryption of 320 bytes.
AES-128 CBC decryption finished. Speed 1.451456 MB/s.
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
AES-192 CBC encryption finished. Speed 1.308963 MB/s.

AES-192 CBC Decryption of 320 bytes.
AES-192 CBC decryption finished. Speed 1.344955 MB/s.
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
AES-256 CBC encryption finished. Speed 1.227198 MB/s.

AES-256 CBC Decryption of 320 bytes.
AES-256 CBC decryption finished. Speed 1.260218 MB/s.
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
DES3 CBC encryption finished. Speed 0.828560 MB/s.

DES3 CBC decryption of 320 bytes.
DES3 CBC decryption finished. Speed 0.904796 MB/s.
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
Computing hash of 128 bytes.
Input string:
          Once upon a midnight dreary,
           while I pondered,
           weak and weary,
           Over many a quaint and curious volume of forgotten lore.

Computed SHA1 at speed 2.072770 MB/s:
29479755d6408ea49cb74c79949f6c303e1f4549

SHA1 finished. Sucess rate 100.000000

Computed SHA256 at speed 1.227499 MB/s:
15ae3b1120b98cfb516bafe14ba2d6a2fcada8727584111c95f355e1033bf89d

SHA256 finished. Sucess rate 100.000000

Computed MD5 at speed 3.489139 MB/s:
9e107d9d372bb6826bd81d3542a419d6MD5 finished. Sucess rate 100.000000



.............. THE  END  OF  THE  MMCAU  DRIVER  EXAMPLE ................................


