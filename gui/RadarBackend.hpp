#pragma once
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QStringList>
#include <QThread>
#include <QUrl>
#include <QVariantMap>
#include <QWaitCondition>
#include <memory>

#include "Metrics.hpp"
#include "RadarParser.hpp"

class RadarBackend : public QObject {
    Q_OBJECT
public:
    // Number of scans preloaded before and after the current index
    static constexpr int WIDTH_OF_PRESCAN = 10;

    explicit RadarBackend(QObject* parent = nullptr);
    ~RadarBackend();

    Q_INVOKABLE void openFolder(const QUrl& folderUrl);
    Q_INVOKABLE void navigate(int index);
    Q_INVOKABLE bool hasScan(int index);
    Q_INVOKABLE QString fileName(int index) const;
    Q_INVOKABLE QVariantMap metricsAt(int index);

    const QStringList& files() const { return files_; }

signals:
    void statusChanged(const QString& message);
    void folderReady(int count);
    void scanCached(int index);

private:
    struct CachedScan {
        std::shared_ptr<RadarScan> scan;
        ScanMetrics metrics;
    };

    void startWorker();
    void stopWorker();
    void rebuildQueue(int center, const QStringList& files);

    QStringList files_;

    QHash<int, CachedScan> scanCache_;
    QMutex cacheMutex_; // mutex on scanCache_

    QList<int> loadQueue_;
    QStringList workerFiles_;
    bool workerShouldStop_ = false;
    QMutex queueMutex_; // mutex on loadQueue_ workerFiles_ workerShouldStop_
    QWaitCondition queueCond_; // wakes the worker when the queue is updated

    QThread* workerThread_ = nullptr;
};
