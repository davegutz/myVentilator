# curlParticle.py
# Stream Particle cloud data from Vent_Photon.ino to local files for local use.
# Edit Vent_Photon in Particle Workbench Visual c++ (main.h).
# Get cURL link from https://console.particle.io/events.   Click on "view events on a terminal"
# That's where the link to STREAM_URL came from.
# Running:
# 	install python:  3.6.8 is latest that supports curl.   Pick option to add python to path.
# 	install dependencies:  cygwin> python -m pip install pycurl certifi
#   go to particle.io and get token:https://console.particle.io/devices/
#           click on device.   Select the little square icon.   Copy token.  Paste into STREAM_URL below
#	running:   	PS> cd /cygdrive/c/Users/daveg/Documents/GitHub/myVentilator/Vent_Photon/dataReduction
#				PS> python curlParticle.py
#  ************if it seems to quit right away, try browsing to the STREAM_URL below.   Token may not match.   See 'cURL link' above
# Plotting:
#	install sciLab 6.1 or later.  (Didn't work with 5.5.2)
# 	install pdftk (use win8 compatibility)
# 	windows - dataReduction> double-click on Vent_data_reduce.sce
# <or> open debug.csv in msexcel
# MIT License

#
# 2021-Feb-02 	Dave Gutz 		Create
# 2021-Nov-08   Dave Gutz       Verify operation after computer rebuild.   Token had changed
# 2023-Nov-03   Dave Gutz       The token had changed again.  Also had to reset the PLC pressing the button.

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

# Serial.println(
#     F("unit,hm,           cTime,        lim, set, Tp,      Ta,      cmd,    T,   OAT,   Ta_o,     err,   prop,   integ,    cont,    pcnt_pot,duty,Ta_filt,  solar,  heat_o, qduct, mdot, mdot_lag,"));

# STREAM_URL = "https://api.particle.io/v1/devices/290037001551353531343431/events?access_token=74d040345f435c0edd6ed09956a4a960568d7686"
STREAM_URL = "https://api.particle.io/v1/devices/290037001551353531343431/events?access_token=b0220f1425eac6856cec588adec2f97156f56b45"
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
    # sys.exit(main())
    main()
