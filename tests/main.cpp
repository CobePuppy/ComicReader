#include <QCoreApplication>
#include <QTest>
#include <QDebug>

#include "unit/TestCacheManager.h"
#include "unit/TestBookmarkManager.h"
#include "unit/TestErrorHandler.h"
#include "unit/TestConfigManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    int result = 0;
    
    qDebug() << "Running ComicReader Unit Tests...";
    qDebug() << "================================";
    
    // 运行CacheManager测试
    {
        TestCacheManager test;
        result += QTest::qExec(&test, argc, argv);
    }
    
    // 运行BookmarkManager测试
    {
        TestBookmarkManager test;
        result += QTest::qExec(&test, argc, argv);
    }
    
    // 运行ErrorHandler测试
    {
        TestErrorHandler test;
        result += QTest::qExec(&test, argc, argv);
    }
    
    // 运行ConfigManager测试
    {
        TestConfigManager test;
        result += QTest::qExec(&test, argc, argv);
    }
    
    qDebug() << "================================";
    if (result == 0) {
        qDebug() << "All tests passed!";
    } else {
        qDebug() << "Some tests failed. Exit code:" << result;
    }
    
    return result;
}