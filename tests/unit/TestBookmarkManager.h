#ifndef TESTBOOKMARKMANAGER_H
#define TESTBOOKMARKMANAGER_H

#include <QObject>
#include <QTest>
#include <QTemporaryDir>
#include <QSignalSpy>
#include "../../include/core/bookmark/BookmarkManager.h"

class TestBookmarkManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 书签基本功能测试
    void testAddBookmark();
    void testRemoveBookmark();
    void testUpdateBookmark();
    void testGetBookmark();
    
    // 书签查询测试
    void testGetAllBookmarksForComic();
    void testGetRecentBookmarks();
    void testSearchBookmarks();
    
    // 快速书签测试
    void testQuickBookmarks();
    
    // 标签管理测试
    void testTagManagement();
    
    // 阅读进度测试
    void testReadingProgress();
    void testProgressStatistics();
    
    // 导入导出测试
    void testExportImportBookmarks();
    void testExportImportProgress();
    
    // 清理功能测试
    void testCleanupFunctions();
    
    // 信号测试
    void testSignals();

private:
    BookmarkManager *m_bookmarkManager;
    QTemporaryDir *m_tempDir;
    
    QString createTestComicFile(const QString &name);
    BookmarkInfo createTestBookmark(const QString &comicPath, int pageNumber);
};

#endif // TESTBOOKMARKMANAGER_H