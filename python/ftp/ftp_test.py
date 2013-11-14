#!/usr/bin/env python

import os
import sys
import getopt
import signal
import random
import time
from ftplib import FTP

MAX_CHILDREN=1
PID_FILE='/var/run/my_prefork_daemon.pid'

hash_children={}

host = "localhost"
user = "anonymous"
password = "anonymous@"

def main():
    init_opt()

    #daemonize()

    write_pid()

    signal.signal(signal.SIGTERM, kill_all_children )

    while True:
        while len( hash_children )>=MAX_CHILDREN:
            child_pid = os.wait()
            del( hash_children[child_pid] )

        pid = os.fork()
        if pid == 0:
            signal.signal( signal.SIGTERM, lambda sig,status:sys.exit(0) )
            break
        else:
            hash_children[pid]=1
            time.sleep(0.1)

    while True:
        random.seed( time.time() )
        i = random.randint( 0, 1 )
        time.sleep(i)
	ftp_test()


def kill_all_children(sig, status):
    for pid in hash_children.keys():
        os.kill( pid, signal.SIGTERM )
    os.exit(0)


def write_pid():
    f = open(PID_FILE, mode='w');
    f.write("%d" % os.getpid())
    f.close()

def daemonize():
    os.chdir("/")
    
    try:
        pid = os.fork()
        if pid > 0:
            sys.exit(0)
            
    except OSError, e: 
        raise Exception, "%s [%d]" % (e.strerror, e.errno)

    os.setsid()
    os.umask(0)
    sys.stdin = open('/dev/null', 'r')
    sys.stdout = open('/dev/null', 'w')
    sys.stderr = open('/dev/null', 'w')

def ftp_test():
    ftp = FTP(host)
    ftp.set_debuglevel(2)
    ftp.login(user,password)
    ftp.retrlines('LIST')
    ftp.quit()

def init_opt():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h:u:p", ["host=", "user=","password="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-h", "--host"):
            host = a
        if o in ("-u", "--user"):
            user = a
        if o in ("-p", "--password"):
            password = a
    print(user)
    print(password)
    print(host)

def usage():
    print 'python ftp_test.py --host=<HOST> --user=<USER> --password=<PASSWORD>'

if __name__ == "__main__":
    main()
