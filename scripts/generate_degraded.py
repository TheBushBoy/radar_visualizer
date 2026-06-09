#!/usr/bin/env python3
"""Generate degraded radar PNG files for testing."""

import argparse
import sys
from pathlib import Path

import numpy as np
from PIL import Image

AZIMUTHS = 400
META_COLS = 11
RANGE_BINS = 3768
PNG_COLS = 3779
DEFAULT_DATA_DIR = "/home/theo/Downloads/radar-oxford/radar/"


def load_scan(path: Path) -> np.ndarray:
    img = np.array(Image.open(path))
    assert img.shape == (AZIMUTHS, PNG_COLS), (
        f"Unexpected shape {img.shape}, expected ({AZIMUTHS}, {PNG_COLS})"
    )
    return img


def save_scan(img: np.ndarray, path: Path) -> None:
    Image.fromarray(img, mode="L").save(path)


def add_noise(img: np.ndarray, offset: int = 60) -> np.ndarray:
    """Raise the noise floor."""
    out = img.copy()
    bins = out[:, META_COLS:].astype(np.int16) + offset
    out[:, META_COLS:] = np.clip(bins, 0, 255).astype(np.uint8)
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-dir", default=DEFAULT_DATA_DIR,
                        help="Directory with original radar PNGs")
    parser.add_argument("--output-dir", default=DEFAULT_DATA_DIR + "noisy/",
                        help="Directory to write degraded PNGs")
    args = parser.parse_args()

    data_dir = Path(args.data_dir)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    pngs = sorted(data_dir.glob("*.png"))
    if not pngs:
        print(f"ERROR: no PNG files found in {data_dir}", file=sys.stderr)
        return 1

    for i, src in enumerate(pngs):
        out_path = out_dir / f"{src.stem}_noisy{src.suffix}"
        save_scan(add_noise(load_scan(src)), out_path)
        print(f"[{i+1}/{len(pngs)}] {out_path.name}", end="\r")

    print(f"\n{len(pngs)} files written to {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
