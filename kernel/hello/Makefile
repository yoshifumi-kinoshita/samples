CFILES = main.c sub.c

obj-m := hello.o
hello-objs := $(CFILES:.c=.o)

all:
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) V=1 modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

