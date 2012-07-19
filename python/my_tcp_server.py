import sys
import socket
import time



serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#serversocket.bind((socket.gethostname(), 8080))
serversocket.bind((socket.gethostbyname("0.0.0.0"), 8080))
serversocket.listen(5)
(client_sock, client_addr) = serversocket.accept()
time.sleep(30)
