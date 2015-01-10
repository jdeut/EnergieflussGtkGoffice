DEV=0


DOC_MODULE=energie
DOC_DIR=../docs/reference/$(DOC_MODULE)


CFILES=$(shell find . -iname "*.c")
OBJ=$(patsubst %.c, %.o, $(CFILES))

SYS:=$(shell uname) 

CC=gcc
PKGS=gtk+-3.0 gmodule-2.0 librsvg-2.0
PKGS_LINUX=$(PKGS) libgsf-1
CFLAGS=`pkg-config --cflags $(PKGS_LINUX)`
LDLIBS=-lm `pkg-config --libs $(PKGS_LINUX)` 

ifeq ($(DEV), 1)
  CFLAGS:=-ggdb -I$$HOME/Src/goffice $(CFLAGS)
  LDLIBS:=$$HOME/Src/goffice/goffice/.libs/libgoffice-0.10.a $(LDLIBS)
else
  PKGS:=libgoffice-0.10 $(PKGS)
endif

FILES_FOR_BUNDLE=main.exe style.css ui.glade dialog-property-editor.ui

# if system is win32 (msys2)
ifneq (, $(findstring MSYS, $(SYS)))
  # the order matters since libgsf-win32-1 is a static library
  PKGS:=libgsf-win32-1 $(PKGS)
  CFLAGS=-ggdb -I../../goffice/ `pkg-config --cflags $(PKGS)`
  LDLIBS=-mwindows -L../../goffice/goffice -lgoffice-0.10 -L/mingw64/lib `pkg-config --libs $(PKGS)` -lz
endif

main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

win32bundle: main
ifneq (, $(findstring MSYS, $(SYS)))
	  mkdir -p bundle
	  cp `ldd main.exe | perl -ne 'if (/\s(\/mingw64\/\S*dll)/) { print $$1, " " }'` bundle
	  cp $(FILES_FOR_BUNDLE) bundle
	  mkdir -p bundle/share/icons/Adwaita
	  cp -r /mingw64/share/icons/Adwaita/* bundle/share/icons/Adwaita
endif

doc:
	gtkdoc-scan --module=$(DOC_MODULE) --source-dir=system
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDLIBS) system/my-flowarrow.o system/my-system.o dialog-property-editor.o" gtkdoc-scangobj --module=$(DOC_MODULE)
	gtkdoc-mkdb --module=$(DOC_MODULE) --output-format=xml
	mkdir -p $(DOC_DIR)
	cd $(DOC_DIR) && gtkdoc-mkhtml $(DOC_MODULE) ../../../src/$(DOC_MODULE)-docs.xml
	gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=$(DOC_DIR)

clean:
	rm -f $(OBJ) main
