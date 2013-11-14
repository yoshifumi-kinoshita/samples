#!/bin/env python

import sys
import smtplib

FROM='ykinoshi@foo.example.com'
TO= sys.argv[1] if len(sys.argv)>1 else 'ykinoshi@foo.example.com'
MSG=sys.argv[2] if len(sys.argv)>2 else 'test\r\n\r\ntest'

def main():
	msg = read_msg()
	send_mail( FROM, TO, msg )

def read_msg():
	f = open( MSG, 'r' );
	return f.read()

def send_mail( fromaddr, toaddr, msg ):
	server = smtplib.SMTP('localhost')
	server.sendmail(fromaddr, toaddr, msg)
	server.quit()


if __name__ == "__main__":
	main()

