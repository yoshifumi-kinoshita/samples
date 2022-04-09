import os
import time

def main():
    pid = os.fork()
  
    if pid > 0:
        print("parent pid: ", os.getpid())
        print("sleeping ...")
        time.sleep(60)
        status = os.wait()
        print("Terminated child's process id:", status[0])
        print("Signal number that killed the child process:", status[1])
        time.sleep(30)
        exit(0)
    else:
        print("child pid: ", os.getpid())
        time.sleep(30)
        exit(0)

if __name__ == "__main__":
    main()

