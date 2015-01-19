PKGS=gtk+-3.0 json-glib-1.0 glib-2.0 libgoffice-0.10 gmodule-2.0
CFLAGS=-g3 `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=-lm `pkg-config --libs $(PKGS)` 
MY_SYSTEM_OBJ=$(shell find $(CURDIR)/../src/system -iname "*.o") ../src/dialog-property-editor.o

CFILES=$(shell find $(CURDIR) -iname "*.c")
EXE=$(patsubst %.c, %, $(CFILES))

all: $(EXE)

json-glib-serialize-my-flow-arrow: json-glib-serialize-my-flow-arrow.o
	$(CC) -o $@ $^ $(MY_SYSTEM_OBJ) $(LDLIBS)

clean:
	rm -f $(EXE) \
	json-glib-serialize-my-flow-arrow.o

