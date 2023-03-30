import re
import time
import os
import threading

TIMEOUT = Exception("TIMEOUT")

class fdspawn():
    def __init__(self, ser):
        self.ser = ser
        self.after = b""
        pass
    def expect(self, s, timeout=5):
        try:
            if self.logfile.name != "<stdout>":
                log_t = threading.Thread(target=self.log_sync,)
                self.logsync = True
                log_t.start()
        except:
            pass
        self.logsync = True
        self.TIMEOUTExp = False
        self.c = self.after
        r_list = []
        if isinstance(s, list):
            for p in s:
                if p == TIMEOUT:
                    r_list.append(TIMEOUT)
                    self.TIMEOUTExp = True
                    continue
                r_list.append(re.compile(rb'.*%s' %p.encode(), re.DOTALL | re.IGNORECASE))
        else:
            r_list.append(re.compile(rb'.*%s' %s.encode(), re.DOTALL | re.IGNORECASE))
        t = time.time()
        while 1:
            c = self.ser.read(1)
            if self.logfile:
                self.logfile.write(c)
                self.logfile.flush()
            self.c += c
            for r in r_list:
                if r == TIMEOUT:
                    continue
                ccc = r.match(self.c)
                if ccc:
                    self.match_index = r_list.index(r)
                    if ccc.lastindex:
                        self.after = self.c[ccc.lastindex:]
                    else:
                        self.after = b''
                    self.before = self.c
                    self.match = ccc
                    self.logsync = False
                    return self.match_index
            if time.time() - t > timeout:
                if self.TIMEOUTExp:
                    return r_list.index(TIMEOUT)
                self.logsync = False
                raise TIMEOUT
    def write(self, s):
        self.ser.write(str.encode(s))
        if self.logfile:
            self.logfile.write(str.encode(s))
            self.logfile.flush()
    def send(self, s):
        self.ser.write(str.encode(s))
    def log_sync(self):
        while 1:
            os.fsync(self.logfile)
            time.sleep(1)
            if self.logsync == False:
                break

