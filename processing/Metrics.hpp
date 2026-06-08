#pragma once
#include <vector>
#include "RadarParser.hpp"

struct AzimuthMetrics {
    float noise_floor;
    float peak_power;
    float snr_db;
    bool valid;
};

struct ScanMetrics {
    std::vector<AzimuthMetrics> per_azimuth;
    float mean_noise_floor;
    float mean_snr_db;
    int total_detections;
    int invalid_azimuths; // interpolated
    bool anomaly_noise; // noise floor > 3 times expected
    bool anomaly_snr; // mean SNR below threshold
    bool anomaly_invalid; // too many invalid azimuths
};

class Metrics {
public:
    // Thresholds for anomaly detection
    struct Config {
        float noise_floor_max  = 0.1f;
        float min_snr_db = 10.0f;
        int max_invalid_pct = 10;
    };

    Metrics();
    explicit Metrics(const Config& cfg);
    ScanMetrics compute(const RadarScan& scan) const;

private:
    Config cfg_;
};
