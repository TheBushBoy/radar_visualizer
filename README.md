# Radar Viewer for Testing and Validation

## Goal

The goal of this project is to demonstrate a simple radar data visualization application and compute a few metrics to perform testing and validation.

## Data

This project is based on the 10GB dataset from the [Oxford Radar RobotCar](https://oxford-robotics-institute.github.io/radar-robotcar-dataset/).

It contains PNG files. Each file encodes one full rotation, 400 rows azimuths per 3779 columns, with 11 bytes of metadata and 3768 range bins. For each azimuth we have the UNIX timestamp, the angle in radians derived from the encoder counter (5600 ticks/revolution), a validity flag, and the normalised power values between 0.0 and 1.0 for all range bins.

## External library

To efficiently parse the PNG files, the project uses the C++ *stb_image* library.

## Architecture

### Processing module

The processing module is a standalone static library with no dependency on Qt. It contains three components:

- `RadarParser`, loads a radar PNG from the Oxford dataset. 

- `Metrics`, computes per-scan statistics from a loaded `RadarScan`: noise floor (median of range bins), peak power, SNR in dB and anomaly flags for excessive noise, low SNR, or too many interpolated azimuths. Thresholds are configurable via `Metrics::Config`.

### GUI module

The GUI is a Qt6/QML application built on top of the processing library. It is split into a C++ backend and a QML front-end:

- `RadarBackend`, when `openFolder()` is called it lists all PNG files instantly, then a persistent worker thread loads scans on demand. Navigation updates a sliding window cache of `WIDTH_OF_PRESCAN` scans on each side of the current index, out-of-window entries are evicted, missing ones are queued in priority order, current index first, then forward before backward. Rapid navigation replaces the queue so stale work is never accumulated.

- `Main.qml`, defines the window layout: a top bar with an *Open folder* button and navigation arrows, a left panel for the PPI display, a right panel for metrics, and a footer showing the current scan name or *Loading…* during fast navigation.

## Python scripts

The scripts directory contains tools for non-regression testing. They compare aggregate metrics between two radar datasets.

Install dependencies:
```bash
cd scripts && .venv/bin/pip install -r requirements.txt
```

- `valid_reference.py`, runs `metrics_cli` on a reference dataset and saves the aggregate statistics to `reference_metrics.json`.
```bash
python scripts/valid_reference.py --data-dir /path/to/reference/
```

- `test_non_regression.py`, runs `metrics_cli` on a new dataset and compares the aggregate statistics against the saved reference. Exits with code 1 if any metric regresses beyond the tolerance (default 5%).
```bash
python scripts/test_non_regression.py --data-dir /path/to/new_dataset/
```

Both scripts can also be triggered from the GUI via the *Record ref.* and *Non-regression* buttons in the top bar.

## Build & Test

```bash
cmake .. -DRADAR_DATA_DIR="/path/to/radar-oxford/radar/"
make
```

Run the application
```bash
./RadarVisualizer
```

Run the tests
```bash
ctest
```