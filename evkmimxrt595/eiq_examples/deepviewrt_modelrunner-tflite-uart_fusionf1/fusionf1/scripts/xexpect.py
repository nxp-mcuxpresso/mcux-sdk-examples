import pexpect.fdpexpect as pexpect
import pexpect as p
import serial
import os
import re
import time

#cmd = """powershell -c \"(Get-WmiObject Win32_USBControllerDevice | %{[wmi]($_.Dependent)} | ?{($_.Manufacturer -like '*NXP*')}| ?{($_.Service -like 'usbser')} | ?{($_.DeviceID -like '*2F6A7901*')}).Name\" """
#cli = p.spawn(cmd)
#s = cli.expect(".*(COM\d+)")
#port = cli.match.group(1).lower()

class spawn():
    def __init__(self, ser):
        self.ser = ser
        self.after = ""
        pass
    def expect(self, s, timeout=5):
        self.c = self.after
        r = re.compile(r".*%s" %s, re.DOTALL | re.IGNORECASE)
        t = time.time()
        while 1:
            self.c += self.ser.read(1).decode()
            ccc = r.match(self.c)
            if ccc:
                self.after = self.c[ccc.lastindex:]
                self.before = self.c
                self.match = ccc
                return ccc
            if time.time() - t > timeout:
                raise Exception("xxx")
                return ccc
    def write(self, s):
        self.ser.write(str.encode(s))
    def send(self, s):
        self.ser.write(str.encode(s))

#qser = serial.Serial('COM14', 115200, timeout=1)
#p = spawn(ser)
#i = p.expect(".*(Modelrunner)", timeout=15)
#print("ssssssss")
#print(i.group(0))
#print(i.group(1))
#p.send("echo off\r")
#i = p.expect("\r\n=>")
#print(i.group(0))

#fd = os.open('com14', os.O_RDWR|os.O_NONBLOC)
#cli = pexpect.fdspawn(fd)


#cli.sendline("\n")

#cli.expect("=>", timeout=5)

#print(cli.before)
#print(cli.match.group(0).lower())
