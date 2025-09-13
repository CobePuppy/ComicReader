#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QQueue>
#include <QMutex>

struct DownloadTask {
    QString id;
    QUrl url;
    QString destinationPath;
    QString fileName;
    bool resumable;
    qint64 downloadedBytes;
    qint64 totalBytes;
    int retryCount;
    QNetworkReply* reply;
    
    DownloadTask() : resumable(false), downloadedBytes(0), totalBytes(0), retryCount(0), reply(nullptr) {}
};

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    enum DownloadStatus {
        Pending,
        Downloading,
        Paused,
        Completed,
        Failed,
        Cancelled
    };

    static DownloadManager* instance();
    ~DownloadManager();

    // 下载任务管理
    QString addDownload(const QUrl &url, const QString &destinationPath, const QString &fileName = QString());
    void pauseDownload(const QString &taskId);
    void resumeDownload(const QString &taskId);
    void cancelDownload(const QString &taskId);
    void removeDownload(const QString &taskId);
    
    // 批量操作
    void pauseAllDownloads();
    void resumeAllDownloads();
    void cancelAllDownloads();
    void clearCompletedDownloads();
    
    // 状态查询
    DownloadStatus getDownloadStatus(const QString &taskId) const;
    QList<DownloadTask> getAllDownloads() const;
    QList<DownloadTask> getActiveDownloads() const;
    QList<DownloadTask> getCompletedDownloads() const;
    
    // 下载统计
    int getTotalDownloads() const;
    int getActiveDownloadCount() const;
    qint64 getTotalDownloadedBytes() const;
    qint64 getTotalRemainingBytes() const;
    
    // 配置管理
    void setMaxConcurrentDownloads(int count);
    void setRetryCount(int count);
    void setRetryDelay(int seconds);
    void setTimeout(int seconds);
    void setSpeedLimit(qint64 bytesPerSecond); // 0 = unlimited
    
    int getMaxConcurrentDownloads() const { return m_maxConcurrentDownloads; }
    int getRetryCount() const { return m_retryCount; }
    int getRetryDelay() const { return m_retryDelay; }
    int getTimeout() const { return m_timeout; }
    qint64 getSpeedLimit() const { return m_speedLimit; }

signals:
    // 下载事件信号
    void downloadStarted(const QString &taskId);
    void downloadProgress(const QString &taskId, qint64 downloaded, qint64 total);
    void downloadPaused(const QString &taskId);
    void downloadResumed(const QString &taskId);
    void downloadCompleted(const QString &taskId, const QString &filePath);
    void downloadFailed(const QString &taskId, const QString &error);
    void downloadCancelled(const QString &taskId);
    
    // 全局状态信号
    void allDownloadsCompleted();
    void downloadSpeedChanged(qint64 bytesPerSecond);

private slots:
    void onDownloadProgress(qint64 downloaded, qint64 total);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError error);
    void onRetryTimer();
    void onSpeedLimitTimer();

private:
    explicit DownloadManager(QObject *parent = nullptr);
    static DownloadManager* m_instance;
    
    // 核心方法
    void startNextDownload();
    void processDownloadQueue();
    void handleDownloadCompletion(DownloadTask* task);
    void handleDownloadError(DownloadTask* task, const QString &error);
    void retryDownload(DownloadTask* task);
    
    // 工具方法
    QString generateTaskId() const;
    QString getFileNameFromUrl(const QUrl &url) const;
    bool isResumable(const QUrl &url) const;
    qint64 getExistingFileSize(const QString &filePath) const;
    void enforceSpeedLimit();
    
    // 网络管理
    QNetworkAccessManager* m_networkManager;
    QQueue<QString> m_pendingQueue;
    QHash<QString, DownloadTask*> m_downloads;
    QHash<QNetworkReply*, QString> m_replyToTaskId;
    
    // 配置参数
    int m_maxConcurrentDownloads;
    int m_retryCount;
    int m_retryDelay;          // 重试延迟（秒）
    int m_timeout;             // 超时时间（秒）
    qint64 m_speedLimit;       // 速度限制（字节/秒）
    
    // 状态管理
    QMutex m_mutex;
    QTimer* m_retryTimer;
    QTimer* m_speedLimitTimer;
    qint64 m_lastSpeedCheckTime;
    qint64 m_bytesDownloadedSinceLastCheck;
    int m_activeDownloads;
};

#endif // DOWNLOADMANAGER_H
