#include <iostream>
#include <string>

#include "RadarParser.hpp"

static constexpr std::string_view DATA_DIR = RADAR_DATA_DIR;

int main(int argc, char *argv[]) {
    RadarScan scan;

    if (!scan.load(std::string(DATA_DIR) + "1547131046353776.png")) {
        std::cerr << "Error loading radar scan: " << scan.error() << std::endl;
        return 1;
    }
    return 0;
}