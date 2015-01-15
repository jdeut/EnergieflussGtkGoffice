DEV=0


DOC_MODULE=energie
DOC_DIR=../docs/reference/$(DOC_MODULE)
DOC_OBJ=$(shell find system/ -iname "*.o") dialog-property-editor.o


CFILES=$(shell find . -iname "*.c")
OBJ=$(patsubst %.c, %.o, $(CFILES))

SYS:=$(shell uname) 

CC=gcc
PKGS=gtk+-3.0 gmodule-2.0 librsvg-2.0
PKGS_LINUX=$(PKGS) libgsf-1
CFLAGS=-ggdb `pkg-config --cflags $(PKGS_LINUX)`
LDLIBS=-ggdb -lm `pkg-config --libs $(PKGS_LINUX)` 

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
	gtkdoc-scan --module=$(DOC_MODULE) --output-dir=.tmp --source-dir=system
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDLIBS) $(DOC_OBJ)" gtkdoc-scangobj --module=$(DOC_MODULE) --output-dir=.tmp 
	cd .tmp && gtkdoc-mkdb --module=$(DOC_MODULE) --output-format=xml 

	mkdir -p $(DOC_DIR)
	echo $$PWD
	cd $(DOC_DIR) && gtkdoc-mkhtml $(DOC_MODULE) ../../../src/.tmp/$(DOC_MODULE)-docs.xml
	cd .tmp && gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=../$(DOC_DIR)
	rm -rf .tmp

clean:
	rm -f $(OBJ) main
