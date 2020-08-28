import serial
from FileReader import FileReader
import time

port = '/dev/ttyUSB0'
baud = 115200


if __name__ == "__main__":
    serial_port = serial.Serial(port, baud, timeout=1)
    while True:
       val = b''
       while val == b'':
           val = serial_port.read(256)
           if len(val) == 0:
               continue
           print("b: {}".format( [hex(x) for x in val ] ))
