import threading

class MyTask(threading.Thread) :
	def run(self) :
		print os.system('mysql -uroot -h 10.64.221.152 --password='' test')

i=0
j=0
t=range(1000)

while i < 1000:
	t[i] = MyTask()

while j < 1000:
	t[j].run()

