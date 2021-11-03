import sys
from binascii import unhexlify as uhx

f = open(sys.argv[1])

data = ""
for l in f.readlines():
    data += uhx("".join(l.strip().split(':')[4:]))

f = open(sys.argv[1] + ".bin", "w+")
f.write(data)
