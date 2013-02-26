#!/usr/bin/env python

import sys
import socket
import threading

NUM=100

host = sys.argv[1]
port = int(sys.argv[2])
msg = sys.argv[3]

class MyTask(threading.Thread) :
	def __init__(self, host, port, msg):
		self.host = host
		self.port = port
		self.msg = msg

        def run(self) :
                self.send_msg()

	def send_msg(self) :
		serversock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		k=0
		while k < NUM:
			serversock.sendto(self.msg, (self.host, self.port))
			k += 1


def main():
    i=0
    j=0
    t=range(NUM)
    while i < NUM:
        t[i] = MyTask(host, port, msg)
        i += 1

    while j < NUM:
        t[j].run()
        j += 1


if __name__ == "__main__":
        main()
