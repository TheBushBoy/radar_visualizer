#include "Metrics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

Metrics::Metrics() : cfg_(Config{}) {}
Metrics::Metrics(const Config& cfg) : cfg_(cfg) {}

ScanMetrics Metrics::compute(const RadarScan& scan) const {
    ScanMetrics result{};
    const auto& azimuths = scan.azimuths();
    result.per_azimuth.reserve(azimuths.size());

    double sum_noise = 0.0, sum_snr = 0.0;

    for (const auto& az : azimuths) {
        AzimuthMetrics am{};
        am.valid = az.valid;
        if (!az.valid) ++result.invalid_azimuths;
        const auto& p = az.power;

        // Noise floor, median of all range bins.
        std::vector<float> sorted_p(p.begin(), p.end());
        std::sort(sorted_p.begin(), sorted_p.end());
        float noise = sorted_p[sorted_p.size() / 2];
        am.noise_floor = noise;

        // Peak
        am.peak_power = *std::max_element(p.begin(), p.end());

        // SNR in dB
        am.snr_db = (noise > 1e-9f)
            ? 20.0f * std::log10(am.peak_power / noise)
            : -1.0f;

        sum_noise += am.noise_floor;
        sum_snr += am.snr_db;

        result.per_azimuth.push_back(am);
    }

    int n = static_cast<int>(azimuths.size());
    result.mean_noise_floor = (n > 0) ? (float)(sum_noise / n) : 0.0f;
    result.mean_snr_db = (n > 0) ? (float)(sum_snr / n) : 0.0f;

    result.anomaly_noise = result.mean_noise_floor > cfg_.noise_floor_max;
    result.anomaly_snr = result.mean_snr_db < cfg_.min_snr_db;
    result.anomaly_invalid = (n > 0) && 
        (result.invalid_azimuths * 100 / n) > cfg_.max_invalid_pct;

    return result;
}
