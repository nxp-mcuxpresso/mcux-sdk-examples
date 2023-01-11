import time
import serial
import os
import glob, os
import simplejson as json
import sys
if sys.platform.startswith('linux'):
    import pexpect.fdpexpect as pexpect
else:
    import xexpect as pexpect

class Dut(object):
    def __init__(self, id):
        log = open("logs/tty-%s.log" %id, 'ab')
        self.serial_id = id
        self.find_serial_path()
        self.ser = serial.Serial(port=self.serial_path, baudrate=115200, timeout=1)
        #self.cli = pexpect.fdspawn(self.ser)
        self.cli = pexpect.fdspawn(self.ser)
        self.cli.logfile = log
        time.sleep(0.5)

        #self.cli.expect("=>")
        #self.reset()
    def __del__(self):
        self.ser.close()

    def reset(self):
        try:
            self.cli.send("reset\r")
            self.cli.expect("=>", 3)
            if sys.platform.startswith('linux'):
                self.cli.send("echo off\r")
                self.cli.expect("=>")
            else:
                self.cli.send("echo on\r")
                self.cli.expect("=>")
            return
        except:
            pass
        if sys.platform.startswith('linux'):
            os.system("/usr/local/mcuxpressoide/ide/binaries/crt_emu_cm_redlink --reset hard --probeserial %s" %self.serial_id)
        else:
            os.system('powershell -c \"C:\\nxp\\MCUXpressoIDE_11.3.1_5248_prc\\ide\\binaries\\crt_emu_cm_redlink --reset hard --probeserial  %s\"' %self.serial_id)
        try:
            self.cli.expect("Modelrunner", 15)
            self.cli.expect("=>")
            if sys.platform.startswith('linux'):
                self.cli.send("echo off\r")
                self.cli.expect("=>")
            else:
                self.cli.send("echo on\r")
                self.cli.expect("=>")
        except:
            pass

    def find_serial_path(self):
        if sys.platform.startswith('linux'):
            #self.serial_path = '/dev/ttyS5'
            #return
            owd = os.getcwd()
            os.chdir("/dev/serial/by-id")
            for file in glob.glob("*%s*" %self.serial_id):
                self.serial_path = "/dev/serial/by-id/%s" %file
            os.chdir(owd)
        else:
            import subprocess
            import re
            cmd = "powershell -c \"((gwmi Win32_USBControllerDevice | %{[wmi]($_.Dependent)} | ?{($_.DeviceID -like '*" + self.serial_id + "*')}).DeviceID |  %{Get-ItemProperty -Path \"HKLM:\SYSTEM\CurrentControlSet\Enum\$_\"}).ParentIdPrefix\""

            o = subprocess.getoutput(cmd).strip()
            cmd =  "powershell -c \"(Get-WmiObject Win32_USBControllerDevice | %{[wmi]($_.Dependent)} |  ?{($_.Service -like 'usbser')} | ?{($_.DeviceID -like '*" + o + "*')}).Name\""
            o = subprocess.getoutput(cmd).strip()
            r = re.match(".*(COM\d+).*", o)
            if(r):
                self.serial_path = r.group(1)
            else:
                cmd = "powershell -c \"(gwmi Win32_USBControllerDevice |%{[wmi]($_.Dependent)}| ?{($_.DeviceID -like '*" + self.serial_id + "*')} | ?{($_.Service -like 'FTSER2K')} | ?{($_.hardwareid -like '*mi_01')}).Name\""
                o = subprocess.getoutput(cmd).strip()
                r = re.match(".*(COM\d+).*", o)
                if (r):
                    self.serial_path = r.group(1)
            return 

    def send_cmd(self, cmd):
        self.cli.write("\r")
        self.cli.expect("(.*)=>", timeout=300)
        self.cli.write("%s\r" %cmd)
        idx = self.cli.expect(["Results:(.*)=>", "TFLite Modelrunner UART"], timeout=300)
        if idx == 1:
            raise Exception("target reset")
        results = self.cli.match.group(1).strip()
        return results

    def send_file(self, cmd, filename):
        fs = os.stat(filename)
        self.cli.write("%s %d\r" %(cmd, fs.st_size))
        self.cli.expect("Ready for.* (.+)\r\n")
        mem = self.cli.match.group(1).decode()
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
                    self.cli.expect("==", timeout=130)
                    a = 0
                    #time.sleep(0.4)
                a+=1
            self.ser.write(b'%%%%%%%')
        idx = self.cli.expect(["=>", "not equal to supported version", "Arena size is too small for all buffers", "TFLite Modelrunner UART"], timeout=300)
        if idx == 1:
            raise Exception("model schema version error")
        if idx == 2:
            raise Exception("arena size is too small for all buffers")
        if idx == 3:
            raise Exception("target reset")

if __name__ == "__main__":
    dut = Dut("FQA2BQPQ")
    dut.send_file("model_loadb" ,"/home/xiao/work/eiq/model/mlperf_tiny_models/keyword_spotting/kws_ref_model.tflite")
    dut.send_file("tensor_loadb input_1" ,"/home/xiao/work/eiq/eembc/benchmark-runner-ml/datasets/kws01/tst_000966_Up_8.bin")
    results = dut.send_cmd("run")
    j = json.loads(results)
    print(json.dumps(j, indent=4,sort_keys=True))
    #results = dut.send_cmd("model")

    #j = json.loads(results)
    #print(json.dumps(j, indent=4,sort_keys=True))

    #dut.send_file("/home/xiao/work/eiq/sdk/eiq/tensorflow-lite/examples/label_image/pcq/mobilenet_v1_0.25_128_quant_int8.tflite")

    #dut.send_file("/tmp/x.log")
    #dut.send_file("/home/xiao/work/eiq/neutron/neutron_testing/tmp/conv_pw_in15x15_f32_tflite_prequantized_glow_cmodel_prequantized/model.tflite")

