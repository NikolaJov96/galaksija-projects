#!/usr/bin/env python3
"""
Generate a GIF that mirrors the Galaksija image slideshow using color source images.

The GIF replicates the exact viewport positions and panning behaviour of the C program:
  - Each image starts with the viewport at the top-left corner.
  - The viewport pans in PAN_STEPS steps toward the bottom-right corner.
  - Frame durations match the delay constants from slideshow.c.
"""

import argparse
from pathlib import Path

from PIL import Image

from preview_utils import (
    PAN_DELAY, PAN_STEPS, HOLD_DELAY, STATIC_DELAY,
    collect_images, build_canvas, get_viewport,
)


def make_frames(
    img_path: Path,
    multiplier: float,
    display_scale: int,
    ms_per_delay: float,
) -> list[tuple[Image.Image, int]]:
    """Return a list of (frame_image, duration_ms) pairs for one source image.

    Mirrors slideshow.c exactly:
      - multiplier > 1: 9 frames (pan steps 0-8); steps 0-7 each shown for
        PAN_DELAY delay units, step 8 held for HOLD_DELAY delay units.
      - multiplier == 1: 1 frame shown for STATIC_DELAY delay units.
    """
    canvas, pan_max_x, pan_max_y = build_canvas(img_path, multiplier, display_scale)

    if pan_max_x == 0 and pan_max_y == 0:
        frame = get_viewport(canvas, 0, 0, display_scale)
        return [(frame, round(STATIC_DELAY * ms_per_delay))]

    frames: list[tuple[Image.Image, int]] = []

    # Step 0: viewport at top-left, shown for PAN_DELAY units.
    frames.append((get_viewport(canvas, 0, 0, display_scale), round(PAN_DELAY * ms_per_delay)))

    # Steps 1..PAN_STEPS: pan linearly toward bottom-right.
    # Integer division matches the C expression:
    #   pan_x = pan_step * image_pan_max_x[i] / PAN_STEPS
    for pan_step in range(1, PAN_STEPS + 1):
        pan_x = pan_step * pan_max_x // PAN_STEPS
        pan_y = pan_step * pan_max_y // PAN_STEPS
        dur = round(HOLD_DELAY * ms_per_delay) if pan_step == PAN_STEPS else round(PAN_DELAY * ms_per_delay)
        frames.append((get_viewport(canvas, pan_x, pan_y, display_scale), dur))

    return frames


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate a color GIF that mirrors the Galaksija image slideshow."
    )
    parser.add_argument(
        "input_dir",
        metavar="INPUT_DIR",
        help="directory containing the color source images",
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
        help="display scale: each Galaksija pixel becomes NxN display pixels "
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
        print(f"  {path.name}...")
        for frame, dur in make_frames(path, args.multiplier, args.scale, args.ms_per_delay):
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
