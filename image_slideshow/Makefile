# Makefile for Image Slideshow project (Z88DK, Galaksija)
#
# Usage:
#   1. Place images in the original_images/ folder
#   2. Run: make
#   3. Clean: make clean

ZCC      = zcc
TARGET   = build/slideshow
SOURCES  = slideshow.c galaksija.c welcome_screen.c images.c
ZCCFLAGS = +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic

ORIGINAL_IMAGES = $(wildcard original_images/*)

.PHONY: all clean builddir

all: builddir $(TARGET)

builddir:
	@mkdir -p build

images.c: $(ORIGINAL_IMAGES) convert_images.py
	python3 convert_images.py original_images .

images.h: images.c

$(TARGET): $(SOURCES) | builddir
	$(ZCC) $(ZCCFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -rf build images.c images.h
