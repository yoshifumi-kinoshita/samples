#!/bin/env python

import sys
import smtplib
import datetime

from email.MIMEText import MIMEText
from email.MIMEMultipart import MIMEMultipart
from email.Header import Header
from email.Utils import formatdate


FROM='ykinoshi@redhat.com'
TO= sys.argv[1] if len(sys.argv)>1 else 'ykinoshi@redhat.com'
SUBJECT='[My Rader]%s' % formatdate()
USER='ykinoshi'

def main():
	text = 'text'
	html = '<html><body><img src="https://metrics.gss.redhat.com/kcs/individual_radar_chart/%s/week/%s/radar_chart.png"></body></html>' % (USER, datetime.datetime.today().strftime("%Y-%m-%d") )
	send( FROM, TO, SUBJECT, text, html );

def send(from_addr, to_addr, subject, text, html):
	msg = MIMEMultipart('alternative')
	msg['Subject'] = Header(subject, 'iso-2022-jp')
	msg['From'] = from_addr
	msg['To'] = to_addr 
	msg['Date'] = formatdate()
	
	#textpart = MIMEText(text, 'plain', 'iso-2022-jp')
	htmlpart = MIMEText(html, 'html', 'iso-2022-jp')
	
	#msg.attach(textpart)
	msg.attach(htmlpart)
	
	smtp = smtplib.SMTP("int-mx.corp.redhat.com", 25)
	smtp.sendmail(from_addr, [to_addr], msg.as_string())
	smtp.close()


if __name__ == "__main__":
	main()

