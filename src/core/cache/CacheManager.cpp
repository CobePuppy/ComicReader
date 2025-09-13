#include "../../include/core/CacheManager.h"
#include "../../include/core/ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDebug>
#include <QMutexLocker>

CacheManager* CacheManager::m_instance = nullptr;

CacheManager* CacheManager::instance()
{
    if (!m_instance) {
        m_instance = new CacheManager();
    }
    return m_instance;
}

CacheManager::CacheManager(QObject *parent)
    : QObject(parent)
    , m_maxMemoryCacheSize(100)  // 100MB
    , m_maxDiskCacheSize(500)    // 500MB
    , m_cleanupTimer(new QTimer(this))
    , m_autoCleanupInterval(30)  // 30分钟
    , m_currentMemorySize(0)
{
    // 设置缓存目录
    ConfigManager *config = ConfigManager::instance();
    m_cacheDirectory = config->getCachePath();
    ensureCacheDirectory();
    
    // 设置自动清理定时器
    m_cleanupTimer->setSingleShot(false);
    m_cleanupTimer->setInterval(m_autoCleanupInterval * 60 * 1000);
    connect(m_cleanupTimer, &QTimer::timeout, this, &CacheManager::onCleanupTimer);
    m_cleanupTimer->start();
}

CacheManager::~CacheManager()
{
    clearMemoryCache();
}

void CacheManager::cachePixmap(const QString &key, const QPixmap &pixmap)
{
    QMutexLocker locker(&m_pixmapCacheMutex);
    
    // 估算内存使用
    int pixmapSize = pixmap.width() * pixmap.height() * 4; // 假设32位色深
    
    // 检查内存限制
    if (m_currentMemorySize + pixmapSize > m_maxMemoryCacheSize * 1024 * 1024) {
        enforceMemoryLimit();
    }
    
    // 如果键已存在，先移除旧的
    if (m_pixmapCache.contains(key)) {
        m_pixmapCache.remove(key);
    }
    
    m_pixmapCache.insert(key, pixmap);
    m_cacheTimestamps.insert(key, QDateTime::currentMSecsSinceEpoch());
    m_currentMemorySize += pixmapSize;
}

QPixmap CacheManager::getCachedPixmap(const QString &key) const
{
    QMutexLocker locker(&m_pixmapCacheMutex);
    return m_pixmapCache.value(key);
}

bool CacheManager::hasPixmap(const QString &key) const
{
    QMutexLocker locker(&m_pixmapCacheMutex);
    return m_pixmapCache.contains(key);
}

void CacheManager::removeCachedPixmap(const QString &key)
{
    QMutexLocker locker(&m_pixmapCacheMutex);
    if (m_pixmapCache.contains(key)) {
        const QPixmap &pixmap = m_pixmapCache.value(key);
        int pixmapSize = pixmap.width() * pixmap.height() * 4;
        m_currentMemorySize -= pixmapSize;
        
        m_pixmapCache.remove(key);
        m_cacheTimestamps.remove(key);
    }
}

void CacheManager::cacheData(const QString &key, const QByteArray &data)
{
    QMutexLocker locker(&m_dataCacheMutex);
    
    // 检查内存限制
    if (m_currentMemorySize + data.size() > m_maxMemoryCacheSize * 1024 * 1024) {
        enforceMemoryLimit();
    }
    
    // 如果键已存在，先移除旧的
    if (m_dataCache.contains(key)) {
        m_currentMemorySize -= m_dataCache.value(key).size();
        m_dataCache.remove(key);
    }
    
    m_dataCache.insert(key, data);
    m_cacheTimestamps.insert(key, QDateTime::currentMSecsSinceEpoch());
    m_currentMemorySize += data.size();
}

QByteArray CacheManager::getCachedData(const QString &key) const
{
    QMutexLocker locker(&m_dataCacheMutex);
    return m_dataCache.value(key);
}

bool CacheManager::hasData(const QString &key) const
{
    QMutexLocker locker(&m_dataCacheMutex);
    return m_dataCache.contains(key);
}

void CacheManager::removeCachedData(const QString &key)
{
    QMutexLocker locker(&m_dataCacheMutex);
    if (m_dataCache.contains(key)) {
        m_currentMemorySize -= m_dataCache.value(key).size();
        m_dataCache.remove(key);
        m_cacheTimestamps.remove(key);
    }
}

void CacheManager::saveToDisk(const QString &key, const QByteArray &data)
{
    ensureCacheDirectory();
    
    QString fileName = generateCacheFileName(key);
    QString filePath = QDir(m_cacheDirectory).absoluteFilePath(fileName);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    } else {
        qWarning() << "Failed to write cache file:" << filePath;
    }
    
    // 检查磁盘缓存大小限制
    enforceDiskLimit();
}

QByteArray CacheManager::loadFromDisk(const QString &key) const
{
    QString fileName = generateCacheFileName(key);
    QString filePath = QDir(m_cacheDirectory).absoluteFilePath(fileName);
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }
    
    return QByteArray();
}

bool CacheManager::existsOnDisk(const QString &key) const
{
    QString fileName = generateCacheFileName(key);
    QString filePath = QDir(m_cacheDirectory).absoluteFilePath(fileName);
    return QFile::exists(filePath);
}

void CacheManager::removeFromDisk(const QString &key)
{
    QString fileName = generateCacheFileName(key);
    QString filePath = QDir(m_cacheDirectory).absoluteFilePath(fileName);
    QFile::remove(filePath);
}

void CacheManager::clearMemoryCache()
{
    {
        QMutexLocker locker(&m_pixmapCacheMutex);
        m_pixmapCache.clear();
    }
    
    {
        QMutexLocker locker(&m_dataCacheMutex);
        m_dataCache.clear();
    }
    
    m_cacheTimestamps.clear();
    m_currentMemorySize = 0;
    
    emit cacheCleared();
}

void CacheManager::clearDiskCache()
{
    QDir cacheDir(m_cacheDirectory);
    if (cacheDir.exists()) {
        QStringList files = cacheDir.entryList(QDir::Files);
        for (const QString &file : files) {
            cacheDir.remove(file);
        }
    }
    
    emit cacheCleared();
}

void CacheManager::clearAllCache()
{
    clearMemoryCache();
    clearDiskCache();
}

int CacheManager::getMemoryCacheSize() const
{
    return m_currentMemorySize / (1024 * 1024); // 返回MB
}

qint64 CacheManager::getDiskCacheSize() const
{
    qint64 totalSize = 0;
    QDir cacheDir(m_cacheDirectory);
    
    if (cacheDir.exists()) {
        QFileInfoList files = cacheDir.entryInfoList(QDir::Files);
        for (const QFileInfo &fileInfo : files) {
            totalSize += fileInfo.size();
        }
    }
    
    return totalSize / (1024 * 1024); // 返回MB
}

int CacheManager::getPixmapCacheCount() const
{
    QMutexLocker locker(&m_pixmapCacheMutex);
    return m_pixmapCache.size();
}

int CacheManager::getDataCacheCount() const
{
    QMutexLocker locker(&m_dataCacheMutex);
    return m_dataCache.size();
}

void CacheManager::setMaxMemoryCacheSize(int sizeMB)
{
    m_maxMemoryCacheSize = sizeMB;
    enforceMemoryLimit();
}

void CacheManager::setMaxDiskCacheSize(qint64 sizeMB)
{
    m_maxDiskCacheSize = sizeMB;
    enforceDiskLimit();
}

void CacheManager::setCacheDirectory(const QString &path)
{
    m_cacheDirectory = path;
    ensureCacheDirectory();
}

void CacheManager::cleanupExpiredCache()
{
    // 清理超过24小时的内存缓存
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 expireTime = 24 * 60 * 60 * 1000; // 24小时
    
    QStringList expiredKeys;
    for (auto it = m_cacheTimestamps.begin(); it != m_cacheTimestamps.end(); ++it) {
        if (currentTime - it.value() > expireTime) {
            expiredKeys.append(it.key());
        }
    }
    
    for (const QString &key : expiredKeys) {
        removeCachedPixmap(key);
        removeCachedData(key);
    }
    
    // 清理磁盘上的旧文件
    cleanupOldFiles();
}

void CacheManager::setAutoCleanupInterval(int minutes)
{
    m_autoCleanupInterval = minutes;
    m_cleanupTimer->setInterval(minutes * 60 * 1000);
}

void CacheManager::onCleanupTimer()
{
    cleanupExpiredCache();
}

void CacheManager::ensureCacheDirectory()
{
    QDir dir;
    if (!dir.exists(m_cacheDirectory)) {
        dir.mkpath(m_cacheDirectory);
    }
}

void CacheManager::cleanupOldFiles()
{
    QDir cacheDir(m_cacheDirectory);
    if (!cacheDir.exists()) return;
    
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-7); // 7天前
    QFileInfoList files = cacheDir.entryInfoList(QDir::Files);
    
    for (const QFileInfo &fileInfo : files) {
        if (fileInfo.lastModified() < cutoffTime) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

void CacheManager::enforceMemoryLimit()
{
    int maxSizeBytes = m_maxMemoryCacheSize * 1024 * 1024;
    
    while (m_currentMemorySize > maxSizeBytes && (!m_pixmapCache.isEmpty() || !m_dataCache.isEmpty())) {
        // 找到最旧的缓存项并移除
        QString oldestKey;
        qint64 oldestTime = QDateTime::currentMSecsSinceEpoch();
        
        for (auto it = m_cacheTimestamps.begin(); it != m_cacheTimestamps.end(); ++it) {
            if (it.value() < oldestTime) {
                oldestTime = it.value();
                oldestKey = it.key();
            }
        }
        
        if (!oldestKey.isEmpty()) {
            removeCachedPixmap(oldestKey);
            removeCachedData(oldestKey);
        } else {
            break;
        }
    }
    
    if (m_currentMemorySize > maxSizeBytes) {
        emit cacheSizeLimitReached();
    }
}

void CacheManager::enforceDiskLimit()
{
    qint64 maxSizeBytes = m_maxDiskCacheSize * 1024 * 1024;
    qint64 currentSize = getDiskCacheSize() * 1024 * 1024;
    
    if (currentSize > maxSizeBytes) {
        // 删除最旧的文件直到符合限制
        QDir cacheDir(m_cacheDirectory);
        QFileInfoList files = cacheDir.entryInfoList(QDir::Files, QDir::Time);
        
        for (const QFileInfo &fileInfo : files) {
            if (currentSize <= maxSizeBytes) break;
            
            currentSize -= fileInfo.size();
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

QString CacheManager::generateCacheFileName(const QString &key) const
{
    // 使用MD5哈希生成文件名，避免特殊字符问题
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(key.toUtf8());
    return hash.result().toHex() + ".cache";
}
