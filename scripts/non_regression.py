"""Record reference metrics for non-regression testing.

Usage:
  python non_regression.py --binary ./build/metrics_cli --data-dir /path/to/radar/
"""

import argparse
import json
import subprocess
from pathlib import Path

REFERENCE_FILE = Path(__file__).parent / "reference_metrics.json"


def run_metrics_cli(binary: Path, png: Path) -> dict:
    result = subprocess.run(
        [str(binary), str(png)],
        capture_output=True, text=True, timeout=30,
    )
    if result.returncode != 0:
        raise RuntimeError(f"metrics_cli failed on {png.name}: {result.stderr.strip()}")
    return json.loads(result.stdout)


def collect_metrics(binary: Path, data_dir: Path) -> dict:
    pngs = sorted(data_dir.glob("*.png"))
    if not pngs:
        raise FileNotFoundError(f"No PNG files found in {data_dir}")
    metrics = {}
    for i, png in enumerate(pngs):
        print(f"  [{i+1}/{len(pngs)}] {png.name}", end="\r")
        metrics[png.name] = run_metrics_cli(binary, png)
    print()
    return metrics


def main():
    parser = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--binary", default="./build/metrics_cli", help="Path to metrics_cli executable")
    parser.add_argument("--data-dir", required=True, help="Directory containing radar PNGs")
    args = parser.parse_args()

    print(f"Recording reference metrics from {args.data_dir}")
    metrics = collect_metrics(Path(args.binary), Path(args.data_dir))
    REFERENCE_FILE.write_text(json.dumps(metrics, indent=2))
    print(f"Saved {len(metrics)} file(s) to {REFERENCE_FILE}")


if __name__ == "__main__":
    main()
