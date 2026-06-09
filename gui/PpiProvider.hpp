#pragma once
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

#include "RadarParser.hpp"


class PpiProvider : public QQuickImageProvider {
public:
    PpiProvider();
    void render(const RadarScan& scan);
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    static constexpr int IMAGE_SIZE = 512;

    QImage image_;
    mutable QMutex mutex_;
};
