Overview
========
The USART example for FreeRTOS demonstrates the possibility to use the USART driver in the RTOS.
The example uses single instance of USART IP and writes string into, then reads back chars.
After every 4B received, these are sent back on USART.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018M board
- Personal Computer

Board settings
==============
To make usart_freertos example work, keep default configuration.
