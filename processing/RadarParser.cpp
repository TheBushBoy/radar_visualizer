#include "RadarParser.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static constexpr float PI = 3.14159265358979323846f;

bool RadarScan::load(const std::string& png_path) {
    azimuths_.clear();
    error_msg_.clear();

    int width = 0, height = 0, channels = 0;
    // channels should be 1 for grayscale, but we force it with stbi_load parameter "desired_channel"
    // so even with an RGB PNG we get 1 byte per pixel

    uint8_t* pixels = stbi_load(png_path.c_str(), &width, &height, &channels, 1);
    
    if (!pixels) {
        error_msg_ = "Cannot load PNG: " + png_path + " (" + stbi_failure_reason() + ")";
        return false;
    }

    if (height != RADAR_AZIMUTHS || width != RADAR_PNG_COLS) {
        stbi_image_free(pixels);
        error_msg_ = "Unexpected dimensions " + std::to_string(width) + "x" +
                     std::to_string(height) + " (expected " +
                     std::to_string(RADAR_PNG_COLS) + "x" +
                     std::to_string(RADAR_AZIMUTHS) + ")";
        return false;
    }

    // Parse each azimuth row
    azimuths_.resize(height);
    for (int r = 0; r < height; ++r) {
        const uint8_t* row = pixels + r * width;
        AzimuthLine& az = azimuths_[r];

        // Bytes 0-7, int64 timestamp, little-endian
        int64_t ts = 0;
        for (int b = 0; b < 8; ++b)
            ts |= static_cast<int64_t>(row[b]) << (b * 8);
        az.timestamp = ts;

        // Bytes 8-9, uint16 sweep counter, little-endian
        uint16_t sweep = static_cast<uint16_t>(row[8]) | (static_cast<uint16_t>(row[9]) << 8);
        az.angle_rad = static_cast<float>(sweep) / ENCODER_SIZE * 2.0f * PI;

        // Byte 10, valid flag, 255 = original, 0 = interpolated
        az.valid = (row[10] == 255);

        // Remaining bytes, normalised FFT power
        az.power.resize(RADAR_RANGE_BINS);
        for (int c = 0; c < RADAR_RANGE_BINS; ++c)
            az.power[c] = row[RADAR_META_COLS + c] / 255.0f;
    }

    stbi_image_free(pixels);
    return true;
}
