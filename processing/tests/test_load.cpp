#include <cmath>
#include <cstdlib>
#include <iostream>
#include "RadarParser.hpp"

static constexpr std::string_view DATA_DIR = RADAR_DATA_DIR;
static const std::string TEST_PNG = std::string(DATA_DIR) + "1547131046353776.png";

static void check(bool cond, const char* msg) {
    if (!cond) { std::cerr << "FAIL: " << msg << "\n"; std::exit(1); }
}

// load() must return false and expose a non-empty error message for bad paths
static void test_invalid_path() {
    RadarScan scan;
    bool ok = scan.load("/nonexistent/path/file.png");
    check(!ok, "load() must return false for a missing file");
    check(!scan.error().empty(), "error() must not be empty after failure");
    std::cout << "PASS: invalid path rejected\n";
}

// A valid PNG must load without error
static void test_load_succeeds() {
    RadarScan scan;
    bool ok = scan.load(TEST_PNG);
    check(ok, "load() must return true for a valid radar PNG");
    std::cout << "PASS: valid PNG loaded\n";
}

// Exactly 400 azimuths per rotation
static void test_azimuth_count() {
    RadarScan scan;
    scan.load(TEST_PNG);
    check(static_cast<int>(scan.azimuths().size()) == RADAR_AZIMUTHS,
        "Expected 400 azimuths");
    std::cout << "PASS: azimuth count = " << scan.azimuths().size() << "\n";
}

// Each azimuth row must have exactly 3768 range bins
static void test_range_bin_count() {
    RadarScan scan;
    scan.load(TEST_PNG);
    for (int i = 0; i < RADAR_AZIMUTHS; ++i)
        check(static_cast<int>(scan.azimuths()[i].power.size()) == RADAR_RANGE_BINS,
            "Each azimuth must have 3768 range bins");
    std::cout << "PASS: range bin count = " << RADAR_RANGE_BINS << " for all azimuths\n";
}

// A real scan must contain at least one original (non-interpolated) azimuth
static void test_has_valid_azimuths() {
    RadarScan scan;
    scan.load(TEST_PNG);
    int count = 0;
    for (const auto& az : scan.azimuths())
        if (az.valid) ++count;
    check(count > 0, "Scan must have at least one valid (non-interpolated) azimuth");
    std::cout << "PASS: valid azimuths = " << count << "/" << RADAR_AZIMUTHS << "\n";
}

int main() {
    test_invalid_path();
    test_load_succeeds();
    test_azimuth_count();
    test_range_bin_count();
    test_has_valid_azimuths();
    std::cout << "All load tests passed.\n";
    return 0;
}
