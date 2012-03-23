#!/usr/bin/env python

import sys

allstats = {}

f = open(sys.argv[1])
line = f.readline()
while line:
	cols = line.split()
	port = cols[2].split('.')[-1]
	if allstats.has_key( port ):
		allstats[ port ] += 1
	else:
		allstats[ port ] = 1
	
	line = f.readline()

f.close

for k, v in sorted( allstats.items(), key=lambda x:x[1] ):
	print k, v

