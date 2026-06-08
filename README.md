# Radar Viewer for Testing and Validation

## Goal

The goal of this project is to demonstrate a simple radar data visualization application and compute a few metrics to perform testing and validation.

## Data

This project is based on the 10GB dataset from the [Oxford Radar RobotCar](https://oxford-robotics-institute.github.io/radar-robotcar-dataset/).

## External library

To efficiently parse the PNG files, the project uses the C++ *stb_image* library.

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