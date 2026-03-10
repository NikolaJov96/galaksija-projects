#!/usr/bin/env python3
"""
Generate a 30 fps video that simulates the Galaksija image slideshow with gradual
character-by-character screen rendering.

Unlike the GIF version (instantaneous viewport updates), this script models what actually
appears on the Galaksija CRT as display_window() writes characters one by one to video
memory in row-major order (left-to-right, top-to-bottom).

Timing model
------------
  render_time  : seconds for display_window() to write all 512 chars (default 3.0 s).
  ms_per_delay : milliseconds per slideshow delay unit (controls pan hold and final hold).

At 30 fps and 3 s render time each display_window() call produces 90 video frames, with
~5.7 color characters updated per frame.

Sequence per image (panning case, matching slideshow.c exactly):
  render(step 0)
  for pan_step = 1..PAN_STEPS:
      hold(PAN_DELAY delay units)   <- delay loop before next render
      render(step pan_step)         <- display_window() for new pan position
  hold(HOLD_DELAY delay units)      <- final hold before advancing

Output: MP4 via imageio/ffmpeg (install with: pip install imageio[ffmpeg]).
"""

import argparse
from pathlib import Path

import numpy as np
from PIL import Image

import imageio

from constants import (
    PIXEL_WIDTH, PIXEL_HEIGHT, TOTAL_CHARS,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    PAN_STEPS, PAN_DELAY, HOLD_DELAY, STATIC_DELAY,
)
from utils import collect_images
from preview_utils import build_canvas, get_viewport

FPS = 30


def composite(
    prev: np.ndarray,
    new: np.ndarray,
    chars_written: int,
    display_scale: int,
) -> np.ndarray:
    """Overlay the first chars_written characters from new onto prev.

    Characters are written in row-major order (cy = 0..15, cx = 0..31), matching
    the inner loops of display_window() in slideshow.c. Each character occupies a
    (2 * display_scale) x (3 * display_scale) pixel block.
    """
    result = prev.copy()
    char_w = 2 * display_scale
    char_h = 3 * display_scale
    remaining = min(chars_written, TOTAL_CHARS)
    for cy in range(SCREEN_HEIGHT):
        if remaining <= 0:
            break
        n = min(remaining, SCREEN_WIDTH)
        y0 = cy * char_h
        y1 = y0 + char_h
        x1 = n * char_w
        result[y0:y1, :x1] = new[y0:y1, :x1]
        remaining -= n
    return result


def generate_frames(
    image_paths: list[Path],
    multiplier: float,
    display_scale: int,
    render_time: float,
    ms_per_delay: float,
) -> list[np.ndarray]:
    """Build and return every video frame as a numpy RGB array."""
    vp_h = PIXEL_HEIGHT * display_scale
    vp_w = PIXEL_WIDTH * display_scale

    render_frames = max(1, round(render_time * FPS))
    chars_per_frame = TOTAL_CHARS / render_frames

    def hold_count(delay_units: int) -> int:
        return max(1, round(delay_units * ms_per_delay / 1000 * FPS))

    def emit_render(
        out: list[np.ndarray],
        prev: np.ndarray,
        new: np.ndarray,
    ) -> np.ndarray:
        """Append render_frames frames showing the gradual char-by-char transition."""
        new_arr = np.array(new, dtype=np.uint8)
        for fi in range(render_frames):
            n = min(TOTAL_CHARS, round((fi + 1) * chars_per_frame))
            out.append(composite(prev, new_arr, n, display_scale))
        return new_arr

    def emit_hold(out: list[np.ndarray], frame: np.ndarray, delay_units: int) -> None:
        for _ in range(hold_count(delay_units)):
            out.append(frame)

    # Galaksija starts with a clear (black) screen.
    prev: np.ndarray = np.zeros((vp_h, vp_w, 3), dtype=np.uint8)
    all_frames: list[np.ndarray] = []

    for img_path in image_paths:
        print(f"  {img_path.name}...")
        canvas, pan_max_x, pan_max_y = build_canvas(img_path, multiplier, display_scale)

        if pan_max_x == 0 and pan_max_y == 0:
            prev = emit_render(all_frames, prev, get_viewport(canvas, 0, 0, display_scale))
            emit_hold(all_frames, prev, STATIC_DELAY)
        else:
            # Step 0: render top-left viewport.
            prev = emit_render(all_frames, prev, get_viewport(canvas, 0, 0, display_scale))

            for pan_step in range(1, PAN_STEPS + 1):
                # The delay loop in slideshow.c runs BEFORE the new pan position
                # is calculated and rendered.
                emit_hold(all_frames, prev, PAN_DELAY)
                pan_x = pan_step * pan_max_x // PAN_STEPS
                pan_y = pan_step * pan_max_y // PAN_STEPS
                prev = emit_render(all_frames, prev, get_viewport(canvas, pan_x, pan_y, display_scale))

            emit_hold(all_frames, prev, HOLD_DELAY)

    return all_frames


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate a 30 fps video simulating Galaksija character-by-character rendering."
    )
    parser.add_argument(
        "input_dir",
        metavar="INPUT_DIR",
        type=Path,
        help="directory containing the color source images",
    )
    parser.add_argument(
        "output",
        metavar="OUTPUT.mp4",
        type=Path,
        help="output video file (MP4 recommended; extension determines format)",
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
             "(default: 8, giving a 512x384 video)",
    )
    parser.add_argument(
        "--render-time",
        type=float,
        default=3.0,
        metavar="S",
        help="seconds for display_window() to write all 512 characters (default: 3.0)",
    )
    parser.add_argument(
        "--ms-per-delay",
        type=float,
        default=50.0,
        metavar="MS",
        help="milliseconds per slideshow delay unit - controls pan hold and final hold "
             "(default: 50, giving 750 ms between pan steps and 2 s final hold)",
    )
    args = parser.parse_args()

    if args.multiplier < 1.0:
        parser.error("--multiplier must be at least 1.0")
    if args.scale < 1:
        parser.error("--scale must be at least 1")

    image_paths = collect_images(args.input_dir)
    if not image_paths:
        parser.error(f"No supported images found in '{args.input_dir}'")

    render_frames = max(1, round(args.render_time * FPS))
    print(
        f"Settings: {len(image_paths)} image(s), render_time={args.render_time} s "
        f"({render_frames} frames/render), ms_per_delay={args.ms_per_delay} ms, "
        f"scale={args.scale}x -> {PIXEL_WIDTH * args.scale}x{PIXEL_HEIGHT * args.scale} px"
    )

    print("Generating frames...")
    frames = generate_frames(
        image_paths,
        multiplier=args.multiplier,
        display_scale=args.scale,
        render_time=args.render_time,
        ms_per_delay=args.ms_per_delay,
    )

    print(f"Writing {args.output} ({len(frames)} frames at {FPS} fps = {len(frames)/FPS:.1f} s)...")
    with imageio.get_writer(str(args.output), fps=FPS, codec="libx264", quality=8) as writer:
        for frame in frames:
            writer.append_data(frame)

    print("Done.")


if __name__ == "__main__":
    main()
