#ifndef TESTCACHEMANAGER_H
#define TESTCACHEMANAGER_H

#include <QObject>
#include <QTest>
#include <QTemporaryDir>
#include <QPixmap>
#include "../../include/core/CacheManager.h"

class TestCacheManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 内存缓存测试
    void testMemoryCacheBasic();
    void testMemoryCacheCapacity();
    void testMemoryCacheEviction();
    
    // 磁盘缓存测试
    void testDiskCacheBasic();
    void testDiskCacheCompression();
    void testDiskCacheCleanup();
    
    // 缓存统计测试
    void testCacheStatistics();
    void testCacheSizeCalculation();
    
    // 错误处理测试
    void testInvalidPaths();
    void testDiskSpaceHandling();

private:
    CacheManager *m_cacheManager;
    QTemporaryDir *m_tempDir;
    
    QPixmap createTestPixmap(int width = 100, int height = 100);
    QString generateTestKey(const QString &prefix = "test");
};

#endif // TESTCACHEMANAGER_H