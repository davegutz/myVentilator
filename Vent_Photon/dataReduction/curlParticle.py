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

STREAM_URL = "https://api.particle.io/v1/devices/events?access_token=7cab32b80bbbfdf8e1e83220c7479343a53e826e"
DEVICE_NAME = 'vent'
cts = datetime.datetime.now().isoformat()
cts = cts.replace(':', '-')
FILE_NAME = DEVICE_NAME+'_'+cts[0:16]+'.csv'
TRAIL_STR = '","ttl"'
DEVICE_NAME_LENGTH = len(DEVICE_NAME)
#USER = "davegutz@alum.mit.edu"
#PASS = "stevie18g"


class Client:
    def __init__(self):
        self.buffer = ""
        self.f = open(FILE_NAME, 'wt')
        self.conn = pycurl.Curl()
        self.conn.setopt(pycurl.URL, STREAM_URL)
        # self.conn.setopt(pycurl.USERPWD, "%s:%s" % (USER, PASS))
        self.conn.setopt(pycurl.WRITEFUNCTION, self.on_receive)
        self.conn.setopt(self.conn.CAINFO, certifi.where())
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


def main():
    client = Client()


if __name__ == '__main__':
    sys.exit(main())
