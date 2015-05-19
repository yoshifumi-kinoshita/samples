#!/usr/bin/env python

import struct, fcntl, os, time

f = open(".history", "r+")
rv = fcntl.fcntl(f, fcntl.F_SETFL, os.O_NDELAY)
lockdata = struct.pack('hhllhh', fcntl.F_WRLCK, 0, 0, 0, 0, 0)
rv = fcntl.fcntl(f, fcntl.F_SETLKW, lockdata)
time.sleep( 10 )

