#include "RadarBackend.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

RadarBackend::RadarBackend(PpiProvider* ppi, QObject* parent) : QObject(parent), ppi_(ppi) {
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

                // render PPI for the current index
                if (idx == navigatingTo_.load())
                    ppi_->render(*cs.scan);
                {
                    QMutexLocker lock(&cacheMutex_);
                    scanCache_.insert(idx, std::move(cs));
                }
                emit scanCached(idx);
            } else if (idx == navigatingTo_.load()) {
                emit statusChanged(QString::fromStdString(cs.scan->error()));
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
    QString path = folderUrl.toLocalFile();
    QStringList names = QDir(path).entryList({"*.png"}, QDir::Files, QDir::Name);

    if (names.isEmpty()) {
        emit statusChanged("No PNG files found in the selected folder.");
        return;
    }

    {
        QMutexLocker lock(&queueMutex_);
        loadQueue_.clear();
    }
    {
        QMutexLocker lock(&cacheMutex_);
        scanCache_.clear();
    }

    files_.clear();
    for (const QString& name : names)
        files_.append(path + "/" + name);

    stereoFiles_.clear();
    QDir stereoDir(QDir(path).absoluteFilePath("../stereo"));
    if (stereoDir.exists()) {
        const QStringList stereoNames = stereoDir.entryList({"*.png"}, QDir::Files, QDir::Name);
        for (const QString& name : stereoNames)
            stereoFiles_.append(stereoDir.absolutePath() + "/" + name);
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

    navigatingTo_.store(index);

    {
        QMutexLocker lock(&cacheMutex_);
        if (scanCache_.contains(index))
            ppi_->render(*scanCache_[index].scan);
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

QString RadarBackend::stereoPath(int index) const {
    if (index < 0 || index >= files_.size() || stereoFiles_.isEmpty())
        return {};
    bool ok;
    qint64 radarTs = QFileInfo(files_[index]).baseName().split('_').first().toLongLong(&ok);
    if (!ok) return {};

    // search for closest stereo timestamp
    int lo = 0, hi = (int)stereoFiles_.size() - 1;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (QFileInfo(stereoFiles_[mid]).baseName().toLongLong() < radarTs)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (lo > 0) {
        qint64 prev = QFileInfo(stereoFiles_[lo - 1]).baseName().toLongLong();
        qint64 cur  = QFileInfo(stereoFiles_[lo]).baseName().toLongLong();
        if (qAbs(prev - radarTs) < qAbs(cur - radarTs))
            return stereoFiles_[lo - 1];
    }
    return stereoFiles_[lo];
}

void RadarBackend::recordReference() {
    if (files_.isEmpty()) return;
    QString folder = QFileInfo(files_.first()).absolutePath();
    QString appDir = QCoreApplication::applicationDirPath();
    QString python = appDir + "/../scripts/.venv/bin/python3";
    QString script = appDir + "/../scripts/valid_reference.py";
    QString binary = appDir + "/metrics_cli";

    emit statusChanged("Recording reference...");
    auto* proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        emit statusChanged(code == 0 ? "Reference recorded."
            : "Record failed: " + proc->readAllStandardError());
        proc->deleteLater();
    });
    proc->start(python, {script, "--binary", binary, "--data-dir", folder});
}

void RadarBackend::runNonRegression() {
    if (files_.isEmpty()) return;
    QString folder = QFileInfo(files_.first()).absolutePath();
    QString appDir = QCoreApplication::applicationDirPath();
    QString python = appDir + "/../scripts/.venv/bin/python3";
    QString script = appDir + "/../scripts/test_non_regression.py";
    QString binary = appDir + "/metrics_cli";

    emit statusChanged("Running non-regression...");
    auto* proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [this, proc](int code, QProcess::ExitStatus) {
        QJsonDocument doc = QJsonDocument::fromJson(proc->readAllStandardOutput());
        if (!doc.isNull()) {
            emit nonRegressionDone(doc.object().toVariantMap());
            emit statusChanged(code == 0 ? "Non-regression OK" : "Non-regression FAILED");
        } else {
            emit statusChanged("Non-regression error: " + proc->readAllStandardError());
        }
        proc->deleteLater();
    });
    proc->start(python, {script, "--binary", binary, "--data-dir", folder, "--json"});
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
