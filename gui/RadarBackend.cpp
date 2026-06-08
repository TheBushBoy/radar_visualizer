#include "RadarBackend.hpp"
#include <QDir>
#include "RadarParser.hpp"

RadarBackend::RadarBackend(QObject* parent) : QObject(parent) {}

RadarBackend::~RadarBackend() {
    if (thread_ && thread_->isRunning()) {
        thread_->requestInterruption();
        thread_->wait();
    }
}

void RadarBackend::scanFolder(const QUrl& folderUrl) {
    if (thread_) {
        if (thread_->isRunning()) {
            thread_->requestInterruption();
            thread_->wait();
        }
        delete thread_;
        thread_ = nullptr;
    }

    QString path = folderUrl.toLocalFile();
    QStringList names = QDir(path).entryList({"*.png"}, QDir::Files, QDir::Name);

    files_.clear();
    for (const QString& name : names)
        files_.append(path + "/" + name);

    metrics_.clear();
    metrics_.reserve(files_.size());

    int total = files_.size();

    if (total == 0) {
        emit statusChanged("No PNG files found in the selected folder.");
        return;
    }

    emit statusChanged(QString("Found %1 files. Starting scan...").arg(total));

    QStringList filesCopy = files_;
    thread_ = QThread::create([this, filesCopy, total]() {
        Metrics calc;
        for (int i = 0; i < total; ++i) {
            if (QThread::currentThread()->isInterruptionRequested())
                return;

            RadarScan scan;
            if (scan.load(filesCopy[i].toStdString()))
                metrics_.append(calc.compute(scan));
            else
                metrics_.append(ScanMetrics{});

            if ((i + 1) % 10 == 0 || i + 1 == total)
                emit statusChanged(
                    QString("Scanning %1 / %2...").arg(i + 1).arg(total));
        }
        emit scanComplete(metrics_.size());
    });
    thread_->start();
}
