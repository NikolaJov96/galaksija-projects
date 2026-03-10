# Image Slideshow

Displays a sequence of images on the screen, cycling through them with a short delay.

Images are converted to the Galaksija's native black-and-white block-graphics format and baked into the program binary at build time.

## Controls

| Key | Action |
|-----|--------|
| RIGHT / any key | Skip to the next image |
| LEFT | Go to the previous image |
| DEL | Exit the slideshow |

## Build

Place your images (PNG, JPG, BMP, ...) into the `original_images` folder.

Install Python dependencies:

```console
pip install -r requirements.txt
```

Ensure Z88DK is properly installed, then build.

On Windows, run:

```console
.\build_slideshow.bat
```

On Linux, run:

```bash
make
```

The build system automatically converts the images and compiles the program.

## Technical notes

The Galaksija screen is 32x16 character cells. Each character encodes a 2x3 pixel block,
giving an effective resolution of 64x48 pixels.

By default the converter stores images at 1.5x the screen resolution (48x24 chars), and
the slideshow pans the viewport from the top-left to the bottom-right corner. The
`--multiplier` flag controls this; pass `1.0` to disable panning.

The converter supports several dithering methods via `--dither`: `floyd-steinberg`
(default), `bayer2`, `bayer4`, `random`, and `blue-noise`.

Display timing is controlled by constants in `slideshow.c`:
- `PAN_STEPS` (default: 8) - number of pan positions per image.
- `PAN_DELAY` (default: 15) - delay loop iterations between pan steps.
- `HOLD_DELAY` (default: 40) - extra iterations to hold the final frame.
- `STATIC_DELAY` (derived: `(PAN_STEPS + 1) * PAN_DELAY`) - total delay for non-panning images.
