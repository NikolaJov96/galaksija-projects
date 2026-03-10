#!/usr/bin/env python3
"""
Generate a 30 fps video that simulates the Galaksija image slideshow with gradual
character-by-character screen rendering.

Unlike the GIF version (instantaneous viewport updates), this script models what actually
appears on the Galaksija CRT as display_window() writes characters one by one to video
memory in row-major order (left-to-right, top-to-bottom).

Timing model
------------
  render_time   : seconds for display_window() to write all 512 chars (default 3.0 s).
  ms_per_delay  : milliseconds per slideshow delay unit (controls pan hold and final hold).

At 30 fps and 3 s render time each display_window() call produces 90 video frames, with
~5.7 color characters updated per frame.

Sequence per image (panning case, matching slideshow.c exactly):
  render(step 0)
  for pan_step = 1..PAN_STEPS:
      hold(PAN_DELAY delay units)          <- delay loop before next render
      render(step pan_step)                <- display_window() for new pan position
  hold(HOLD_DELAY delay units)            <- final hold before advancing

Output: MP4 via imageio/ffmpeg (install with: pip install imageio[ffmpeg]).
"""

import argparse
from pathlib import Path

import numpy as np
from PIL import Image

import imageio

# ---------------------------------------------------------------------------
# Constants matching slideshow.c and convert_images.py
# ---------------------------------------------------------------------------
FPS           = 30
SCREEN_WIDTH  = 32
SCREEN_HEIGHT = 16
TOTAL_CHARS   = SCREEN_WIDTH * SCREEN_HEIGHT  # 512
PIXEL_WIDTH   = SCREEN_WIDTH  * 2             # 64
PIXEL_HEIGHT  = SCREEN_HEIGHT * 3             # 48

PAN_STEPS    = 8
PAN_DELAY    = 15
HOLD_DELAY   = 40
STATIC_DELAY = (PAN_STEPS + 1) * PAN_DELAY

SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff", ".webp"}


def collect_images(input_dir: Path) -> list[Path]:
    return sorted(
        f for f in input_dir.iterdir()
        if f.suffix.lower() in SUPPORTED_EXTENSIONS
    )


def build_canvas(
    img_path: Path,
    multiplier: float,
    display_scale: int,
) -> tuple[np.ndarray, int, int]:
    """Scale the source image onto a canvas matching the Galaksija oversized grid.

    Returns:
        canvas     -- numpy RGB array of the full canvas
        pan_max_x  -- max pan offset in chars (X)
        pan_max_y  -- max pan offset in chars (Y)
    """
    img_cols = round(SCREEN_WIDTH  * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)
    gal_canvas_w = img_cols * 2
    gal_canvas_h = img_rows * 3

    img = Image.open(img_path).convert("RGB")
    src_w, src_h = img.size

    if multiplier <= 1.0:
        # Crop to 4:3 (Galaksija's pixel aspect ratio), resize to exact screen size.
        ratio = PIXEL_WIDTH / PIXEL_HEIGHT
        cur_ratio = src_w / src_h
        if cur_ratio > ratio:
            new_w = int(src_h * ratio)
            left  = (src_w - new_w) // 2
            img   = img.crop((left, 0, left + new_w, src_h))
        elif cur_ratio < ratio:
            new_h = int(src_w / ratio)
            top   = (src_h - new_h) // 2
            img   = img.crop((0, top, src_w, top + new_h))
        disp_w = PIXEL_WIDTH  * display_scale
        disp_h = PIXEL_HEIGHT * display_scale
        canvas = np.array(img.resize((disp_w, disp_h), Image.LANCZOS), dtype=np.uint8)
        return canvas, 0, 0

    # Scale to fit inside the oversized canvas (mirrors convert_images.py).
    gal_scale     = min(gal_canvas_w / src_w, gal_canvas_h / src_h)
    content_gal_w = round(src_w * gal_scale)
    content_gal_h = round(src_h * gal_scale)
    content_cols  = content_gal_w // 2
    content_rows  = content_gal_h // 3
    pan_max_x = max(0, content_cols - SCREEN_WIDTH)
    pan_max_y = max(0, content_rows - SCREEN_HEIGHT)

    content_disp_w = content_gal_w * display_scale
    content_disp_h = content_gal_h * display_scale
    canvas_disp_w  = gal_canvas_w  * display_scale
    canvas_disp_h  = gal_canvas_h  * display_scale

    img_scaled = img.resize((content_disp_w, content_disp_h), Image.LANCZOS)
    canvas_img = Image.new("RGB", (canvas_disp_w, canvas_disp_h), (0, 0, 0))
    canvas_img.paste(img_scaled, (0, 0))
    return np.array(canvas_img, dtype=np.uint8), pan_max_x, pan_max_y


def get_viewport(canvas: np.ndarray, pan_x: int, pan_y: int, display_scale: int) -> np.ndarray:
    """Crop the viewport at the given pan offset (in chars)."""
    left = pan_x * 2 * display_scale
    top  = pan_y * 3 * display_scale
    vp_w = PIXEL_WIDTH  * display_scale
    vp_h = PIXEL_HEIGHT * display_scale
    return canvas[top:top + vp_h, left:left + vp_w].copy()


def composite(
    prev:         np.ndarray,
    new:          np.ndarray,
    chars_written: int,
    display_scale: int,
) -> np.ndarray:
    """Overlay the first `chars_written` characters from `new` onto `prev`.

    Characters are written in row-major order (cy = 0..15, cx = 0..31), matching
    the inner loops of display_window() in slideshow.c.  Each character occupies a
    (2 * display_scale) x (3 * display_scale) pixel block.
    """
    result = prev.copy()
    char_w = 2 * display_scale
    char_h = 3 * display_scale
    remaining = min(chars_written, TOTAL_CHARS)
    for cy in range(SCREEN_HEIGHT):
        if remaining <= 0:
            break
        n    = min(remaining, SCREEN_WIDTH)
        y0   = cy * char_h
        y1   = y0 + char_h
        x1   = n  * char_w
        result[y0:y1, :x1] = new[y0:y1, :x1]
        remaining -= n
    return result


def generate_frames(
    image_paths:   list[Path],
    multiplier:    float,
    display_scale: int,
    render_time:   float,
    ms_per_delay:  float,
) -> list[np.ndarray]:
    """Build and return every video frame as a numpy RGB array."""
    vp_h = PIXEL_HEIGHT * display_scale
    vp_w = PIXEL_WIDTH  * display_scale

    # Number of video frames that display_window() spans.
    render_frames   = max(1, round(render_time * FPS))
    chars_per_frame = TOTAL_CHARS / render_frames

    def hold_frames(delay_units: int) -> int:
        return max(1, round(delay_units * ms_per_delay / 1000 * FPS))

    def emit_render(
        frames_out: list[np.ndarray],
        prev:       np.ndarray,
        new:        np.ndarray,
    ) -> np.ndarray:
        """Append render_frames frames showing the gradual char-by-char transition."""
        for fi in range(render_frames):
            n     = min(TOTAL_CHARS, round((fi + 1) * chars_per_frame))
            frame = composite(prev, new, n, display_scale)
            frames_out.append(frame)
        return new.copy()

    def emit_hold(
        frames_out: list[np.ndarray],
        frame:      np.ndarray,
        delay_units: int,
    ) -> None:
        n = hold_frames(delay_units)
        for _ in range(n):
            frames_out.append(frame)

    # Galaksija starts with a clear (black) screen.
    prev = np.zeros((vp_h, vp_w, 3), dtype=np.uint8)
    all_frames: list[np.ndarray] = []

    for img_path in image_paths:
        print(f"  {img_path.name}...")
        canvas, pan_max_x, pan_max_y = build_canvas(img_path, multiplier, display_scale)

        if pan_max_x == 0 and pan_max_y == 0:
            # No pan: one render, then hold for STATIC_DELAY.
            new_vp = get_viewport(canvas, 0, 0, display_scale)
            prev   = emit_render(all_frames, prev, new_vp)
            emit_hold(all_frames, prev, STATIC_DELAY)
        else:
            # Step 0: render top-left viewport.
            new_vp = get_viewport(canvas, 0, 0, display_scale)
            prev   = emit_render(all_frames, prev, new_vp)

            for pan_step in range(1, PAN_STEPS + 1):
                # Hold current frame for PAN_DELAY units (the delay loop in slideshow.c
                # runs BEFORE the new pan position is calculated and rendered).
                emit_hold(all_frames, prev, PAN_DELAY)

                pan_x  = pan_step * pan_max_x // PAN_STEPS
                pan_y  = pan_step * pan_max_y // PAN_STEPS
                new_vp = get_viewport(canvas, pan_x, pan_y, display_scale)
                prev   = emit_render(all_frames, prev, new_vp)

            # Hold the final frame.
            emit_hold(all_frames, prev, HOLD_DELAY)

    return all_frames


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate a 30 fps video simulating Galaksija character-by-character rendering."
    )
    parser.add_argument(
        "input_dir",
        metavar="INPUT_DIR",
        help="directory containing the full-resolution color source images",
    )
    parser.add_argument(
        "output",
        metavar="OUTPUT.mp4",
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

    input_dir   = Path(args.input_dir)
    image_paths = collect_images(input_dir)
    if not image_paths:
        parser.error(f"No supported images found in '{input_dir}'")

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

    output_path = Path(args.output)
    print(f"Writing {output_path}  ({len(frames)} frames at {FPS} fps = {len(frames)/FPS:.1f} s)...")
    with imageio.get_writer(str(output_path), fps=FPS, codec="libx264", quality=8) as writer:
        for frame in frames:
            writer.append_data(frame)

    print("Done.")


if __name__ == "__main__":
    main()
