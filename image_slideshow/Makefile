# Makefile for Image Slideshow project (Z88DK, Galaksija)
#
# Usage:
#   1. Place images in original_images/
#   2. Run: make
#   3. Clean: make clean

ZCC = zcc
TARGET = build/slideshow
SOURCES = c/slideshow.c c/galaksija.c c/welcome_screen.c c/images.c
ZCCFLAGS = +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic -Ic

ORIGINAL_IMAGES = $(wildcard original_images/*)
PREVIEW_UTILS = python/preview_utils.py

.PHONY: all clean builddir

all: builddir $(TARGET) build/slideshow_preview.gif build/slideshow_preview.mp4

builddir:
	@mkdir -p build

c/constants.h: python/constants.py
	python3 python/constants.py c/constants.h

c/images.c: $(ORIGINAL_IMAGES) python/convert_images.py python/constants.py | builddir
	python3 python/convert_images.py original_images c --preview-dir build

c/images.h: c/images.c

$(TARGET): $(SOURCES) c/constants.h | builddir
	$(ZCC) $(ZCCFLAGS) -o $(TARGET) $(SOURCES)

build/slideshow_preview.gif: $(ORIGINAL_IMAGES) python/generate_gif.py $(PREVIEW_UTILS) python/constants.py | builddir
	python3 python/generate_gif.py original_images build/slideshow_preview.gif

build/slideshow_preview.mp4: $(ORIGINAL_IMAGES) python/generate_video.py $(PREVIEW_UTILS) python/constants.py | builddir
	python3 python/generate_video.py original_images build/slideshow_preview.mp4

clean:
	rm -rf build c/images.c c/images.h c/constants.h
