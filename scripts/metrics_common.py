import json
import subprocess
from pathlib import Path

REFERENCE_FILE = Path(__file__).parent / "reference_metrics.json"


def run_metrics_cli(binary: Path, path: Path) -> dict:
    """Call metrics_cli on a single PNG or a directory. Returns parsed JSON."""
    result = subprocess.run(
        [str(binary), str(path)],
        capture_output=True, text=True, timeout=120,
    )
    if result.returncode != 0:
        raise RuntimeError(f"metrics_cli failed: {result.stderr.strip()}")
    return json.loads(result.stdout)
