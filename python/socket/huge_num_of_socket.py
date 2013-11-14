#!/usr/bin/env python

import socket

host = 'localhost'
port = 80
NUM = 64000
so = range(NUM+1)

for i in range(NUM):
	so[i]= socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	so[i].connect((host, port))
