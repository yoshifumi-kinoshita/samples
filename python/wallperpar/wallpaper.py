#!/usr/bin/env python

import os
import sys
import signal
import time
import random
import shutil


MAX_CHILDREN=1
INTERVAL=180
PID_FILE='/var/run/wallpaper.pid'
PICTURE_DIR=os.environ['HOME'] + '/Pictures'

hash_children={}

def main():
    daemonize()

    #write_pid()

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
        change_wallpaper()
        time.sleep( INTERVAL )


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

def change_wallpaper():
    cwd = os.getcwd()
    os.chdir( PICTURE_DIR )
    files = os.listdir('wallpapers')
    random.seed( time.time() )
    i = random.randint( 0, len(files)-1 )
    shutil.copy("wallpapers/%s" % (files[i])  , 'wallpaper')
    os.chdir( cwd )

if __name__ == "__main__":
    main()

