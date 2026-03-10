"""
Shared utilities for Galaksija slideshow color preview generation.

Used by generate_gif.py and generate_video.py.
Constants and image-layout logic mirror convert_images.py and slideshow.c exactly.
"""

from pathlib import Path

from PIL import Image
from PIL.Image import Image as PILImage

from constants import SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_WIDTH, PIXEL_HEIGHT


def build_canvas(
    img_path: Path,
    multiplier: float,
    display_scale: int,
) -> tuple[PILImage, int, int]:
    """Scale the source image onto a canvas matching the Galaksija oversized grid.

    Mirrors the layout logic of convert_images.py (color, no dithering).
    Each Galaksija pixel maps to display_scale x display_scale output pixels.

    Returns:
        canvas    -- PIL RGB image of the full scaled canvas
        pan_max_x -- max pan offset in chars (X)
        pan_max_y -- max pan offset in chars (Y)
    """
    img_cols = round(SCREEN_WIDTH * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)
    gal_canvas_w = img_cols * 2
    gal_canvas_h = img_rows * 3

    img = Image.open(img_path).convert("RGB")
    src_w, src_h = img.size

    if multiplier <= 1.0:
        # Crop to 4:3 (Galaksija pixel aspect ratio), resize to exact screen size.
        ratio = PIXEL_WIDTH / PIXEL_HEIGHT
        cur_ratio = src_w / src_h
        if cur_ratio > ratio:
            new_w = int(src_h * ratio)
            left = (src_w - new_w) // 2
            img = img.crop((left, 0, left + new_w, src_h))
        elif cur_ratio < ratio:
            new_h = int(src_w / ratio)
            top = (src_h - new_h) // 2
            img = img.crop((0, top, src_w, top + new_h))
        disp_w = PIXEL_WIDTH * display_scale
        disp_h = PIXEL_HEIGHT * display_scale
        return img.resize((disp_w, disp_h), Image.LANCZOS), 0, 0

    # Scale to fit inside the oversized canvas, anchored at top-left.
    # Mirrors convert_images.py: scale = min(px_w / src_w, px_h / src_h)
    gal_scale = min(gal_canvas_w / src_w, gal_canvas_h / src_h)
    content_gal_w = round(src_w * gal_scale)
    content_gal_h = round(src_h * gal_scale)
    content_cols = content_gal_w // 2
    content_rows = content_gal_h // 3
    pan_max_x = max(0, content_cols - SCREEN_WIDTH)
    pan_max_y = max(0, content_rows - SCREEN_HEIGHT)

    canvas_disp_w = gal_canvas_w * display_scale
    canvas_disp_h = gal_canvas_h * display_scale
    content_disp_w = content_gal_w * display_scale
    content_disp_h = content_gal_h * display_scale

    img_scaled = img.resize((content_disp_w, content_disp_h), Image.LANCZOS)
    canvas = Image.new("RGB", (canvas_disp_w, canvas_disp_h), (0, 0, 0))
    canvas.paste(img_scaled, (0, 0))
    return canvas, pan_max_x, pan_max_y


def get_viewport(
    canvas: PILImage,
    pan_x: int,
    pan_y: int,
    display_scale: int,
) -> PILImage:
    """Crop the screen-sized viewport from canvas at the given pan offset (in chars)."""
    left = pan_x * 2 * display_scale
    top = pan_y * 3 * display_scale
    vp_w = PIXEL_WIDTH * display_scale
    vp_h = PIXEL_HEIGHT * display_scale
    return canvas.crop((left, top, left + vp_w, top + vp_h))
