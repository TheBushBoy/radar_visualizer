#include "RadarBackend.hpp"
#include <QDir>
#include <QFileInfo>

RadarBackend::RadarBackend(QObject* parent) : QObject(parent) {
    startWorker();
}

RadarBackend::~RadarBackend() {
    stopWorker();
}

void RadarBackend::startWorker() {
    workerThread_ = QThread::create([this]() {
        Metrics calc;
        while (true) {
            int idx = -1;
            QStringList files;
            {
                QMutexLocker lock(&queueMutex_);
                // Sleep until action needed
                while (loadQueue_.isEmpty() && !workerShouldStop_)
                    queueCond_.wait(&queueMutex_);
                if (workerShouldStop_)
                    return;
                idx = loadQueue_.takeFirst();
                files = workerFiles_; // snapshot of the file list
            }

            {
                QMutexLocker lock(&cacheMutex_);
                if (scanCache_.contains(idx))
                    continue;
            }

            if (idx < 0 || idx >= files.size())
                continue;

            CachedScan cs;
            cs.scan = std::make_shared<RadarScan>();
            if (cs.scan->load(files[idx].toStdString())) {
                cs.metrics = calc.compute(*cs.scan);
                {
                    QMutexLocker lock(&cacheMutex_);
                    scanCache_.insert(idx, std::move(cs));
                }

                emit scanCached(idx);
            }
        }
    });
    workerThread_->start();
}

void RadarBackend::stopWorker() {
    {
        QMutexLocker lock(&queueMutex_);
        workerShouldStop_ = true;
        loadQueue_.clear();
        queueCond_.wakeOne();
    }
    if (workerThread_) {
        workerThread_->wait();
        delete workerThread_;
        workerThread_ = nullptr;
    }
}

void RadarBackend::rebuildQueue(int center, const QStringList& files) {
    int total = files.size();
    if (total == 0) return;

    // cache current index first, then alternating forward/backward
    QList<int> queue;
    if (center >= 0 && center < total)
        queue.append(center);
    for (int d = 1; d <= WIDTH_OF_PRESCAN; ++d) {
        if (center + d < total) queue.append(center + d);
        if (center - d >= 0) queue.append(center - d);
    }

    // filter indices already in cache
    {
        QMutexLocker cacheLock(&cacheMutex_);
        QList<int> filtered;
        for (int i : queue)
            if (!scanCache_.contains(i))
                filtered.append(i);
        queue = filtered;
    }

    {
        QMutexLocker lock(&queueMutex_);
        loadQueue_ = queue;
        workerFiles_ = files;
        queueCond_.wakeOne();
    }
}

void RadarBackend::openFolder(const QUrl& folderUrl) {
    {
        QMutexLocker lock(&queueMutex_);
        loadQueue_.clear();
    }
    {
        QMutexLocker lock(&cacheMutex_);
        scanCache_.clear();
    }

    QString path = folderUrl.toLocalFile();
    QStringList names = QDir(path).entryList({"*.png"}, QDir::Files, QDir::Name);

    files_.clear();
    for (const QString& name : names)
        files_.append(path + "/" + name);

    if (files_.isEmpty()) {
        emit statusChanged("No PNG files found in the selected folder.");
        return;
    }

    emit folderReady(files_.size());
}

void RadarBackend::navigate(int index) {
    int total = files_.size();
    if (total == 0 || index < 0 || index >= total)
        return;

    int lo = qMax(0, index - WIDTH_OF_PRESCAN);
    int hi = qMin(total - 1, index + WIDTH_OF_PRESCAN);
    {
        QMutexLocker lock(&cacheMutex_);
        const QList<int> keys = scanCache_.keys();
        for (int k : keys)
            if (k < lo || k > hi)
                scanCache_.remove(k);
    }

    rebuildQueue(index, files_);
}

bool RadarBackend::hasScan(int index) {
    QMutexLocker lock(&cacheMutex_);
    return scanCache_.contains(index);
}

QString RadarBackend::fileName(int index) const {
    if (index < 0 || index >= files_.size())
        return {};
    return QFileInfo(files_[index]).fileName();
}

QVariantMap RadarBackend::metricsAt(int index) {
    QMutexLocker lock(&cacheMutex_);
    if (!scanCache_.contains(index))
        return {};
    const ScanMetrics& m = scanCache_[index].metrics;
    return {
        {"meanNoiseFloor", m.mean_noise_floor},
        {"meanSnrDb", m.mean_snr_db},
        {"invalidAzimuths", m.invalid_azimuths},
        {"anomalyNoise", m.anomaly_noise},
        {"anomalySnr", m.anomaly_snr},
        {"anomalyInvalid", m.anomaly_invalid}
    };
}
