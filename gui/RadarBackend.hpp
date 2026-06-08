#pragma once
#include <QObject>
#include <QStringList>
#include <QThread>
#include <QUrl>

#include "Metrics.hpp"

class RadarBackend : public QObject {
    Q_OBJECT
public:
    explicit RadarBackend(QObject* parent = nullptr);
    ~RadarBackend();

    Q_INVOKABLE void scanFolder(const QUrl& folderUrl);

    const QStringList& files() const { return files_; }
    const QVector<ScanMetrics>& metrics() const { return metrics_; }

signals:
    void statusChanged(const QString& message);
    void scanComplete(int count);

private:
    QStringList files_;
    QVector<ScanMetrics> metrics_;
    QThread* thread_ = nullptr;
};
