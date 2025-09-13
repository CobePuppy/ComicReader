#include "../../include/network/DownloadManager.h"
#include "../../include/core/ConfigManager.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QMutexLocker>

DownloadManager* DownloadManager::m_instance = nullptr;

DownloadManager* DownloadManager::instance()
{
    if (!m_instance) {
        m_instance = new DownloadManager();
    }
    return m_instance;
}

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_maxConcurrentDownloads(3)
    , m_retryCount(3)
    , m_retryDelay(5)
    , m_timeout(30)
    , m_speedLimit(0)
    , m_retryTimer(new QTimer(this))
    , m_speedLimitTimer(new QTimer(this))
    , m_lastSpeedCheckTime(0)
    , m_bytesDownloadedSinceLastCheck(0)
    , m_activeDownloads(0)
{
    // 配置网络管理器
    m_networkManager->setTransferTimeout(m_timeout * 1000);
    
    // 设置重试定时器
    m_retryTimer->setSingleShot(true);
    connect(m_retryTimer, &QTimer::timeout, this, &DownloadManager::onRetryTimer);
    
    // 设置速度限制定时器
    m_speedLimitTimer->setInterval(1000); // 每秒检查一次
    connect(m_speedLimitTimer, &QTimer::timeout, this, &DownloadManager::onSpeedLimitTimer);
    
    // 从配置加载设置
    ConfigManager* config = ConfigManager::instance();
    m_maxConcurrentDownloads = config->getValue("download/maxConcurrent", 3).toInt();
    m_retryCount = config->getValue("download/retryCount", 3).toInt();
    m_retryDelay = config->getValue("download/retryDelay", 5).toInt();
    m_timeout = config->getValue("download/timeout", 30).toInt();
    m_speedLimit = config->getValue("download/speedLimit", 0).toLongLong();
}

DownloadManager::~DownloadManager()
{
    cancelAllDownloads();
    
    // 清理下载任务
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        delete it.value();
    }
    m_downloads.clear();
}

QString DownloadManager::addDownload(const QUrl &url, const QString &destinationPath, const QString &fileName)
{
    QMutexLocker locker(&m_mutex);
    
    // 生成任务ID
    QString taskId = generateTaskId();
    
    // 创建下载任务
    DownloadTask* task = new DownloadTask();
    task->id = taskId;
    task->url = url;
    task->destinationPath = destinationPath;
    task->fileName = fileName.isEmpty() ? getFileNameFromUrl(url) : fileName;
    task->resumable = isResumable(url);
    task->retryCount = 0;
    
    // 检查是否已存在部分下载的文件
    QString fullPath = QDir(destinationPath).absoluteFilePath(task->fileName);
    if (QFile::exists(fullPath)) {
        task->downloadedBytes = getExistingFileSize(fullPath);
    }
    
    // 添加到任务列表
    m_downloads.insert(taskId, task);
    m_pendingQueue.enqueue(taskId);
    
    // 尝试开始下载
    processDownloadQueue();
    
    return taskId;
}

void DownloadManager::pauseDownload(const QString &taskId)
{
    QMutexLocker locker(&m_mutex);
    
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) return;
    
    if (task->reply) {
        task->reply->abort();
        task->reply = nullptr;
        m_activeDownloads--;
    }
    
    // 从待下载队列中移除
    QQueue<QString> newQueue;
    while (!m_pendingQueue.isEmpty()) {
        QString id = m_pendingQueue.dequeue();
        if (id != taskId) {
            newQueue.enqueue(id);
        }
    }
    m_pendingQueue = newQueue;
    
    emit downloadPaused(taskId);
    
    // 尝试开始下一个下载
    processDownloadQueue();
}

void DownloadManager::resumeDownload(const QString &taskId)
{
    QMutexLocker locker(&m_mutex);
    
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) return;
    
    // 添加到待下载队列
    m_pendingQueue.enqueue(taskId);
    
    emit downloadResumed(taskId);
    
    // 尝试开始下载
    processDownloadQueue();
}

void DownloadManager::cancelDownload(const QString &taskId)
{
    QMutexLocker locker(&m_mutex);
    
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) return;
    
    if (task->reply) {
        task->reply->abort();
        task->reply = nullptr;
        m_activeDownloads--;
    }
    
    // 从待下载队列中移除
    QQueue<QString> newQueue;
    while (!m_pendingQueue.isEmpty()) {
        QString id = m_pendingQueue.dequeue();
        if (id != taskId) {
            newQueue.enqueue(id);
        }
    }
    m_pendingQueue = newQueue;
    
    emit downloadCancelled(taskId);
    
    // 尝试开始下一个下载
    processDownloadQueue();
}

void DownloadManager::removeDownload(const QString &taskId)
{
    cancelDownload(taskId);
    
    QMutexLocker locker(&m_mutex);
    
    DownloadTask* task = m_downloads.take(taskId);
    if (task) {
        delete task;
    }
}

void DownloadManager::pauseAllDownloads()
{
    QMutexLocker locker(&m_mutex);
    
    // 暂停所有活动下载
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (task->reply) {
            task->reply->abort();
            task->reply = nullptr;
        }
    }
    
    m_activeDownloads = 0;
    m_pendingQueue.clear();
    
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        emit downloadPaused(it.key());
    }
}

void DownloadManager::resumeAllDownloads()
{
    QMutexLocker locker(&m_mutex);
    
    // 将所有未完成的任务添加到队列
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (!task->reply && task->downloadedBytes < task->totalBytes) {
            m_pendingQueue.enqueue(it.key());
            emit downloadResumed(it.key());
        }
    }
    
    processDownloadQueue();
}

void DownloadManager::cancelAllDownloads()
{
    QMutexLocker locker(&m_mutex);
    
    // 取消所有活动下载
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (task->reply) {
            task->reply->abort();
            task->reply = nullptr;
        }
        emit downloadCancelled(it.key());
    }
    
    m_activeDownloads = 0;
    m_pendingQueue.clear();
}

void DownloadManager::clearCompletedDownloads()
{
    QMutexLocker locker(&m_mutex);
    
    QList<QString> completedTasks;
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (task->downloadedBytes >= task->totalBytes && task->totalBytes > 0) {
            completedTasks.append(it.key());
        }
    }
    
    for (const QString &taskId : completedTasks) {
        DownloadTask* task = m_downloads.take(taskId);
        delete task;
    }
}

DownloadManager::DownloadStatus DownloadManager::getDownloadStatus(const QString &taskId) const
{
    QMutexLocker locker(&m_mutex);
    
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) return Failed;
    
    if (task->reply) {
        return Downloading;
    } else if (task->downloadedBytes >= task->totalBytes && task->totalBytes > 0) {
        return Completed;
    } else if (m_pendingQueue.contains(taskId)) {
        return Pending;
    } else {
        return Paused;
    }
}

QList<DownloadTask> DownloadManager::getAllDownloads() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<DownloadTask> result;
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        result.append(*(it.value()));
    }
    return result;
}

QList<DownloadTask> DownloadManager::getActiveDownloads() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<DownloadTask> result;
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (task->reply || m_pendingQueue.contains(it.key())) {
            result.append(*task);
        }
    }
    return result;
}

QList<DownloadTask> DownloadManager::getCompletedDownloads() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<DownloadTask> result;
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (task->downloadedBytes >= task->totalBytes && task->totalBytes > 0) {
            result.append(*task);
        }
    }
    return result;
}

int DownloadManager::getTotalDownloads() const
{
    QMutexLocker locker(&m_mutex);
    return m_downloads.size();
}

int DownloadManager::getActiveDownloadCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_activeDownloads;
}

qint64 DownloadManager::getTotalDownloadedBytes() const
{
    QMutexLocker locker(&m_mutex);
    
    qint64 total = 0;
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        total += it.value()->downloadedBytes;
    }
    return total;
}

qint64 DownloadManager::getTotalRemainingBytes() const
{
    QMutexLocker locker(&m_mutex);
    
    qint64 remaining = 0;
    for (auto it = m_downloads.begin(); it != m_downloads.end(); ++it) {
        DownloadTask* task = it.value();
        if (task->totalBytes > 0) {
            remaining += qMax(0LL, task->totalBytes - task->downloadedBytes);
        }
    }
    return remaining;
}

void DownloadManager::setMaxConcurrentDownloads(int count)
{
    m_maxConcurrentDownloads = qMax(1, count);
    ConfigManager::instance()->setValue("download/maxConcurrent", count);
    processDownloadQueue();
}

void DownloadManager::setRetryCount(int count)
{
    m_retryCount = qMax(0, count);
    ConfigManager::instance()->setValue("download/retryCount", count);
}

void DownloadManager::setRetryDelay(int seconds)
{
    m_retryDelay = qMax(1, seconds);
    ConfigManager::instance()->setValue("download/retryDelay", seconds);
}

void DownloadManager::setTimeout(int seconds)
{
    m_timeout = qMax(5, seconds);
    m_networkManager->setTransferTimeout(m_timeout * 1000);
    ConfigManager::instance()->setValue("download/timeout", seconds);
}

void DownloadManager::setSpeedLimit(qint64 bytesPerSecond)
{
    m_speedLimit = qMax(0LL, bytesPerSecond);
    ConfigManager::instance()->setValue("download/speedLimit", m_speedLimit);
    
    if (m_speedLimit > 0) {
        if (!m_speedLimitTimer->isActive()) {
            m_speedLimitTimer->start();
            m_lastSpeedCheckTime = QDateTime::currentMSecsSinceEpoch();
            m_bytesDownloadedSinceLastCheck = 0;
        }
    } else {
        m_speedLimitTimer->stop();
    }
}

void DownloadManager::onDownloadProgress(qint64 downloaded, qint64 total)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString taskId = m_replyToTaskId.value(reply);
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) return;
    
    qint64 previousDownloaded = task->downloadedBytes;
    task->downloadedBytes = downloaded;
    task->totalBytes = total;
    
    // 统计下载速度
    if (m_speedLimit > 0) {
        m_bytesDownloadedSinceLastCheck += (downloaded - previousDownloaded);
    }
    
    emit downloadProgress(taskId, downloaded, total);
}

void DownloadManager::onDownloadFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString taskId = m_replyToTaskId.take(reply);
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) {
        reply->deleteLater();
        return;
    }
    
    m_activeDownloads--;
    task->reply = nullptr;
    
    if (reply->error() == QNetworkReply::NoError) {
        // 下载成功
        handleDownloadCompletion(task);
    } else {
        // 下载出错
        handleDownloadError(task, reply->errorString());
    }
    
    reply->deleteLater();
    
    // 处理下载队列
    processDownloadQueue();
    
    // 检查是否所有下载都完成了
    if (getActiveDownloadCount() == 0 && m_pendingQueue.isEmpty()) {
        emit allDownloadsCompleted();
    }
}

void DownloadManager::onDownloadError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    // 错误处理在 onDownloadFinished 中进行
}

void DownloadManager::onRetryTimer()
{
    processDownloadQueue();
}

void DownloadManager::onSpeedLimitTimer()
{
    if (m_speedLimit <= 0) return;
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedMs = currentTime - m_lastSpeedCheckTime;
    
    if (elapsedMs >= 1000) { // 至少1秒
        qint64 currentSpeed = (m_bytesDownloadedSinceLastCheck * 1000) / elapsedMs;
        
        emit downloadSpeedChanged(currentSpeed);
        
        // 如果超过速度限制，暂停一段时间
        if (currentSpeed > m_speedLimit) {
            qint64 excessBytes = m_bytesDownloadedSinceLastCheck - (m_speedLimit * elapsedMs / 1000);
            qint64 delayMs = (excessBytes * 1000) / m_speedLimit;
            
            if (delayMs > 0) {
                QThread::msleep(qMin(delayMs, 1000LL)); // 最多延迟1秒
            }
        }
        
        m_lastSpeedCheckTime = currentTime;
        m_bytesDownloadedSinceLastCheck = 0;
    }
}

void DownloadManager::startNextDownload()
{
    if (m_activeDownloads >= m_maxConcurrentDownloads || m_pendingQueue.isEmpty()) {
        return;
    }
    
    QString taskId = m_pendingQueue.dequeue();
    DownloadTask* task = m_downloads.value(taskId);
    if (!task) return;
    
    // 创建网络请求
    QNetworkRequest request(task->url);
    request.setRawHeader("User-Agent", "ComicReader/1.0");
    
    // 如果支持断点续传，设置Range头
    if (task->resumable && task->downloadedBytes > 0) {
        QString range = QString("bytes=%1-").arg(task->downloadedBytes);
        request.setRawHeader("Range", range.toUtf8());
    }
    
    // 发起下载请求
    QNetworkReply* reply = m_networkManager->get(request);
    task->reply = reply;
    m_replyToTaskId.insert(reply, taskId);
    m_activeDownloads++;
    
    // 连接信号
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, &DownloadManager::onDownloadFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &DownloadManager::onDownloadError);
    
    emit downloadStarted(taskId);
}

void DownloadManager::processDownloadQueue()
{
    while (m_activeDownloads < m_maxConcurrentDownloads && !m_pendingQueue.isEmpty()) {
        startNextDownload();
    }
}

void DownloadManager::handleDownloadCompletion(DownloadTask* task)
{
    // 保存下载的数据到文件
    if (task->reply) {
        QByteArray data = task->reply->readAll();
        QString fullPath = QDir(task->destinationPath).absoluteFilePath(task->fileName);
        
        // 确保目录存在
        QDir dir;
        dir.mkpath(task->destinationPath);
        
        QFile file(fullPath);
        QIODevice::OpenMode mode = task->downloadedBytes > 0 ? QIODevice::WriteOnly | QIODevice::Append 
                                                              : QIODevice::WriteOnly;
        
        if (file.open(mode)) {
            file.write(data);
            file.close();
            
            task->downloadedBytes = file.size();
            emit downloadCompleted(task->id, fullPath);
        } else {
            handleDownloadError(task, QString("Failed to write file: %1").arg(file.errorString()));
        }
    }
}

void DownloadManager::handleDownloadError(DownloadTask* task, const QString &error)
{
    if (task->retryCount < m_retryCount) {
        // 重试下载
        task->retryCount++;
        retryDownload(task);
    } else {
        // 重试次数用完，标记为失败
        emit downloadFailed(task->id, error);
    }
}

void DownloadManager::retryDownload(DownloadTask* task)
{
    // 延迟重试
    m_pendingQueue.enqueue(task->id);
    m_retryTimer->start(m_retryDelay * 1000);
}

QString DownloadManager::generateTaskId() const
{
    return QCryptographicHash::hash(
        QString("%1-%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(qrand()).toUtf8(),
        QCryptographicHash::Md5
    ).toHex();
}

QString DownloadManager::getFileNameFromUrl(const QUrl &url) const
{
    QString fileName = QFileInfo(url.path()).fileName();
    if (fileName.isEmpty()) {
        fileName = QString("download_%1").arg(QDateTime::currentMSecsSinceEpoch());
    }
    return fileName;
}

bool DownloadManager::isResumable(const QUrl &url) const
{
    // 大多数HTTP服务器支持断点续传
    // 可以通过HEAD请求检查Accept-Ranges头来确认
    return url.scheme().toLower() == "http" || url.scheme().toLower() == "https";
}

qint64 DownloadManager::getExistingFileSize(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() ? fileInfo.size() : 0;
}
