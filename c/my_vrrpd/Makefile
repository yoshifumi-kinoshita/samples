all: my_vrrpd

my_vrrpd: my_vrrpd.c
	gcc -Wall -g -lpthread my_vrrpd.c -o my_vrrpd

add: add.c
	gcc -Wall -g add.c -o add

del: del.c
	gcc -Wall -g del.c -o del

set_mac: set_mac.c
	gcc -Wall -g set_mac.c -o set_mac

clean:
	rm -f my_vrrpd
