#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Navtech CTS350-X scan constants of the Oxford Radar RobotCar Dataset
static constexpr int RADAR_AZIMUTHS = 400;
static constexpr int RADAR_RANGE_BINS = 3768; // cols 11-3778 (FFT power)
static constexpr int RADAR_META_COLS = 11; // timestamp(8) + sweep(2) + valid(1)
static constexpr int RADAR_PNG_COLS = 3779; // 11 metadata + 3768 range bins
static constexpr float RADAR_RESOLUTION_M = 0.0432f; // metres per range bin
static constexpr int ENCODER_SIZE = 5600;

struct AzimuthLine {
    int64_t timestamp;
    float angle_rad;
    bool valid; // true means original, not interpolated
    std::vector<float> power;
};

class RadarScan {
public:
    bool load(const std::string& png_path);
    const std::vector<AzimuthLine>& azimuths() const { return azimuths_; }
    float resolution() const { return RADAR_RESOLUTION_M; }
    const std::string& error() const { return error_msg_; }
    static float bin_to_metres(int bin) { return bin * RADAR_RESOLUTION_M; }

private:
    std::vector<AzimuthLine> azimuths_;
    std::string error_msg_;
};
