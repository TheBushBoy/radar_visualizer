#pragma once
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QStringList>
#include <QThread>
#include <QUrl>
#include <QVariantMap>
#include <QWaitCondition>
#include <atomic>
#include <memory>

#include "Metrics.hpp"
#include "PpiProvider.hpp"
#include "RadarParser.hpp"

class RadarBackend : public QObject {
    Q_OBJECT
public:
    // Number of scans preloaded before and after the current index
    static constexpr int WIDTH_OF_PRESCAN = 10;

    explicit RadarBackend(PpiProvider* ppi, QObject* parent = nullptr);
    ~RadarBackend();

    Q_INVOKABLE void openFolder(const QUrl& folderUrl);
    Q_INVOKABLE void navigate(int index);
    Q_INVOKABLE bool hasScan(int index);
    Q_INVOKABLE QString fileName(int index) const;
    Q_INVOKABLE QVariantMap metricsAt(int index);
    Q_INVOKABLE QString stereoPath(int index) const;
    Q_INVOKABLE void recordReference();
    Q_INVOKABLE void runNonRegression();

    const QStringList& files() const { return files_; }

signals:
    void statusChanged(const QString& message);
    void folderReady(int count);
    void scanCached(int index);
    void nonRegressionDone(const QVariantMap& result);

private:
    struct CachedScan {
        std::shared_ptr<RadarScan> scan;
        ScanMetrics metrics;
    };

    void startWorker();
    void stopWorker();
    void rebuildQueue(int center, const QStringList& files);

    PpiProvider* ppi_;
    std::atomic<int> navigatingTo_{-1}; // index to render PPI for

    QStringList files_;
    QStringList stereoFiles_; // sorted by timestamp filename

    QHash<int, CachedScan> scanCache_;
    QMutex cacheMutex_; // mutex on scanCache_

    QList<int> loadQueue_;
    QStringList workerFiles_;
    bool workerShouldStop_ = false;
    QMutex queueMutex_; // mutex on loadQueue_ workerFiles_ workerShouldStop_
    QWaitCondition queueCond_; // wakes the worker when the queue is updated

    QThread* workerThread_ = nullptr;
};
