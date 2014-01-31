#!/usr/bin/python

import sys
import getopt
import socket
import time

def main():
	host=sys.argv[1]
	port=int( sys.argv[2] )
	count=int( sys.argv[3] )

	for i in range(count):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.settimeout(3600)
		s.connect((host,port))
		time.sleep(1)
		s.close()
		
	

if __name__ == "__main__":
    main()

