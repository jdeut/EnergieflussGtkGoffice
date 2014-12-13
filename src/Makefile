SYS:=$(shell uname) 
PKGS=gtk+-3.0 libgoffice-0.10 gmodule-2.0
CFLAGS=-ggdb `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=-lm `pkg-config --libs $(PKGS)`

CFILES=$(shell find . -iname "*.c")
OBJ=$(patsubst %.c, %.o, $(CFILES))

ifneq (, $(findstring MSYS, $(SYS)))
  LDLIBS=-L/mingw64/lib `pkg-config --libs libgoffice-0.10 gmodule-2.0 gtk+-3.0 libgsf-win32-1 librsvg-2.0` -lintl -lz -lm
endif

main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) main
