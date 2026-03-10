#!/usr/bin/env python3
"""
Generate a GIF that mirrors the Galaksija image slideshow using full-resolution color images.

The GIF replicates the exact viewport positions and panning behaviour of the C program:
  - Each image starts with the viewport at the top-left corner.
  - The viewport pans in PAN_STEPS steps toward the bottom-right corner.
  - Frame durations match the delay constants from slideshow.c.

Usage:
    python generate_gif.py full_original_images/slideshow_color.gif
    python generate_gif.py full_original_images/slideshow_color.gif --scale 4 --ms-per-delay 40
"""

import argparse
from pathlib import Path

from PIL import Image

# ---------------------------------------------------------------------------
# Constants matching slideshow.c and convert_images.py
# ---------------------------------------------------------------------------
SCREEN_WIDTH  = 32   # character columns
SCREEN_HEIGHT = 16   # character rows
PIXEL_WIDTH   = SCREEN_WIDTH  * 2   # 64  (Galaksija pixels)
PIXEL_HEIGHT  = SCREEN_HEIGHT * 3   # 48  (Galaksija pixels)

PAN_STEPS    = 8
PAN_DELAY    = 15   # delay units between pan steps (frames 0..7 are each shown this long)
HOLD_DELAY   = 40   # delay units to hold the final frame before advancing
STATIC_DELAY = (PAN_STEPS + 1) * PAN_DELAY  # total delay for non-panning images

SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff", ".webp"}


def collect_images(input_dir: Path) -> list[Path]:
    return sorted(
        f for f in input_dir.iterdir()
        if f.suffix.lower() in SUPPORTED_EXTENSIONS
    )


def make_frames(
    img_path: Path,
    multiplier: float,
    display_scale: int,
    ms_per_delay: float,
) -> list[tuple[Image.Image, int]]:
    """Return a list of (frame_image, duration_ms) pairs for one source image.

    The logic mirrors slideshow.c exactly:
      - multiplier > 1: 9 frames (pan steps 0-8); steps 0-7 each shown for
        PAN_DELAY delay units, step 8 held for HOLD_DELAY delay units.
      - multiplier == 1: 1 frame shown for STATIC_DELAY delay units.
    """
    img_cols = round(SCREEN_WIDTH  * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)

    # Canvas size in Galaksija pixels (stored-image resolution)
    gal_canvas_w = img_cols * 2
    gal_canvas_h = img_rows * 3

    # Viewport size in Galaksija pixels (what the screen shows)
    gal_vp_w = PIXEL_WIDTH   # 64
    gal_vp_h = PIXEL_HEIGHT  # 48

    img = Image.open(img_path).convert("RGB")
    src_w, src_h = img.size

    if multiplier <= 1.0:
        # Crop to the Galaksija 4:3 aspect ratio, then resize to the viewport.
        ratio = gal_vp_w / gal_vp_h
        cur_ratio = src_w / src_h
        if cur_ratio > ratio:
            new_w = int(src_h * ratio)
            left = (src_w - new_w) // 2
            img = img.crop((left, 0, left + new_w, src_h))
        elif cur_ratio < ratio:
            new_h = int(src_w / ratio)
            top  = (src_h - new_h) // 2
            img = img.crop((0, top, src_w, top + new_h))

        disp_w = gal_vp_w * display_scale
        disp_h = gal_vp_h * display_scale
        frame  = img.resize((disp_w, disp_h), Image.LANCZOS)
        return [(frame, round(STATIC_DELAY * ms_per_delay))]

    # --- panning case (multiplier > 1) ---

    # Scale the source image so it fits inside the Galaksija canvas, anchored top-left.
    # Compute content dimensions in Galaksija pixels (mirrors convert_images.py).
    gal_scale     = min(gal_canvas_w / src_w, gal_canvas_h / src_h)
    content_gal_w = round(src_w * gal_scale)
    content_gal_h = round(src_h * gal_scale)

    # Pan limits in character units (mirrors convert_images.py exactly).
    content_cols = content_gal_w // 2
    content_rows = content_gal_h // 3
    pan_max_x = max(0, content_cols - SCREEN_WIDTH)
    pan_max_y = max(0, content_rows - SCREEN_HEIGHT)

    # Build a high-res canvas with the scaled color image pasted at (0, 0).
    canvas_disp_w = gal_canvas_w * display_scale
    canvas_disp_h = gal_canvas_h * display_scale
    content_disp_w = content_gal_w * display_scale
    content_disp_h = content_gal_h * display_scale
    vp_disp_w = gal_vp_w * display_scale
    vp_disp_h = gal_vp_h * display_scale

    img_scaled = img.resize((content_disp_w, content_disp_h), Image.LANCZOS)
    canvas = Image.new("RGB", (canvas_disp_w, canvas_disp_h), (0, 0, 0))
    canvas.paste(img_scaled, (0, 0))

    def crop_viewport(pan_x: int, pan_y: int) -> Image.Image:
        """Crop the viewport at the given pan offset (in chars)."""
        left = pan_x * 2 * display_scale
        top  = pan_y * 3 * display_scale
        return canvas.crop((left, top, left + vp_disp_w, top + vp_disp_h))

    frames: list[tuple[Image.Image, int]] = []

    # Step 0: viewport at top-left, shown for PAN_DELAY units.
    frames.append((crop_viewport(0, 0), round(PAN_DELAY * ms_per_delay)))

    # Steps 1..PAN_STEPS: pan linearly toward bottom-right.
    # The integer division matches the C expression:
    #   pan_x = pan_step * image_pan_max_x[i] / PAN_STEPS
    for pan_step in range(1, PAN_STEPS + 1):
        pan_x = pan_step * pan_max_x // PAN_STEPS
        pan_y = pan_step * pan_max_y // PAN_STEPS
        if pan_step < PAN_STEPS:
            dur = round(PAN_DELAY * ms_per_delay)
        else:
            # Final frame is held for HOLD_DELAY before advancing to the next image.
            dur = round(HOLD_DELAY * ms_per_delay)
        frames.append((crop_viewport(pan_x, pan_y), dur))

    return frames


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate a color GIF that mirrors the Galaksija image slideshow."
    )
    parser.add_argument(
        "input_dir",
        metavar="INPUT_DIR",
        help="directory containing the full-resolution color source images",
    )
    parser.add_argument(
        "output",
        metavar="OUTPUT.gif",
        help="path for the output GIF file",
    )
    parser.add_argument(
        "--multiplier",
        type=float,
        default=1.5,
        metavar="N",
        help="same multiplier used when building the slideshow (default: 1.5)",
    )
    parser.add_argument(
        "--scale",
        type=int,
        default=8,
        metavar="N",
        help="display scale factor: each Galaksija pixel becomes NxN display pixels "
             "(default: 8, giving a 512x384 GIF)",
    )
    parser.add_argument(
        "--ms-per-delay",
        type=float,
        default=50.0,
        metavar="MS",
        help="milliseconds per slideshow delay unit - tune to match real hardware speed "
             "(default: 50, giving ~8.75 s per image at PAN_STEPS=8)",
    )
    parser.add_argument(
        "--no-loop",
        action="store_true",
        help="play the GIF once instead of looping",
    )
    args = parser.parse_args()

    if args.multiplier < 1.0:
        parser.error("--multiplier must be at least 1.0")
    if args.scale < 1:
        parser.error("--scale must be at least 1")

    input_dir = Path(args.input_dir)
    image_paths = collect_images(input_dir)
    if not image_paths:
        parser.error(f"No supported images found in '{input_dir}'")

    all_frames: list[Image.Image] = []
    all_durations: list[int] = []

    for path in image_paths:
        print(f"Processing {path.name}...")
        frames = make_frames(path, args.multiplier, args.scale, args.ms_per_delay)
        for frame, dur in frames:
            all_frames.append(frame)
            all_durations.append(dur)

    output_path = Path(args.output)
    print(f"Saving {output_path} ({len(all_frames)} frames)...")
    all_frames[0].save(
        output_path,
        save_all=True,
        append_images=all_frames[1:],
        duration=all_durations,
        loop=0 if not args.no_loop else 1,
        optimize=False,
    )
    print(f"Done. {output_path} - {len(image_paths)} image(s), {len(all_frames)} frames total.")


if __name__ == "__main__":
    main()
