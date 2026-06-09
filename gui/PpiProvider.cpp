#include "PpiProvider.hpp"
#include <cmath>

static constexpr float PI = 3.14159265358979f;

PpiProvider::PpiProvider()
    : QQuickImageProvider(QQuickImageProvider::Image) {
    image_ = QImage(IMAGE_SIZE, IMAGE_SIZE, QImage::Format_ARGB32);
    image_.fill(0);
}

void PpiProvider::render(const RadarScan& scan) {
    QImage img(IMAGE_SIZE, IMAGE_SIZE, QImage::Format_ARGB32);
    img.fill(0); // transparent

    const auto& azimuths = scan.azimuths();
    const int nAz = static_cast<int>(azimuths.size());
    const int nRange = RADAR_RANGE_BINS;
    const float cx = IMAGE_SIZE / 2.0f;
    const float cy = IMAGE_SIZE / 2.0f;
    const float maxR = IMAGE_SIZE / 2.0f - 1.0f;

    for (int py = 0; py < IMAGE_SIZE; ++py) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(py));
        for (int px = 0; px < IMAGE_SIZE; ++px) {
            float dx = px - cx;
            float dy = py - cy;
            float r = std::sqrt(dx * dx + dy * dy);
            if (r > maxR) continue;

            int rangeIdx = static_cast<int>(r / maxR * (nRange - 1));

            float angle = std::atan2(dx, -dy); // north up, clockwise rotation
            if (angle < 0.0f) angle += 2.0f * PI;

            int azIdx = static_cast<int>(angle / (2.0f * PI) * nAz);
            if (azIdx >= nAz) azIdx = nAz - 1;

            // sqrt to lift weak returns above black
            uchar gray = static_cast<uchar>(std::sqrt(azimuths[azIdx].power[rangeIdx]) * 255.0f);
            line[px] = qRgba(gray, gray, gray, 255);
        }
    }

    QMutexLocker lock(&mutex_);
    image_ = std::move(img);
}

QImage PpiProvider::requestImage(const QString& /*id*/, QSize* size, const QSize& requestedSize) {
    QMutexLocker lock(&mutex_);
    QImage result = image_;
    if (size) *size = result.size();
    if (requestedSize.isValid())
        result = result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return result;
}
