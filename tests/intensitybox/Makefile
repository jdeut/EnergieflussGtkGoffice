PKGS=gtk+-3.0 glib-2.0 gmodule-2.0
CFLAGS=-g3 `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=`pkg-config --libs $(PKGS)`

CFILES=main.c ../../src/system/my-intensitybox.c
OBJ=$(patsubst %.c, %.o, $(CFILES))

main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) main
