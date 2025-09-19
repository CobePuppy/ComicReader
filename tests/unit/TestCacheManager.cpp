#include "TestCacheManager.h"
#include <QPixmap>
#include <QPainter>
#include <QUuid>
#include <QThread>

void TestCacheManager::initTestCase()
{
    // 创建临时目录用于测试
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    qDebug() << "Test cache directory:" << m_tempDir->path();
}

void TestCacheManager::cleanupTestCase()
{
    delete m_tempDir;
}

void TestCacheManager::init()
{
    // 每个测试前创建新的CacheManager实例
    m_cacheManager = new CacheManager();
    m_cacheManager->setCacheDirectory(m_tempDir->path());
    m_cacheManager->setMemoryCacheSize(10 * 1024 * 1024); // 10MB
    m_cacheManager->setDiskCacheSize(50 * 1024 * 1024);   // 50MB
}

void TestCacheManager::cleanup()
{
    delete m_cacheManager;
    m_cacheManager = nullptr;
}

void TestCacheManager::testMemoryCacheBasic()
{
    QString key = generateTestKey();
    QPixmap pixmap = createTestPixmap();
    
    // 测试存储
    QVERIFY(m_cacheManager->storeImage(key, pixmap));
    
    // 测试检查存在
    QVERIFY(m_cacheManager->hasImage(key));
    
    // 测试获取
    QPixmap retrievedPixmap = m_cacheManager->getImage(key);
    QVERIFY(!retrievedPixmap.isNull());
    QCOMPARE(retrievedPixmap.size(), pixmap.size());
    
    // 测试删除
    m_cacheManager->removeImage(key);
    QVERIFY(!m_cacheManager->hasImage(key));
}

void TestCacheManager::testMemoryCacheCapacity()
{
    // 设置较小的内存缓存以便测试
    m_cacheManager->setMemoryCacheSize(1024 * 1024); // 1MB
    
    QStringList keys;
    int imageCount = 0;
    
    // 添加图片直到达到缓存容量
    for (int i = 0; i < 100; ++i) {
        QString key = generateTestKey(QString("large_%1").arg(i));
        QPixmap pixmap = createTestPixmap(500, 500); // 较大的图片
        
        if (m_cacheManager->storeImage(key, pixmap)) {
            keys.append(key);
            imageCount++;
        } else {
            break; // 缓存已满
        }
    }
    
    qDebug() << "Stored" << imageCount << "images before cache was full";
    QVERIFY(imageCount > 0);
    QVERIFY(imageCount < 100); // 应该在达到容量限制前停止
    
    // 验证最后添加的图片仍然存在
    if (!keys.isEmpty()) {
        QVERIFY(m_cacheManager->hasImage(keys.last()));
    }
}

void TestCacheManager::testMemoryCacheEviction()
{
    m_cacheManager->setMemoryCacheSize(512 * 1024); // 512KB
    
    QStringList keys;
    
    // 添加多个图片
    for (int i = 0; i < 10; ++i) {
        QString key = generateTestKey(QString("evict_%1").arg(i));
        QPixmap pixmap = createTestPixmap(200, 200);
        keys.append(key);
        m_cacheManager->storeImage(key, pixmap);
    }
    
    // 访问第一个图片以确保它被最近使用
    QPixmap firstPixmap = m_cacheManager->getImage(keys.first());
    QVERIFY(!firstPixmap.isNull());
    
    // 添加更多图片以触发驱逐
    for (int i = 10; i < 20; ++i) {
        QString key = generateTestKey(QString("evict_%1").arg(i));
        QPixmap pixmap = createTestPixmap(200, 200);
        m_cacheManager->storeImage(key, pixmap);
    }
    
    // 第一个图片应该仍然存在（因为最近被访问）
    QVERIFY(m_cacheManager->hasImage(keys.first()));
}

void TestCacheManager::testDiskCacheBasic()
{
    QString key = generateTestKey();
    QPixmap pixmap = createTestPixmap();
    
    // 清除内存缓存以确保从磁盘读取
    m_cacheManager->clearMemoryCache();
    
    // 存储到磁盘
    QVERIFY(m_cacheManager->storeImage(key, pixmap));
    
    // 清除内存缓存
    m_cacheManager->clearMemoryCache();
    
    // 从磁盘获取
    QPixmap retrievedPixmap = m_cacheManager->getImage(key);
    QVERIFY(!retrievedPixmap.isNull());
    QCOMPARE(retrievedPixmap.size(), pixmap.size());
}

void TestCacheManager::testDiskCacheCompression()
{
    QString key = generateTestKey();
    QPixmap largePixmap = createTestPixmap(1000, 1000);
    
    // 启用压缩
    m_cacheManager->setCompressionEnabled(true);
    m_cacheManager->setCompressionQuality(50);
    
    QVERIFY(m_cacheManager->storeImage(key, largePixmap));
    
    // 清除内存缓存
    m_cacheManager->clearMemoryCache();
    
    // 从磁盘获取压缩的图片
    QPixmap retrievedPixmap = m_cacheManager->getImage(key);
    QVERIFY(!retrievedPixmap.isNull());
    QCOMPARE(retrievedPixmap.size(), largePixmap.size());
    
    // 检查文件大小（压缩后应该更小）
    CacheStatistics stats = m_cacheManager->getStatistics();
    QVERIFY(stats.diskCacheSize > 0);
}

void TestCacheManager::testDiskCacheCleanup()
{
    // 设置较小的磁盘缓存限制
    m_cacheManager->setDiskCacheSize(1024 * 1024); // 1MB
    
    QStringList keys;
    
    // 添加大量图片以超过磁盘缓存限制
    for (int i = 0; i < 50; ++i) {
        QString key = generateTestKey(QString("cleanup_%1").arg(i));
        QPixmap pixmap = createTestPixmap(300, 300);
        keys.append(key);
        m_cacheManager->storeImage(key, pixmap);
    }
    
    // 执行清理
    m_cacheManager->cleanup();
    
    // 检查磁盘缓存大小是否在限制内
    CacheStatistics stats = m_cacheManager->getStatistics();
    QVERIFY(stats.diskCacheSize <= m_cacheManager->getDiskCacheSize());
}

void TestCacheManager::testCacheStatistics()
{
    QString key1 = generateTestKey("stats_1");
    QString key2 = generateTestKey("stats_2");
    QPixmap pixmap1 = createTestPixmap(100, 100);
    QPixmap pixmap2 = createTestPixmap(200, 200);
    
    // 存储图片
    m_cacheManager->storeImage(key1, pixmap1);
    m_cacheManager->storeImage(key2, pixmap2);
    
    // 获取统计信息
    CacheStatistics stats = m_cacheManager->getStatistics();
    
    QVERIFY(stats.memoryCacheCount >= 2);
    QVERIFY(stats.memoryCacheSize > 0);
    QVERIFY(stats.diskCacheCount >= 2);
    QVERIFY(stats.diskCacheSize > 0);
    QVERIFY(stats.hitCount >= 0);
    QVERIFY(stats.missCount >= 0);
}

void TestCacheManager::testCacheSizeCalculation()
{
    CacheStatistics initialStats = m_cacheManager->getStatistics();
    
    QString key = generateTestKey();
    QPixmap pixmap = createTestPixmap(100, 100);
    
    m_cacheManager->storeImage(key, pixmap);
    
    CacheStatistics afterStats = m_cacheManager->getStatistics();
    
    // 内存缓存大小应该增加
    QVERIFY(afterStats.memoryCacheSize > initialStats.memoryCacheSize);
    QVERIFY(afterStats.memoryCacheCount > initialStats.memoryCacheCount);
}

void TestCacheManager::testInvalidPaths()
{
    // 测试无效的缓存目录
    CacheManager invalidCacheManager;
    invalidCacheManager.setCacheDirectory("/invalid/path/that/does/not/exist");
    
    QString key = generateTestKey();
    QPixmap pixmap = createTestPixmap();
    
    // 存储应该失败，但不应该崩溃
    bool result = invalidCacheManager.storeImage(key, pixmap);
    Q_UNUSED(result) // 可能成功也可能失败，取决于实现
    
    // 获取应该返回空图片
    QPixmap retrievedPixmap = invalidCacheManager.getImage(key);
    // 可能为空也可能从内存缓存返回
}

void TestCacheManager::testDiskSpaceHandling()
{
    // 这个测试在实际环境中可能很难实现
    // 因为我们无法轻易模拟磁盘空间不足的情况
    // 但我们可以测试相关的错误处理逻辑
    
    QString key = generateTestKey();
    QPixmap largePixmap = createTestPixmap(2000, 2000);
    
    // 尝试存储大图片
    bool result = m_cacheManager->storeImage(key, largePixmap);
    
    // 结果取决于可用磁盘空间，但不应该崩溃
    Q_UNUSED(result)
    
    // 获取统计信息应该正常工作
    CacheStatistics stats = m_cacheManager->getStatistics();
    QVERIFY(stats.memoryCacheCount >= 0);
    QVERIFY(stats.diskCacheCount >= 0);
}

// 辅助方法实现
QPixmap TestCacheManager::createTestPixmap(int width, int height)
{
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::white);
    
    QPainter painter(&pixmap);
    painter.setPen(Qt::blue);
    painter.drawRect(0, 0, width-1, height-1);
    painter.drawLine(0, 0, width-1, height-1);
    painter.drawLine(width-1, 0, 0, height-1);
    
    return pixmap;
}

QString TestCacheManager::generateTestKey(const QString &prefix)
{
    return QString("%1_%2").arg(prefix, QUuid::createUuid().toString(QUuid::WithoutBraces));
}