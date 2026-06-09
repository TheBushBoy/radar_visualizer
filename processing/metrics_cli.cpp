#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

#include "Metrics.hpp"
#include "RadarParser.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2 || std::strcmp(argv[1], "--help") == 0) {
        std::fprintf(stderr, "Usage: metrics_cli <directory>\n");
        return 1;
    }

    fs::path dir(argv[1]);
    if (!fs::is_directory(dir)) {
        std::fprintf(stderr, "Error: %s is not a directory\n", argv[1]);
        return 1;
    }

    std::vector<fs::path> pngs;
    for (const auto& entry : fs::directory_iterator(dir))
        if (entry.path().extension() == ".png")
            pngs.push_back(entry.path());
    std::sort(pngs.begin(), pngs.end());

    if (pngs.empty()) {
        std::fprintf(stderr, "Error: no PNG files in %s\n", argv[1]);
        return 1;
    }

    Metrics m;
    int n = 0;
    double sum_noise = 0, sum_snr = 0, sum_invalid = 0;
    int cnt_noise = 0, cnt_snr = 0, cnt_invalid = 0;

    for (const auto& png : pngs) {
        RadarScan scan;
        if (!scan.load(png.string())) {
            std::fprintf(stderr, "Error loading %s: %s\n",
                         png.filename().c_str(), scan.error().c_str());
            return 1;
        }
        ScanMetrics r = m.compute(scan);
        sum_noise += r.mean_noise_floor;
        sum_snr += r.mean_snr_db;
        sum_invalid += r.invalid_azimuths;
        if (r.anomaly_noise) ++cnt_noise;
        if (r.anomaly_snr) ++cnt_snr;
        if (r.anomaly_invalid) ++cnt_invalid;
        ++n;
    }

    std::printf(
        "{\n"
        "  \"n_files\": %d,\n"
        "  \"mean_noise_floor\": %.6f,\n"
        "  \"mean_snr_db\": %.6f,\n"
        "  \"mean_invalid_azimuths\": %.2f,\n"
        "  \"anomaly_noise_pct\": %.2f,\n"
        "  \"anomaly_snr_pct\": %.2f,\n"
        "  \"anomaly_invalid_pct\": %.2f\n"
        "}\n",
        n,
        sum_noise / n, sum_snr / n, sum_invalid / n,
        (double)cnt_noise / n * 100,
        (double)cnt_snr / n * 100,
        (double)cnt_invalid / n * 100
    );
    return 0;
}
