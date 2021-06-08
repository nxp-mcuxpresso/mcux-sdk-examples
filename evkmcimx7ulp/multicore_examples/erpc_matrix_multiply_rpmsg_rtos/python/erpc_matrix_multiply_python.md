Overview
========
The Multicore eRPC Matrix Multiply RTOS project is a simple demonstration program that uses the KSDK
software and the Multicore SDK to show how to implement the Remote Procedure Call between cores of
the multicore system. The primary core (eRPC client) is a python application running in Linux user
space and the secondary core (eRPC server) is realized by a remote core communicating with Linux 
using RPMsg. The erpcMatrixMultiply() eRPC call is issued to let the secondary core to perform
the multiplication of two randomly generated matrices. The original matrices and the result matrix
is printed out to the Linux console. The matrix multiplication can be issued repeatedly when 
pressing any key when the python script is running.

Requirements
============
- You need to have kernel module imx_rpmsg_tty.ko inserted in your Linux Kernel. You can verify this
  be `ls /dev | grep RPMSG`, you should see /dev/ttyRPMSG.
- You need python (2 or 3) installed in your Linux system.
- You need Enum34 and PySerial python packages installed.
- You need erpc python package installed, you can find this in KSDK.
- Run by typing: `python example.py` 
