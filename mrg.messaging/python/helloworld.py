#!/usr/bin/env python

import sys
from qpid.messaging import *

#broker =  "localhost:5672" if len(sys.argv)<2 else sys.argv[1]
#address = "amq.topic" if len(sys.argv)<3 else sys.argv[2]
broker =  len(sys.argv)<2 and "localhost:5672" or sys.argv[1]
address = len(sys.argv)<3 and "amq.topic" or sys.argv[2]

connection = Connection(broker)

try:
  connection.open()
  session = connection.session()

  sender = session.sender(address)
  receiver = session.receiver(address)

  sender.send(Message("Hello world!"));

  message = receiver.fetch(timeout=1)
  print message.content
  session.acknowledge()

except MessagingError,m:
  print m
#finally:
#  connection.close()

connection.close()
