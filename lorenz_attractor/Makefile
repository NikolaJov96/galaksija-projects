# Makefile for Lorenz Attractor project (Z88DK, Galaksija)
# Usage: make (or make clean)

ZCC = zcc
TARGET = build/lorenz
SOURCES = lorenz.c galaksija.c globals.c welcome_screen.c
ZCCFLAGS = +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic

.PHONY: all clean builddir

all: builddir $(TARGET)

builddir:
	@mkdir -p build

$(TARGET): $(SOURCES) | builddir
	$(ZCC) $(ZCCFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -rf build
