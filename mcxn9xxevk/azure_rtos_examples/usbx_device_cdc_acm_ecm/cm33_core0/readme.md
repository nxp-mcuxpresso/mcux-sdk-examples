Overview
========
This example works as a USB composite device with CDC-ACM and CDC-ECM. When connecting to a Linux PC,
it will appear as one USB CDC-ACM device (for example, /dev/ttyACM0) and one USB CDC-ECM device.

Linux supports the USB CDC-ECM device by the cdc_ether driver but Window can not. The example enables
TCP/IP stack. It means the program running on the target device can communicate with PC through TCP/IP.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- Target Board
- Personal Computer(Linux PC)

Board settings
==============
This example only supports the high-speed USB port, J27.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
3.  Write the program to the flash of the target board.
4.  Press the reset button on your board to start the demo.
5.  Connect a USB cable between the Linux PC and the on-board high-speed USB port, J27.
6.  The Linux PC can detect one USB CDC-ACM device and one USB CDC-ECM device.

Running the demo
================

When the demo is running, the serial port will output:

Start USBX device ACM and ECM example...
USB CDC-ACM device is activated.
USB CDC-ECM device is activated.


Use the dmesg command to check on the Linux PC. If everything is going well,
the following message will appear.

[1201781.223713] usb 3-10: new high-speed USB device number 43 using xhci_hcd
[1201781.372216] usb 3-10: New USB device found, idVendor=1fc9, idProduct=00b3
[1201781.372221] usb 3-10: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[1201781.372225] usb 3-10: Product: USB CDCECM Device
[1201781.372228] usb 3-10: Manufacturer: NXP SEMICONDUCTORS
[1201781.372230] usb 3-10: SerialNumber: 0001
[1201781.376188] cdc_acm 3-10:1.0: ttyACM0: USB ACM device
[1201781.380675] cdc_ether 3-10:1.2 eth1: register 'cdc_ether' at usb-0000:00:14.0-10, CDC Ethernet Device, 00:11:22:33:44:56
[1201781.402376] cdc_ether 3-10:1.2 enx001122334456: renamed from eth1


From the above information, the USB ACM device is /dev/ttyACM0 and the USB ECM device is
the Ethernet interface enx001122334456.

The USB ACM device, /dev/ttyACM0,  will output "OK" every second.

OK
OK
OK


For the Ethernet interface (enx001122334456) corresponding to the USB ECM device,
setup it using the following commands.

$ sudo ifconfig  enx001122334456 192.168.18.2

The example running on the target device had been set as the IP address, 192.168.18.1.
If the following ping command succeeds, it means the USB ECM device works as expected.

$ ping 192.168.18.1
PING 192.168.18.1 (192.168.18.1) 56(84) bytes of data.
64 bytes from 192.168.18.1: icmp_seq=1 ttl=128 time=0.170 ms
64 bytes from 192.168.18.1: icmp_seq=2 ttl=128 time=0.163 ms
64 bytes from 192.168.18.1: icmp_seq=3 ttl=128 time=0.213 ms


Note that if the Linux PC you are using enables NetworkManager to manage the Ethernet interface.
It may change the static IP address of the Ethernet interface. To avoid this from happening,
set the Ethernet interface as unmanaged before setting the static IP address. Use the command, nmcli:

$ nmcli device set enx001122334456 managed no


The USB descriptor can be checked by the command on PC:

$ lsusb -v -d 1fc9:00b3
Bus 003 Device 043: ID 1fc9:00b3 NXP Semiconductors 
Device Descriptor:
  bLength                18
  bDescriptorType         1
  bcdUSB               2.00
  bDeviceClass          239 Miscellaneous Device
  bDeviceSubClass         2 ?
  bDeviceProtocol         1 Interface Association
  bMaxPacketSize0        64
  idVendor           0x1fc9 NXP Semiconductors
  idProduct          0x00b3 
  bcdDevice            1.00
  iManufacturer           1 NXP SEMICONDUCTORS
  iProduct                2 USB CDCECM Device
  iSerial                 3 0001
  bNumConfigurations      1
  Configuration Descriptor:
    bLength                 9
    bDescriptorType         2
    wTotalLength          154
    bNumInterfaces          4
    bConfigurationValue     1
    iConfiguration          0 
    bmAttributes         0xa0
      (Bus Powered)
      Remote Wakeup
    MaxPower              100mA
    Interface Association:
      bLength                 8
      bDescriptorType        11
      bFirstInterface         0
      bInterfaceCount         2
      bFunctionClass          2 Communications
      bFunctionSubClass       2 Abstract (modem)
      bFunctionProtocol       0 None
      iFunction               0 
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        0
      bAlternateSetting       0
      bNumEndpoints           1
      bInterfaceClass         2 Communications
      bInterfaceSubClass      2 Abstract (modem)
      bInterfaceProtocol      1 AT-commands (v.25ter)
      iInterface              0 
      CDC Header:
        bcdCDC               1.10
      CDC ACM:
        bmCapabilities       0x02
          line coding and serial state
      CDC Union:
        bMasterInterface        0
        bSlaveInterface         1 
      CDC Call Management:
        bmCapabilities       0x00
        bDataInterface          1
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x81  EP 1 IN
        bmAttributes            3
          Transfer Type            Interrupt
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0008  1x 8 bytes
        bInterval               8
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        1
      bAlternateSetting       0
      bNumEndpoints           2
      bInterfaceClass        10 CDC Data
      bInterfaceSubClass      0 Unused
      bInterfaceProtocol      0 
      iInterface              0 
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x82  EP 2 IN
        bmAttributes            2
          Transfer Type            Bulk
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0200  1x 512 bytes
        bInterval               0
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x02  EP 2 OUT
        bmAttributes            2
          Transfer Type            Bulk
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0200  1x 512 bytes
        bInterval               0
    Interface Association:
      bLength                 8
      bDescriptorType        11
      bFirstInterface         2
      bInterfaceCount         2
      bFunctionClass          2 Communications
      bFunctionSubClass       0 
      bFunctionProtocol       0 
      iFunction               0 
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        2
      bAlternateSetting       0
      bNumEndpoints           1
      bInterfaceClass         2 Communications
      bInterfaceSubClass      6 Ethernet Networking
      bInterfaceProtocol      0 
      iInterface              0 
      CDC Header:
        bcdCDC               1.10
      CDC Union:
        bMasterInterface        2
        bSlaveInterface         3 
      CDC Ethernet:
        iMacAddress                      4 001122334456
        bmEthernetStatistics    0x00000000
        wMaxSegmentSize               1514
        wNumberMCFilters            0x0000
        bNumberPowerFilters              0
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x83  EP 3 IN
        bmAttributes            3
          Transfer Type            Interrupt
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0008  1x 8 bytes
        bInterval               8
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        3
      bAlternateSetting       0
      bNumEndpoints           0
      bInterfaceClass        10 CDC Data
      bInterfaceSubClass      0 Unused
      bInterfaceProtocol      0 
      iInterface              0 
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        3
      bAlternateSetting       1
      bNumEndpoints           2
      bInterfaceClass        10 CDC Data
      bInterfaceSubClass      0 Unused
      bInterfaceProtocol      0 
      iInterface              0 
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x84  EP 4 IN
        bmAttributes            2
          Transfer Type            Bulk
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0200  1x 512 bytes
        bInterval               0
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x04  EP 4 OUT
        bmAttributes            2
          Transfer Type            Bulk
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0200  1x 512 bytes
        bInterval               0
Device Qualifier (for other device speed):
  bLength                10
  bDescriptorType         6
  bcdUSB               2.00
  bDeviceClass            0 (Defined at Interface level)
  bDeviceSubClass         0 
  bDeviceProtocol         0 
  bMaxPacketSize0        64
  bNumConfigurations      1
Device Status:     0x0000
  (Bus Powered)
