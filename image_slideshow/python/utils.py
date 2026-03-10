"""
Common utilities shared by all Galaksija slideshow Python scripts.
"""

from pathlib import Path

from PIL import Image
from PIL.Image import Image as PILImage

from constants import SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_WIDTH, PIXEL_HEIGHT


SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff", ".webp"}


def collect_images(input_dir: Path) -> list[Path]:
    """Return a sorted list of supported image paths in input_dir."""
    return sorted(
        f for f in input_dir.iterdir()
        if f.suffix.lower() in SUPPORTED_EXTENSIONS
    )


def load_scaled_image(img_path: Path, multiplier: float, scale: int = 1) -> tuple[PILImage, int, int]:
    """Open and scale an image to the Galaksija canvas grid.

    The result is sized at (img_cols * 2 * scale) x (img_rows * 3 * scale) pixels.
    Use scale=1 (default) for Galaksija pixel coordinates (used by convert_images.py).
    Use scale=display_scale for direct display-resolution output (used by preview_utils.py),
    which avoids a quality-degrading two-step resize.

    multiplier <= 1.0: center-crop to Galaksija aspect ratio, resize to exact
                       screen size; pan limits are both 0.
    multiplier > 1.0:  scale to fit within the oversized grid, anchor at top-left;
                       remaining area is black.

    Returns:
        img       -- RGB PIL image
        pan_max_x -- max pan offset in X (chars)
        pan_max_y -- max pan offset in Y (chars)
    """
    img_cols = round(SCREEN_WIDTH * multiplier)
    img_rows = round(SCREEN_HEIGHT * multiplier)
    px_w = img_cols * 2 * scale
    px_h = img_rows * 3 * scale

    img = Image.open(img_path).convert("RGB")
    src_w, src_h = img.size

    if multiplier <= 1.0:
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
        return img.resize((px_w, px_h), Image.LANCZOS), 0, 0

    # Scale to fit entirely within the oversized grid, anchor at top-left.
    fit_scale = min(px_w / src_w, px_h / src_h)
    content_px_w = round(src_w * fit_scale)
    content_px_h = round(src_h * fit_scale)
    content_cols = content_px_w // (2 * scale)
    content_rows = content_px_h // (3 * scale)
    pan_max_x = max(0, content_cols - SCREEN_WIDTH)
    pan_max_y = max(0, content_rows - SCREEN_HEIGHT)
    img_scaled = img.resize((content_px_w, content_px_h), Image.LANCZOS)
    canvas = Image.new("RGB", (px_w, px_h), (0, 0, 0))
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
