"""Non-regression test between two radar datasets.

Usage:
  python test_non_regression.py --data-dir /path/to/new_dataset/
  python test_non_regression.py --data-dir /path/to/new_dataset/ --tolerance 10.0
"""

import argparse
import json
import sys
from pathlib import Path

from metrics_common import REFERENCE_FILE, run_metrics_cli

FLOAT_KEYS = ("mean_noise_floor", "mean_snr_db", "mean_invalid_azimuths")
PCT_KEYS   = ("anomaly_noise_pct", "anomaly_snr_pct", "anomaly_invalid_pct")


def compare(current: dict, reference: dict, tol: float) -> dict:
    """Compare aggregate stats."""
    results = {}
    for key in FLOAT_KEYS:
        ref_val, cur_val = reference[key], current[key]
        rel = abs(cur_val - ref_val) / abs(ref_val) if abs(ref_val) > 1e-9 else abs(cur_val - ref_val)
        results[key] = {"pass": rel <= tol, "ref": ref_val, "cur": cur_val, "delta_pct": rel * 100}
    for key in PCT_KEYS:
        ref_val, cur_val = reference[key], current[key]
        delta = abs(cur_val - ref_val)
        results[key] = {"pass": delta <= tol * 100, "ref": ref_val, "cur": cur_val, "delta_pct": delta}
    return results


def run(binary: Path, data_dir: Path, tolerance: float = 5.0) -> dict:
    """Run non-regression tests and return results dict."""
    reference = json.loads(REFERENCE_FILE.read_text())
    current = run_metrics_cli(binary, data_dir)
    results = compare(current, reference, tol=tolerance / 100.0)
    return {"results": results, "current_stats": current, "ref_stats": reference}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", default="./build/metrics_cli")
    parser.add_argument("--data-dir", required=True)
    parser.add_argument("--tolerance", type=float, default=5.0)
    args = parser.parse_args()

    if not REFERENCE_FILE.exists():
        sys.exit(f"ERROR: no reference file. Run valid_reference.py first.")

    output = run(Path(args.binary), Path(args.data_dir), args.tolerance)
    results = output["results"]
    passed = sum(1 for r in results.values() if r["pass"])
    total = len(results)
    for metric, r in results.items():
        print(f"  [{'PASS' if r['pass'] else 'FAIL'}] {metric:<26} ref={r['ref']:.4f} cur={r['cur']:.4f} delta={r['delta_pct']:.1f}%")
    print(f"\n{passed}/{total} passed")
    if passed < total:
        sys.exit(1)


if __name__ == "__main__":
    main()
