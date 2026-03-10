#!/usr/bin/env python3
"""
Convert images to Galaksija slideshow C source files.

Galaksija screen layout:
    - 32x16 character cells, each 2x3 pixels => 64x48 effective pixel resolution
    - Pixel block characters use values 128-191 (128 = all black, 191 = all white)
    - Bit layout within each character (bit N => pixel on if set):
        bit 0 (  1): top-left      bit 1 (  2): top-right
        bit 2 (  4): mid-left      bit 3 (  8): mid-right
        bit 4 ( 16): bot-left      bit 5 ( 32): bot-right

Output:
    <output_dir>/images.h          -- declares the image array and size constants
    <output_dir>/images.c          -- defines the image data as a const array
    <output_dir>/galaksija_images/ -- upscaled B&W previews, one PNG per input image
"""

import argparse
from enum import Enum
from pathlib import Path
import numpy as np
from PIL import Image
from PIL.Image import Image as PILImage

from constants import SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_WIDTH, PIXEL_HEIGHT, TOTAL_CHARS
from utils import collect_images

IMAGE_SIZE = TOTAL_CHARS  # alias used in write_header / write_source

# Galaksija character set mappings for Serbian Latin characters.
# Chars 91-94 in Galaksija's ROM render as Č, Ć, Ž, Š respectively.
_GALAKSIJA_CHARS: dict[str, str] = {
    "Č": chr(91),
    "Ć": chr(92),
    "Ž": chr(93),
    "Š": chr(94),
    "Đ": "DJ",
}


def to_galaksija_name(stem: str) -> str:
    """Uppercase stem, replace underscores with spaces, and map Serbian Latin
    characters to Galaksija equivalents."""
    result = ""
    for ch in stem.upper():
        if ch == "_":
            result += " "
        else:
            result += _GALAKSIJA_CHARS.get(ch, ch)
    return result

# Scale factor for the preview images saved to galaksija_images/.
# 8x gives 512x384 — large enough to see clearly, with crisp pixel blocks.
PREVIEW_SCALE = 8

class DitherMethod(str, Enum):
    FLOYD_STEINBERG = "floyd-steinberg"
    BAYER2          = "bayer2"
    BAYER4          = "bayer4"
    RANDOM          = "random"
    BLUE_NOISE      = "blue-noise"

    def __str__(self) -> str:
        return self.value

# Standard Bayer ordered-dither matrices.
# Threshold for pixel (x, y) = (M[y % n][x % n] + 0.5) / n² × 255
_BAYER_2X2: np.ndarray = np.array([[0, 2], [3, 1]], dtype=float)
_BAYER_4X4: np.ndarray = np.array(
    [[ 0,  8,  2, 10],
     [12,  4, 14,  6],
     [ 3, 11,  1,  9],
     [15,  7, 13,  5]],
    dtype=float,
)


def _bayer_dither(gray: np.ndarray, matrix: np.ndarray) -> PILImage:
    h, w = gray.shape
    n = matrix.shape[0]
    threshold = np.tile((matrix + 0.5) / (n * n) * 255, (h // n + 1, w // n + 1))[:h, :w]
    result = ((gray >= threshold) * 255).astype(np.uint8)
    return Image.fromarray(result).convert("1")


def _random_dither(gray: np.ndarray) -> PILImage:
    threshold = np.random.uniform(0, 255, gray.shape)
    result = ((gray >= threshold) * 255).astype(np.uint8)
    return Image.fromarray(result).convert("1")


def _blue_noise_dither(gray: np.ndarray) -> PILImage:
    h, w = gray.shape
    f = np.fft.fft2(np.random.rand(h, w))
    yy, xx = np.meshgrid(np.fft.fftfreq(h) * h, np.fft.fftfreq(w) * w, indexing="ij")
    freq_mag = np.sqrt(xx ** 2 + yy ** 2)
    freq_mag[0, 0] = 1  # avoid zeroing out DC before normalisation
    noise = np.real(np.fft.ifft2(f * freq_mag))
    noise = (noise - noise.min()) / (noise.max() - noise.min()) * 255
    result = ((gray >= noise) * 255).astype(np.uint8)
    return Image.fromarray(result).convert("1")


def apply_dither(img: PILImage, method: DitherMethod) -> PILImage:
    """Convert a colour image to 1-bit B&W using the chosen dithering method."""
    if method == DitherMethod.FLOYD_STEINBERG:
        return img.convert("1", dither=Image.Dither.FLOYDSTEINBERG)
    gray = np.array(img.convert("L"), dtype=float)
    if method == DitherMethod.BAYER2:
        return _bayer_dither(gray, _BAYER_2X2)
    if method == DitherMethod.BAYER4:
        return _bayer_dither(gray, _BAYER_4X4)
    if method == DitherMethod.RANDOM:
        return _random_dither(gray)
    if method == DitherMethod.BLUE_NOISE:
        return _blue_noise_dither(gray)
    raise ValueError(f"Unknown dither method: {method!r}")


def center_crop(img: PILImage, target_ratio: float) -> PILImage:
    """Crop img symmetrically to target_ratio (width / height) without resizing."""
    w, h = img.size
    current_ratio = w / h
    if current_ratio > target_ratio:
        new_w = int(h * target_ratio)
        left = (w - new_w) // 2
        return img.crop((left, 0, left + new_w, h))
    elif current_ratio < target_ratio:
        new_h = int(w / target_ratio)
        top = (h - new_h) // 2
        return img.crop((0, top, w, top + new_h))
    return img


def image_to_galaksija(img_path: Path, dither: DitherMethod, multiplier: float = 1.0) -> tuple[list[int], PILImage, int, int]:
    """Convert an image file to Galaksija character values and a B&W preview image.

    With multiplier > 1, the image is stored at multiplier × the screen resolution,
    producing a (SCREEN_WIDTH*m) × (SCREEN_HEIGHT*m) character grid for pan effects.

    Returns:
        chars     -- list of (SCREEN_WIDTH*m)*(SCREEN_HEIGHT*m) character values
        bw_img    -- the (PIXEL_WIDTH*m)×(PIXEL_HEIGHT*m) B&W PIL Image used to produce chars
        pan_max_x -- max pan offset in X (chars); limited to actual image content
        pan_max_y -- max pan offset in Y (chars); limited to actual image content
    """
    img_cols = round(SCREEN_WIDTH  * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)
    px_w     = img_cols * 2
    px_h     = img_rows * 3

    img = Image.open(img_path).convert("RGB")
    if multiplier <= 1.0:
        # At 1:1 scale, crop to Galaksija's 4:3 ratio then resize to exact screen size.
        img = center_crop(img, PIXEL_WIDTH / PIXEL_HEIGHT)
        img = img.resize((px_w, px_h), Image.LANCZOS)
        pan_max_x = 0
        pan_max_y = 0
    else:
        # At larger scales, preserve the source image's natural aspect ratio.
        # Scale to fit entirely within the oversized grid and place at (0, 0);
        # the remaining area stays black so the pan covers the actual image content.
        src_w, src_h = img.size
        scale = min(px_w / src_w, px_h / src_h)
        content_px_w = round(src_w * scale)
        content_px_h = round(src_h * scale)
        img = img.resize((content_px_w, content_px_h), Image.LANCZOS)
        canvas = Image.new("RGB", (px_w, px_h), (0, 0, 0))
        canvas.paste(img, (0, 0))
        img = canvas
        # Pan limit: how far the viewport can travel before hitting black padding.
        content_cols = content_px_w // 2
        content_rows = content_px_h // 3
        pan_max_x = max(0, content_cols - SCREEN_WIDTH)
        pan_max_y = max(0, content_rows - SCREEN_HEIGHT)
    bw_img = apply_dither(img, dither)

    pixels = bw_img.load()
    chars = []

    for cy in range(img_rows):
        for cx in range(img_cols):
            px = cx * 2
            py = cy * 3

            def p(dx, dy):
                # PIL mode "1": 0 = black (off), 255 = white (on)
                return 1 if pixels[px + dx, py + dy] else 0

            char_val  = 128
            char_val += p(0, 0) *  1   # top-left
            char_val += p(1, 0) *  2   # top-right
            char_val += p(0, 1) *  4   # mid-left
            char_val += p(1, 1) *  8   # mid-right
            char_val += p(0, 2) * 16   # bot-left
            char_val += p(1, 2) * 32   # bot-right

            chars.append(char_val)

    return chars, bw_img, pan_max_x, pan_max_y


def save_preview(bw_img: PILImage, output_dir: Path, stem: str, pan_max_x: int = 0, pan_max_y: int = 0) -> None:
    """Save B&W previews to <output_dir>/galaksija_images/.
    If pan_max_x/y > 0, crops the image to the actual content area (no black padding).
    """
    preview_dir = output_dir / "galaksija_images"
    preview_dir.mkdir(parents=True, exist_ok=True)
    if pan_max_x > 0 or pan_max_y > 0:
        content_px_w = (pan_max_x + SCREEN_WIDTH) * 2
        content_px_h = (pan_max_y + SCREEN_HEIGHT) * 3
        bw_img = bw_img.crop((0, 0, content_px_w, content_px_h))
    bw_img.save(preview_dir / f"{stem}.png")
    upscaled = bw_img.resize(
        (bw_img.width * PREVIEW_SCALE, bw_img.height * PREVIEW_SCALE),
        Image.NEAREST,
    )
    upscaled.save(preview_dir / f"{stem}_8x.png")


def write_header(output_dir: Path, num_images: int, multiplier: float = 1.0) -> None:
    img_cols = round(SCREEN_WIDTH * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)
    with open(output_dir / "images.h", "w", encoding="utf-8") as f:
        f.write("/* Auto-generated by convert_images.py -- do not edit by hand */\n\n")
        f.write("#ifndef IMAGES_H\n")
        f.write("#define IMAGES_H\n\n")
        f.write('#include "constants.h"\n\n')
        f.write(f"#define IMAGE_COLS     {img_cols}  /* SCREEN_WIDTH  * {multiplier} */\n")
        f.write(f"#define IMAGE_ROWS     {img_rows}  /* SCREEN_HEIGHT * {multiplier} */\n")
        f.write(f"#define IMAGE_SIZE     (IMAGE_COLS * IMAGE_ROWS)\n")
        f.write(f"#define MAX_PAN_X      {img_cols - SCREEN_WIDTH}   /* IMAGE_COLS - SCREEN_WIDTH  */\n")
        f.write(f"#define MAX_PAN_Y      {img_rows - SCREEN_HEIGHT}   /* IMAGE_ROWS - SCREEN_HEIGHT */\n")
        f.write(f"#define NUM_IMAGES     {num_images}\n\n")
        f.write("extern const unsigned char images[NUM_IMAGES][IMAGE_SIZE];\n")
        f.write("extern const char *image_names[NUM_IMAGES];\n")
        f.write("extern const unsigned char image_name_lengths[NUM_IMAGES];\n")
        f.write("extern const unsigned char image_pan_max_x[NUM_IMAGES];\n")
        f.write("extern const unsigned char image_pan_max_y[NUM_IMAGES];\n\n")
        f.write("#endif /* IMAGES_H */\n")


def write_source(output_dir: Path, image_paths: list[Path], all_images: list[list[int]], pan_limits: list[tuple[int, int]], multiplier: float = 1.0) -> None:
    img_cols = round(SCREEN_WIDTH * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)
    with open(output_dir / "images.c", "w", encoding="utf-8") as f:
        f.write("/* Auto-generated by convert_images.py -- do not edit by hand */\n\n")
        f.write('#include "images.h"\n\n')
        f.write("const unsigned char images[NUM_IMAGES][IMAGE_SIZE] =\n{\n")

        for i, chars in enumerate(all_images):
            f.write(f"    /* {i}: {image_paths[i].name} */\n")
            f.write("    {\n")
            for row in range(img_rows):
                row_data = chars[row * img_cols : (row + 1) * img_cols]
                line = ", ".join(str(v) for v in row_data)
                f.write(f"        {line},\n")
            separator = "," if i < len(all_images) - 1 else ""
            f.write(f"    }}{separator}\n")

        f.write("};\n\n")

        names = [to_galaksija_name(path.stem) for path in image_paths]

        f.write("const char *image_names[NUM_IMAGES] =\n{\n")
        for i, name in enumerate(names):
            # Escape backslash and double-quote for C string literals.
            # chr(92)=\ maps to Galaksija's Ć glyph; it must be written as \\ in C.
            c_name = name.replace("\\", "\\\\").replace('"', '\\"')
            separator = "," if i < len(names) - 1 else ""
            f.write(f'    "{c_name}"{separator}\n')
        f.write("};\n\n")

        f.write("const unsigned char image_name_lengths[NUM_IMAGES] =\n{\n")
        lengths = ", ".join(str(len(name)) for name in names)
        f.write(f"    {lengths}\n")
        f.write("};\n\n")

        f.write("const unsigned char image_pan_max_x[NUM_IMAGES] =\n{\n")
        f.write("    " + ", ".join(str(pmx) for pmx, _ in pan_limits) + "\n")
        f.write("};\n\n")

        f.write("const unsigned char image_pan_max_y[NUM_IMAGES] =\n{\n")
        f.write("    " + ", ".join(str(pmy) for _, pmy in pan_limits) + "\n")
        f.write("};\n")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert images to Galaksija slideshow C source files (images.h, images.c)."
    )
    parser.add_argument(
        "input_dir",
        metavar="INPUT_DIR",
        type=Path,
        help="directory containing input images",
    )
    parser.add_argument(
        "output_dir",
        metavar="OUTPUT_DIR",
        type=Path,
        nargs="?",
        default=Path("."),
        help="directory where images.h and images.c are written (default: current directory)",
    )
    parser.add_argument(
        "--preview-dir",
        metavar="DIR",
        type=Path,
        default=None,
        help="directory where galaksija_images/ is written (default: same as OUTPUT_DIR)",
    )
    parser.add_argument(
        "--dither",
        type=DitherMethod,
        choices=list(DitherMethod),
        default=DitherMethod.FLOYD_STEINBERG,
        help="dithering method (default: %(default)s)",
    )
    parser.add_argument(
        "--multiplier",
        type=float,
        default=1.5,
        metavar="N",
        help="image size multiplier for pan effect; images are stored at N x the screen "
             "resolution and the viewport pans from top-left to bottom-right (default: 1.5)",
    )
    args = parser.parse_args()

    if args.multiplier < 1.0:
        parser.error("--multiplier must be at least 1.0")

    image_paths = collect_images(args.input_dir)
    if not image_paths:
        parser.error(f"No supported image files found in '{args.input_dir}'")

    all_images = []
    pan_limits = []
    for path in image_paths:
        print(f"Converting {path.name}...")
        chars, bw_img, pan_max_x, pan_max_y = image_to_galaksija(path, args.dither, args.multiplier)
        all_images.append(chars)
        pan_limits.append((pan_max_x, pan_max_y))
        if args.preview_dir is not None:
            save_preview(bw_img, args.preview_dir, path.stem, pan_max_x, pan_max_y)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_header(args.output_dir, len(all_images), args.multiplier)
    write_source(args.output_dir, image_paths, all_images, pan_limits, args.multiplier)

    print(f"Done. Generated images.h, images.c, and galaksija_images/ with {len(all_images)} image(s).")


if __name__ == "__main__":
    main()
