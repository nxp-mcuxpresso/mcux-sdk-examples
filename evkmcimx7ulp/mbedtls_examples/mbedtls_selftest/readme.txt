Overview
========
The mbedTLS SelfTest program performs cryptographic algorithm testing and prints results to the
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
The log below shows the output of the mbedtls_selftest example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mbedTLS version 2.13.1
fsys=115200000
Using following implementations:
  SHA: MMCAU HW accelerated
  AES: LTC HW accelerated
  AES GCM: MMCAU HW accelerated
  DES: MMCAU HW accelerated
  Asymmetric encryption: Software implementation

  MD5 test #1: passed
  MD5 test #2: passed
  MD5 test #3: passed
  MD5 test #4: passed
  MD5 test #5: passed
  MD5 test #6: passed
  MD5 test #7: passed

  SHA-1 test #1: passed
  SHA-1 test #2: passed
  SHA-1 test #3: passed

  SHA-224 test #1: passed
  SHA-224 test #2: passed
  SHA-224 test #3: passed
  SHA-256 test #1: passed
  SHA-256 test #2: passed
  SHA-256 test #3: passed

  SHA-384 test #1: passed
  SHA-384 test #2: passed
  SHA-384 test #3: passed
  SHA-512 test #1: passed
  SHA-512 test #2: passed
  SHA-512 test #3: passed

  DES -ECB- 56 (dec): passed
  DES -ECB- 56 (enc): passed
  DES3-ECB-112 (dec): passed
  DES3-ECB-112 (enc): passed
  DES3-ECB-168 (dec): passed
  DES3-ECB-168 (enc): passed

  DES -CBC- 56 (dec): passed
  DES -CBC- 56 (enc): passed
  DES3-CBC-112 (dec): passed
  DES3-CBC-112 (enc): passed
  DES3-CBC-168 (dec): passed
  DES3-CBC-168 (enc): passed

  AES-ECB-128 (dec): passed
  AES-ECB-128 (enc): passed
  AES-ECB-192 (dec): skipped
  AES-ECB-192 (enc): skipped
  AES-ECB-256 (dec): skipped
  AES-ECB-256 (enc): skipped

  AES-CBC-128 (dec): passed
  AES-CBC-128 (enc): passed
  AES-CBC-192 (dec): skipped
  AES-CBC-192 (enc): skipped
  AES-CBC-256 (dec): skipped
  AES-CBC-256 (enc): skipped

  AES-CFB128-128 (dec): passed
  AES-CFB128-128 (enc): passed
  AES-CFB128-192 (dec): skipped
  AES-CFB128-192 (enc): skipped
  AES-CFB128-256 (dec): skipped
  AES-CFB128-256 (enc): skipped

  AES-CTR-128 (dec): passed
  AES-CTR-128 (enc): passed
  AES-CTR-128 (dec): passed
  AES-CTR-128 (enc): passed
  AES-CTR-128 (dec): passed
  AES-CTR-128 (enc): passed

  AES-GCM-128 #0 (enc): passed
  AES-GCM-128 #0 (dec): passed
  AES-GCM-128 #0 split (enc): passed
  AES-GCM-128 #0 split (dec): passed
  AES-GCM-128 #1 (enc): passed
  AES-GCM-128 #1 (dec): passed
  AES-GCM-128 #1 split (enc): passed
  AES-GCM-128 #1 split (dec): passed
  AES-GCM-128 #2 (enc): passed
  AES-GCM-128 #2 (dec): passed
  AES-GCM-128 #2 split (enc): passed
  AES-GCM-128 #2 split (dec): passed
  AES-GCM-128 #3 (enc): passed
  AES-GCM-128 #3 (dec): passed
  AES-GCM-128 #3 split (enc): passed
  AES-GCM-128 #3 split (dec): passed
  AES-GCM-128 #4 (enc): passed
  AES-GCM-128 #4 (dec): passed
  AES-GCM-128 #4 split (enc): passed
  AES-GCM-128 #4 split (dec): passed
  AES-GCM-128 #5 (enc): passed
  AES-GCM-128 #5 (dec): passed
  AES-GCM-128 #5 split (enc): passed
  AES-GCM-128 #5 split (dec): passed

  CCM-AES #1: passed
  CCM-AES #2: passed
  CCM-AES #3: passed

  Base64 encoding test: passed
  Base64 decoding test: passed

  MPI test #1 (mul_mpi): passed
  MPI test #2 (div_mpi): passed
  MPI test #3 (exp_mod): passed
  MPI test #4 (inv_mod): passed
  MPI test #5 (simple gcd): passed

  RSA parse key #1       : passed
  RSA-1024 key validation: passed
  PKCS#1 encryption      : passed
  PKCS#1 decryption      : passed
  PKCS#1 data sign       : passed
  PKCS#1 sig. verify     : passed

  X.509 certificate load: passed
  X.509 signature verify: passed

  HMAC_DRBG (PR = True) : passed
  HMAC_DRBG (PR = False) : passed

  ECP test #1 (constant op_count, base point G): passed
  ECP test #2 (constant op_count, other point): passed

  DHM parameter load: passed

  ENTROPY test: passed

  PBKDF2 (SHA1) #0: passed
  PBKDF2 (SHA1) #1: passed
  PBKDF2 (SHA1) #2: passed
  PBKDF2 (SHA1) #3: passed
  PBKDF2 (SHA1) #4: passed
  PBKDF2 (SHA1) #5: passed

  Executed 17 test suites

                           [ All tests PASS ]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
