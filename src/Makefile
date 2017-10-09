CC = gcc
CPPFLAGS =
CFLAGS = -std=c11 -D_XOPEN_SOURCE=700 -O2 -Wall -Wextra -Wformat=2
LDFLAGS =
LOADLIBES =
LDLIBS = 
OBJFLAGS = -c

.DEFAULT: all
.PHONY: all
all: 
	$(CC) $(CFLAGS) httpd.c -o httpd

server:
	$(CC) $(CFLAGS) httpd.c -o httpd

clean: rm -f *.o

distclean: rm -f httpd
