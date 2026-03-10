# Makefile for Image Slideshow project (Z88DK, Galaksija)
#
# Usage:
#   1. Place images in original_images/
#   2. Run: make
#   3. Clean: make clean

ZCC = zcc
TARGET = build/slideshow
SOURCES = slideshow.c galaksija.c welcome_screen.c images.c
ZCCFLAGS = +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic

ORIGINAL_IMAGES = $(wildcard original_images/*)
PREVIEW_UTILS = preview_utils.py

.PHONY: all clean builddir

all: builddir $(TARGET) build/slideshow_preview.gif build/slideshow_preview.mp4

builddir:
	@mkdir -p build

images.c: $(ORIGINAL_IMAGES) convert_images.py
	python3 convert_images.py original_images .

images.h: images.c

$(TARGET): $(SOURCES) | builddir
	$(ZCC) $(ZCCFLAGS) -o $(TARGET) $(SOURCES)

build/slideshow_preview.gif: $(ORIGINAL_IMAGES) generate_gif.py $(PREVIEW_UTILS) | builddir
	python3 generate_gif.py original_images build/slideshow_preview.gif

build/slideshow_preview.mp4: $(ORIGINAL_IMAGES) generate_video.py $(PREVIEW_UTILS) | builddir
	python3 generate_video.py original_images build/slideshow_preview.mp4

clean:
	rm -rf build images.c images.h
