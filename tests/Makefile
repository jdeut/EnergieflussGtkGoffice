PKGS=json-glib-1.0 glib-2.0 
CFLAGS=-g3 `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=`pkg-config --libs $(PKGS)`

CFILES=$(shell find $(CURDIR) -iname "*.c")
EXE=$(patsubst %.c, %, $(CFILES))

all:
	@@echo "nothing"

clean:
	rm -f $(EXE)
