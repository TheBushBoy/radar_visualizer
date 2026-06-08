#include <cstdlib>
#include <iostream>
#include <cmath>
#include "RadarParser.hpp"
#include "Metrics.hpp"

static constexpr std::string_view DATA_DIR = RADAR_DATA_DIR;
static const std::string TEST_PNG = std::string(DATA_DIR) + "1547131046353776.png";

static void check(bool cond, const char* msg) {
    if (!cond) { std::cerr << "FAIL: " << msg << "\n"; std::exit(1); }
}

static RadarScan load_scan() {
    RadarScan scan;
    if (!scan.load(TEST_PNG)) {
        std::cerr << "Could not load test PNG: " << scan.error() << "\n";
        std::exit(2);
    }
    return scan;
}

// compute() must return one AzimuthMetrics per azimuth row
static void test_per_azimuth_count() {
    RadarScan scan = load_scan();
    Metrics m;
    ScanMetrics result = m.compute(scan);
    check(result.per_azimuth.size() == scan.azimuths().size(),
        "per_azimuth size must match azimuth count");
    std::cout << "PASS: per_azimuth count = " << result.per_azimuth.size() << "\n";
}

// Noise floor is computed from normalised [0,1] power — must stay in that range
static void test_noise_floor_in_range() {
    RadarScan scan = load_scan();
    Metrics m;
    ScanMetrics result = m.compute(scan);
    for (const auto& az : result.per_azimuth)
        check(az.noise_floor >= 0.0f && az.noise_floor <= 1.0f,
            "noise_floor must be in [0.0, 1.0]");
    std::cout << "PASS: all noise floors in [0.0, 1.0]\n";
}

// peak_power >= noise_floor for every azimuth (peak is the max, noise is the median)
static void test_peak_above_noise() {
    RadarScan scan = load_scan();
    Metrics m;
    ScanMetrics result = m.compute(scan);
    for (const auto& az : result.per_azimuth)
        check(az.peak_power >= az.noise_floor,
              "peak_power must be >= noise_floor");
    std::cout << "PASS: peak >= noise for all azimuths\n";
}

// A real urban scan must have positive mean SNR (signal above noise floor)
static void test_mean_snr_positive() {
    RadarScan scan = load_scan();
    Metrics m;
    ScanMetrics result = m.compute(scan);
    check(result.mean_snr_db > 0.0f, "mean SNR must be positive for a real scan");
    std::cout << "PASS: mean_snr_db = " << result.mean_snr_db << " dB\n";
}

// Raising the anomaly thresholds beyond any real value must suppress all anomalies
static void test_no_anomaly_with_loose_config() {
    RadarScan scan = load_scan();
    Metrics::Config cfg;
    cfg.noise_floor_max = 1.0f; // max possible
    cfg.min_snr_db = -100.0f; // unreachable low threshold
    cfg.max_invalid_pct = 100; // allow all azimuths to be invalid
    Metrics m(cfg);
    ScanMetrics result = m.compute(scan);
    check(!result.anomaly_noise, "anomaly_noise must be false with loose config");
    check(!result.anomaly_snr, "anomaly_snr must be false with loose config");
    check(!result.anomaly_invalid, "anomaly_invalid must be false with loose config");
    std::cout << "PASS: no anomalies with loose thresholds\n";
}

int main() {
    test_per_azimuth_count();
    test_noise_floor_in_range();
    test_peak_above_noise();
    test_mean_snr_positive();
    test_no_anomaly_with_loose_config();
    std::cout << "All metrics tests passed.\n";
    return 0;
}
