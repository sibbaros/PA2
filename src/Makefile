CC = gcc
CPPFLAGS =
CFLAGS = -std=c11 -D_XOPEN_SOURCE=700 -O2 -Wall -Wextra -Wformat=2
LDFLAGS = `pkg-config --cflags --libs glib-2.0`
LOADLIBES =
LDLIBS = 
OBJFLAGS = -c

.DEFAULT: all
.PHONY: all
all: 
	$(CC) httpd.c -o httpd $(LDFLAGS) $(CFLAGS)

server:
	$(CC) httpd.c -o httpd $()

clean: rm -f *.o

distclean: rm -f httpd
