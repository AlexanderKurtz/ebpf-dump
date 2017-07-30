CC       = gcc
CFLAGS   = -std=gnu11 -O2 -Wall -Werror
PROGRAMS = server-raw server-tcp server-udp

all: $(PROGRAMS)

server-raw: common.c -lbcc

server-tcp: common.c -lbcc

server-udp: common.c -lbcc

clean:
	rm --force --verbose -- $(PROGRAMS)

.PHONY: all clean
