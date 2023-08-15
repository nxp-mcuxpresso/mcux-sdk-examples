#!/usr/bin/python
# This script is used to test the usbx_device_hid_generic example.
# This script will send a string to a device running the usbx_device_hid_generic example,
# and the device will send the string back.

import time
import usb.core
import usb.util
import sys


while True:
    dev = usb.core.find(idVendor=0x1fc9, idProduct=0x00a2)
    if dev:
        break
    time.sleep(1)

print("Found device")

cfg = dev.get_active_configuration()
intf = cfg[(0,0)]

if dev.is_kernel_driver_active(0):
    try:
        dev.detach_kernel_driver(0)
    except usb.core.USBError as e:
        sys.exit("Could not detatch kernel driver from interface({0}): {1}".format(i, str(e)))

endpoint_out = usb.util.find_descriptor(intf,
    # match the first OUT endpoint
    custom_match = \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_OUT)

endpoint_in = usb.util.find_descriptor(intf,
    # match the first OUT endpoint
    custom_match = \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_IN)

message = b'Message from HOST'

while True:
    print("SEND:")
    print(message.decode())
    endpoint_out.write(message)

    buffer = endpoint_in.read(100)
    print("REV:")
    print(buffer.tobytes().decode())
    time.sleep(1)

