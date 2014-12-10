PKGS=gtk+-3.0 libgoffice-0.10 gmodule-2.0
CFLAGS=-ggdb `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=-lm `pkg-config --libs $(PKGS)`

CFILES=$(shell find . -iname "*.c")
OBJ=$(patsubst %.c, %.o, $(CFILES))

main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) main
