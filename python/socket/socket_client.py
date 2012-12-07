#!/usr/bin/python

import sys
import getopt
import socket
import time

def main():
	host=sys.argv[1]
	port=int( sys.argv[2] )

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.settimeout(3600)
	s.connect((host,port))
	
	s.send(host)
	time.sleep(30)
	s.close()

if __name__ == "__main__":
    main()

