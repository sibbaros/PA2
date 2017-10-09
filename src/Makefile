CC = gcc
CPPFLAGS =
CFLAGS = -g -std=c11 -D_XOPEN_SOURCE=700 -O2 -Wall -Wextra -Wformat=2 #$(shell pkg-config --cflags   glib-2.0)
CFLAGS += `pkg-config --cflags --libs glib-2.0`
LDFLAGS =
LOADLIBES =
LDLIBS = #$(shell pkg-config --cflags glib-2.0)#-I/usr/include/glib-2.0 -lglib-2.0
OBJFLAGS = -c

.DEFAULT: all
.PHONY: all
all: 
    $(CC) $(CFLAGS) httpd.c -o httpd

clean:
    rm -f *.o

distclean: clean
    rm -f httpd