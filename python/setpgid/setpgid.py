#!/usr/bin/env python

import os
import sys
import time

try:
	pid = os.fork()

except OSError, e: 
	raise Exception, "%s [%d]" % (e.strerror, e.errno)

if pid > 0:
	time.sleep( 30 )
	sys.exit(0)
else:
	os.setsid()
	os.system('/bin/sleep 30')


