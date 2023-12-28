import time
import serial
import os
import glob, os
import json
import sys
import time
if sys.platform.startswith('linux'):
    import pexpect.fdpexpect as fdpexpect
    import pexpect
else:
    import xexpect as fdpexpect
    import xexpect as pexpect

class Dut(object):
    def __init__(self, id):
        if not os.path.exists(os.path.dirname("logs/")):
            os.makedirs(os.path.dirname("logs/"))
        log = open("logs/tty-%s.log" %id, 'ab')
        self.serial_id = id
        self.find_serial_path()
        while 1:
            try:
                self.ser = serial.Serial(port=self.serial_path, baudrate=115200, timeout=1)
                break
            except:
                time.sleep(5)
                continue
        self.cli = fdpexpect.fdspawn(self.ser)
        self.cli.logfile = log
        time.sleep(0.5)

    def __del__(self):
        self.ser.close()

    def reset(self):
        try:
            self.cli.send("\r")
            self.cli.expect("=>", 3)
            self.cli.send("reset\r")
            self.cli.expect("Modelrunner", 15)
            self.cli.expect("=>")
            return
        except:
            pass
        if sys.platform.startswith('linux'):
            os.system("/usr/local/mcuxpressoide/ide/binaries/crt_emu_cm_redlink --reset hard --probeserial %s" %self.serial_id)
        try:
            self.cli.expect("Modelrunner", 15)
            self.cli.expect("=>")
        except:
            pass

    def find_serial_path(self):
        if sys.platform.startswith('linux'):
            owd = os.getcwd()
            os.chdir("/dev/serial/by-id")
            for file in glob.glob("*%s*" %self.serial_id):
                self.serial_path = "/dev/serial/by-id/%s" %file
                os.chdir(owd)
                return
            os.chdir(owd)
            self.serial_path="/dev/%s" %self.serial_id

        else:
            self.serial_path = self.serial_id
            return

    def send_cmd(self, cmd):
        self.cli.write("\r")
        idx = self.cli.expect(["(.*)=>", pexpect.TIMEOUT], timeout=1)
        if idx == 1:
            return json.dumps({"error": "timeout"})
        self.cli.write("%s\r" %cmd)
        idx = self.cli.expect(["Results:(.*)=> ",  pexpect.TIMEOUT], timeout=10)
        if idx == 1:
            return json.dumps({"outputs": self.cli.match.group(1).strip().decode()})
        elif idx == 2:
            return json.dumps({"error": "timeout"})
        results = self.cli.match.group(1).strip().decode()
        ret = results.replace(",\b","")
        return ret

    def send_file(self, cmd, filename):
        filename = os.path.expanduser(filename)
        fs = os.stat(filename)
        self.cli.write("%s %d\r" %(cmd, fs.st_size))
        idx = self.cli.expect(["Ready for.* (.+)\r\n", "=> ", pexpect.TIMEOUT])
        if idx == 1 or idx == 2:
            return 1
        try:
            mem = self.cli.match.group(1).decode()
        except:
            mem="MEM"
            print(mem)
        time.sleep(1)
        with open(filename, 'rb') as fd:
            a = 1
            time.sleep(3)
            while 1:
                c11 = fd.read(1)
                if c11 == b'':
                    break
                self.ser.write(c11)
                if a == 16384 and mem == "Flash":
                    idx = self.cli.expect(["==", pexpect.TIMEOUT], timeout=130)
                    if idx == 1:
                        return 1
                    a = 0
                    #time.sleep(0.4)
                a+=1
            self.ser.write(b'%%%%%%%')
        self.cli.expect(["=> ", pexpect.TIMEOUT], timeout=300)
        return 0

