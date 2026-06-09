#include <cstdio>
#include <cstring>

#include "Metrics.hpp"
#include "RadarParser.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2 || std::strcmp(argv[1], "--help") == 0) {
        std::fprintf(stderr, "Usage: metrics_cli <scan.png>\n");
        std::fprintf(stderr, "Outputs scan metrics as JSON on stdout.\n");
        return 1;
    }

    RadarScan scan;
    if (!scan.load(argv[1])) {
        std::fprintf(stderr, "Error: %s\n", scan.error().c_str());
        return 1;
    }

    Metrics m;
    ScanMetrics r = m.compute(scan);

    std::printf(
        "{\n"
        "  \"mean_noise_floor\": %.6f,\n"
        "  \"mean_snr_db\": %.6f,\n"
        "  \"invalid_azimuths\": %d,\n"
        "  \"anomaly_noise\": %s,\n"
        "  \"anomaly_snr\": %s,\n"
        "  \"anomaly_invalid\": %s\n"
        "}\n",
        (double)r.mean_noise_floor,
        (double)r.mean_snr_db,
        r.invalid_azimuths,
        r.anomaly_noise ? "true" : "false",
        r.anomaly_snr ? "true" : "false",
        r.anomaly_invalid ? "true" : "false"
    );
    return 0;
}
