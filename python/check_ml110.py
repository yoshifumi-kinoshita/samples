#!/bin/env python

import sys
import httplib
import re
import smtplib

URL='http://nttxstore.jp/_II_HP13894432'
FROM='ykinoshi@redhat.com'
TO= sys.argv[1] if len(sys.argv)>1 else 'ykinoshi@redhat.com'
SUBJECT='ML110 G7'


def main():
	price = get_price()
	msg = 'From: %s\r\nTo:%s\r\nSubject:%s\r\n\r\n' % (FROM,TO,SUBJECT) + 'ML110 G7: ' + price + ' yen'
	send_mail( FROM, TO, msg )

def get_price():
	conn = httplib.HTTPConnection("nttxstore.jp")
	conn.request("GET", "/_II_HP13894432")
	result = conn.getresponse()
	cont = result.read()
	price = re.search(r'.*<dd class="price"><span>(.*?00)', cont).group(1)
	coupon = ' (+coupon)' if re.search(r'coupon', cont) else ''
	return price + coupon

def send_mail( fromaddr, toaddr, msg ):
	server = smtplib.SMTP('int-mx.corp.redhat.com.')
	server.sendmail(fromaddr, toaddr, msg)
	server.quit()


if __name__ == "__main__":
	main()

