SYS:=$(shell uname) 
PKGS=libgoffice-0.10 gtk+-3.0 gmodule-2.0
CFLAGS=-ggdb `pkg-config --cflags $(PKGS)`
CC=gcc
LDLIBS=-lm `pkg-config --libs $(PKGS)`
FILES_FOR_BUNDLE=main.exe style.css ui.glade dialog-property-editor.ui
CFILES=$(shell find . -iname "*.c")
OBJ=$(patsubst %.c, %.o, $(CFILES))

# if system is win32 (msys2)
ifneq (, $(findstring MSYS, $(SYS)))
  LDLIBS=-L/mingw64/lib `pkg-config --libs $(PKGS) libgsf-win32-1 librsvg-2.0` -lz
endif

main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

win32bundle: main
ifneq (, $(findstring MSYS, $(SYS)))
	  mkdir -p bundle
	  cp `ldd main.exe | perl -ne 'if (/\s(\/mingw64\/\S*dll)/) { print $$1, " " }'` bundle
	  cp $(FILES_FOR_BUNDLE) bundle
endif

clean:
	rm -f $(OBJ) main
