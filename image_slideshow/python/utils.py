"""
Common utilities shared by all Galaksija slideshow Python scripts.
"""

from pathlib import Path


SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tiff", ".webp"}


def collect_images(input_dir: Path) -> list[Path]:
    """Return a sorted list of supported image paths in input_dir."""
    return sorted(
        f for f in input_dir.iterdir()
        if f.suffix.lower() in SUPPORTED_EXTENSIONS
    )
