import sys
import pycurl
import certifi
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO, BytesIO
import json

STREAM_URL = "https://api.particle.io/v1/devices/events?access_token=7cab32b80bbbfdf8e1e83220c7479343a53e826e"
#USER = "davegutz@alum.mit.edu"
#PASS = "stevie18g"


class Client:
    def __init__(self):
        self.buffer = ""
        self.f = open('out.txt', 'wt')
        self.conn = pycurl.Curl()
        self.conn.setopt(pycurl.URL, STREAM_URL)
        # self.conn.setopt(pycurl.USERPWD, "%s:%s" % (USER, PASS))
        self.conn.setopt(pycurl.WRITEFUNCTION, self.on_receive)
        self.conn.setopt(self.conn.CAINFO, certifi.where())
        #sys.stdout = self.f
        self.conn.perform()

    def on_receive(self, byte_obj):
        text_obj = byte_obj.decode('UTF-8').strip()
        if 'vent,' in text_obj:
            print(text_obj)


def main():
    client = Client()


if __name__ == '__main__':
    sys.exit(main())
