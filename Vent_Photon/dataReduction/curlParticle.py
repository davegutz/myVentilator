# curlParticle.py
# Stream Particle cloud data from Vent_Photon.ino to local files for local use.
# Edit Vent_Photon in Particle Workbench Visual c++ (main.h).
# Get cURL link from https://console.particle.io/events.   Click on "view events on a terminal"
# That's wehre the link to STREAM_URL came from.
# Running:
# 	install python:  3.6.8 is latest that supports curl.   Pick option to add python to path.
# 	install dependencies:  cygwin> python -m pip install pycurl certifi
#   go to particle.io and get token:https://console.particle.io/devices/
#           click on device.   Select the little square icon.   Copy token.  Paste into STREAM_URL below
#	running:   	cygwin> cd /cygdrive/c/Users/Dave/Documents/GitHub/myVentilator/Vent_Photon/dataReduction
#				cygwin> python curlParticle.py
# Plotting:
#	install sciLab 6.1 or later.  (Didn't work with 5.5.2)
# 	install pdftk (use win8 compatibility)
# 	windows - dataReduction> double-click on Vent_data_reduce.sce
# <or> open debug.csv in msexcel
# MIT License
#
# 2021-Feb-02 	Dave Gutz 		Create
# 2021-Nov-08   Dave Gutz       Verify operation after computer rebuild.   Token had changed

import sys
import pycurl
import certifi
import string
import datetime
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO, BytesIO
import json

STREAM_URL = "https://api.particle.io/v1/devices/events?access_token=9661ef494e1719ef619ff2c1e92742e40ddfac0d"
DEVICE_NAME = 'vent'
cts = datetime.datetime.now().isoformat()
cts = cts.replace(':', '-')
FILE_NAME = DEVICE_NAME+'_'+cts[0:16]+'.csv'
DEBUG_FILE_NAME = 'debug.csv'
TRAIL_STR = '","ttl"'
DEVICE_NAME_LENGTH = len(DEVICE_NAME)
#USER = "davegutz@alum.mit.edu"
#PASS = "stevie18g"


class Client:
    def __init__(self):
        self.buffer = ""
        self.f = open(FILE_NAME, 'wt')
        self.fb = open(DEBUG_FILE_NAME, 'wt')
        self.conn = pycurl.Curl()
        self.conn.setopt(pycurl.URL, STREAM_URL)
        # self.conn.setopt(pycurl.USERPWD, "%s:%s" % (USER, PASS))
        self.conn.setopt(pycurl.WRITEFUNCTION, self.on_receive)
        self.conn.setopt(self.conn.CAINFO, certifi.where())
        # self.conn.setopt(self.conn.VERBOSE, True)
        self.conn.perform()

    def on_receive(self, byte_obj):
        text_obj = byte_obj.decode('UTF-8').strip()
        beg = text_obj.find(DEVICE_NAME+',')
        if beg>0:
            beg += DEVICE_NAME_LENGTH+1
            end = text_obj.find(TRAIL_STR)
            csv_string = text_obj[beg:end]
            if len(csv_string)>1:
                print(csv_string)
                self.f.write(csv_string+'\n')
                self.f.flush()
                self.fb.write(csv_string+'\n')
                self.fb.flush()


def main():
    client = Client()


if __name__ == '__main__':
    sys.exit(main())
