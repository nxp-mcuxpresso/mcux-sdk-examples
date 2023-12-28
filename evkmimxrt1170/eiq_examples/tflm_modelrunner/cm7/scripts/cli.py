import xexpect as pexpect
import serial
import json
import re
import sys
from modelrunner import Dut
import signal

def handler(sig, fram):
    print("bye")
    sys.exit(0)

def usage():
    u = '''
 model_loadb <model path> # Download TFLite Model
 tensor_loadb <input tensor name> <tensor path> # Download Input tensor
 run output=<output tensor name> # Model RunInference
 model # Print layer timings.
 exit # exit cli
'''
    print(u)

def parse_cmd(cmd):
    c = cmd.split()
    try:
        if not c:
            d.cli.write("\r")
            idx = d.cli.expect(["TFLite Modelrunner", "=> "],timeout=0.1)
            if idx == 0:
                d.cli.expect("=> ",timeout=0.1)
            d.cli.expect("=> ",timeout=0.1)
        elif c[0] == "tensor_loadb":
            d.send_file(" ".join(c[:-1]),c[2])
        elif c[0] == "model_loadb":
            d.send_file(c[0],c[1])
        elif c[0] == "model":
            d.cli.logfile_read = None
            results = d.send_cmd("model")
            j = json.loads(results)
            bj = json.dumps(j,sort_keys=True,indent=2)
            print(bj)
            d.cli.logfile_read = sys.stdout.buffer
            d.cli.expect(".*=> ", timeout=0.1)
            d.cli.write("\r")
            d.cli.expect(".*=> ")
        elif c[0] == "run":
            d.cli.logfile_read = None
            results = d.send_cmd(cmd)
            j = json.loads(results)
            bj = json.dumps(j,sort_keys=True,indent=2)
            print(bj)
            d.cli.logfile_read = sys.stdout.buffer
            d.cli.expect(".*=> ", timeout=0.1)
            d.cli.write("\r")
            d.cli.expect(".*=> ")
        elif c[0] == "help":
            usage()
            d.cli.write("\r")
            d.cli.expect("=> ")
        elif c[0] == "exit":
            print("bye")
            sys.exit(0)
        elif c[0] == "reset":
            d.reset()
        else:
            d.cli.write("%s\r" %cmd)
            d.cli.expect(".*=> ")
    except Exception as e:
        pass

if __name__ == "__main__":
    signal.signal(signal.SIGINT, handler)
    sid = sys.argv[1]
    d = Dut(sid)
    d.ser.timeout=0.1
    d.cli.logfile_read = sys.stdout.buffer
    d.reset()
    while 1:
        cmd = sys.stdin.readline().strip()
        parse_cmd(cmd)

