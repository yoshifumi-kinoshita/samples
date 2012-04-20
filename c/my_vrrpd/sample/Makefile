CC 	=  gcc
#CFLAGS 	=  -Wall -g -O2
CFLAGS 	=  -Wall -g

INCDIRS = -I.
CFLAGS	+= $(INCDIRS)

LIBDIRS =
LIBS	=


all: get_ifinfo arp_send gratuitous_arp

get_ifinfo: get_ifinfo.c
	${CC} ${CFLAGS} -o get_ifinfo get_ifinfo.c ${OBJ} ${LIBDIRS} ${LIBS}

arp_send: arp_send.c
	${CC} ${CFLAGS} -o arp_send arp_send.c ${OBJ} ${LIBDIRS} ${LIBS}

gratuitous_arp: gratuitous_arp.c
	${CC} ${CFLAGS} -o gratuitous_arp gratuitous_arp.c ${OBJ} ${LIBDIRS} ${LIBS}

clean :
	rm -f get_ifinfo arp_send gratuitous_arp
