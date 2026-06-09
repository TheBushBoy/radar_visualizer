"""Record reference metrics for non-regression testing.

Usage:
  python valid_reference.py --binary ./build/metrics_cli --data-dir /path/to/radar/
"""

import argparse
import json
from pathlib import Path

from metrics_common import REFERENCE_FILE, run_metrics_cli


def record(binary: Path, data_dir: Path) -> dict:
    """Compute and save aggregate reference metrics. Returns the saved dict."""
    stats = run_metrics_cli(binary, data_dir)
    REFERENCE_FILE.write_text(json.dumps(stats, indent=2))
    return stats


def main():
    parser = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--binary", default="./build/metrics_cli")
    parser.add_argument("--data-dir", required=True)
    args = parser.parse_args()

    stats = record(Path(args.binary), Path(args.data_dir))
    print(f"Saved reference ({stats['n_files']} files) to {REFERENCE_FILE}")


if __name__ == "__main__":
    main()
