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
VALID_FLAG_COL = 10
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


def mark_invalid(img: np.ndarray, pct: float = 10) -> np.ndarray:
    """Mark a percentage of azimuths as interpolated."""
    out = img.copy()
    n = int(AZIMUTHS * pct / 100)
    rows = np.random.choice(AZIMUTHS, n, replace=False)
    out[rows, VALID_FLAG_COL] = 0
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-dir", default=DEFAULT_DATA_DIR,
                        help="Directory with original radar PNGs")
    parser.add_argument("--output-dir", default=DEFAULT_DATA_DIR + "degraded/",
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
        ref = load_scan(src)
        save_scan(add_noise(ref), out_dir / f"{src.stem}_noisy{src.suffix}")
        save_scan(mark_invalid(ref), out_dir / f"{src.stem}_invalid{src.suffix}")
        print(f"[{i+1}/{len(pngs)}] {src.name}", end="\r")

    print(f"\n{len(pngs)} files written to {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
