#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QMutex>
#include <QTimer>
#include <QDir>

/**
 * @brief 缓存管理器
 * 管理图片缓存、临时文件等
 */
class CacheManager : public QObject
{
    Q_OBJECT

public:
    static CacheManager* instance();
    
    // 图片缓存
    void cachePixmap(const QString &key, const QPixmap &pixmap);
    QPixmap getCachedPixmap(const QString &key) const;
    bool hasPixmap(const QString &key) const;
    void removeCachedPixmap(const QString &key);
    
    // 数据缓存
    void cacheData(const QString &key, const QByteArray &data);
    QByteArray getCachedData(const QString &key) const;
    bool hasData(const QString &key) const;
    void removeCachedData(const QString &key);
    
    // 磁盘缓存
    void saveToDisk(const QString &key, const QByteArray &data);
    QByteArray loadFromDisk(const QString &key) const;
    bool existsOnDisk(const QString &key) const;
    void removeFromDisk(const QString &key);
    
    // 缓存管理
    void clearMemoryCache();
    void clearDiskCache();
    void clearAllCache();
    
    // 缓存统计
    int getMemoryCacheSize() const;
    qint64 getDiskCacheSize() const;
    int getPixmapCacheCount() const;
    int getDataCacheCount() const;
    
    // 设置缓存限制
    void setMaxMemoryCacheSize(int sizeMB);
    void setMaxDiskCacheSize(qint64 sizeMB);
    void setCacheDirectory(const QString &path);
    
    // 缓存清理
    void cleanupExpiredCache();
    void setAutoCleanupInterval(int minutes);

signals:
    void cacheCleared();
    void cacheSizeLimitReached();

private slots:
    void onCleanupTimer();

private:
    explicit CacheManager(QObject *parent = nullptr);
    ~CacheManager();
    
    void ensureCacheDirectory();
    void cleanupOldFiles();
    void enforceMemoryLimit();
    void enforceDiskLimit();
    QString generateCacheFileName(const QString &key) const;
    
    static CacheManager *m_instance;
    
    // 内存缓存
    mutable QMutex m_pixmapCacheMutex;
    mutable QMutex m_dataCacheMutex;
    QHash<QString, QPixmap> m_pixmapCache;
    QHash<QString, QByteArray> m_dataCache;
    QHash<QString, qint64> m_cacheTimestamps;
    
    // 缓存设置
    int m_maxMemoryCacheSize;      // MB
    qint64 m_maxDiskCacheSize;     // MB
    QString m_cacheDirectory;
    
    // 清理定时器
    QTimer *m_cleanupTimer;
    int m_autoCleanupInterval;     // minutes
    
    // 当前缓存大小（估算）
    mutable int m_currentMemorySize;
};

#endif // CACHEMANAGER_H
