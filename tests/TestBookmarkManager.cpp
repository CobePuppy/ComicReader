#include "TestBookmarkManager.h"
#include "../src/managers/BookmarkManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QSignalSpy>

void TestBookmarkManager::initTestCase()
{
    // 设置测试目录
    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReaderTest";
    QDir().mkpath(testDir);
    QDir::setCurrent(testDir);
    
    bookmarkManager = new BookmarkManager(this);
}

void TestBookmarkManager::cleanupTestCase()
{
    delete bookmarkManager;
    
    // 清理测试文件
    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReaderTest";
    QDir(testDir).removeRecursively();
}

void TestBookmarkManager::init()
{
    // 每个测试前清理数据
    bookmarkManager->clearAllBookmarks();
}

void TestBookmarkManager::testAddBookmark()
{
    BookmarkInfo bookmark;
    bookmark.id = "test-bookmark-1";
    bookmark.filePath = "/test/comic.cbz";
    bookmark.pageIndex = 5;
    bookmark.title = "Test Bookmark";
    bookmark.description = "This is a test bookmark";
    bookmark.tags = QStringList() << "action" << "adventure";
    bookmark.timestamp = QDateTime::currentDateTime();
    
    QSignalSpy spy(bookmarkManager, &BookmarkManager::bookmarkAdded);
    
    bookmarkManager->addBookmark(bookmark);
    
    QCOMPARE(spy.count(), 1);
    
    QList<BookmarkInfo> bookmarks = bookmarkManager->getAllBookmarks();
    QCOMPARE(bookmarks.size(), 1);
    QCOMPARE(bookmarks.first().title, QString("Test Bookmark"));
    QCOMPARE(bookmarks.first().pageIndex, 5);
    QCOMPARE(bookmarks.first().tags.size(), 2);
}

void TestBookmarkManager::testRemoveBookmark()
{
    // 先添加一个书签
    BookmarkInfo bookmark;
    bookmark.id = "test-bookmark-2";
    bookmark.filePath = "/test/comic.cbz";
    bookmark.pageIndex = 10;
    bookmark.title = "To be removed";
    
    bookmarkManager->addBookmark(bookmark);
    QCOMPARE(bookmarkManager->getAllBookmarks().size(), 1);
    
    QSignalSpy spy(bookmarkManager, &BookmarkManager::bookmarkRemoved);
    
    bookmarkManager->removeBookmark("test-bookmark-2");
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(bookmarkManager->getAllBookmarks().size(), 0);
}

void TestBookmarkManager::testUpdateBookmark()
{
    // 添加初始书签
    BookmarkInfo bookmark;
    bookmark.id = "test-bookmark-3";
    bookmark.filePath = "/test/comic.cbz";
    bookmark.pageIndex = 15;
    bookmark.title = "Original Title";
    
    bookmarkManager->addBookmark(bookmark);
    
    // 更新书签
    bookmark.title = "Updated Title";
    bookmark.description = "New description";
    bookmark.tags = QStringList() << "updated";
    
    QSignalSpy spy(bookmarkManager, &BookmarkManager::bookmarkUpdated);
    
    bookmarkManager->updateBookmark(bookmark);
    
    QCOMPARE(spy.count(), 1);
    
    QList<BookmarkInfo> bookmarks = bookmarkManager->getAllBookmarks();
    QCOMPARE(bookmarks.size(), 1);
    QCOMPARE(bookmarks.first().title, QString("Updated Title"));
    QCOMPARE(bookmarks.first().description, QString("New description"));
    QCOMPARE(bookmarks.first().tags.first(), QString("updated"));
}

void TestBookmarkManager::testSearchBookmarks()
{
    // 添加多个书签
    BookmarkInfo bookmark1;
    bookmark1.id = "search-test-1";
    bookmark1.title = "Action Comic";
    bookmark1.tags = QStringList() << "action" << "superhero";
    bookmarkManager->addBookmark(bookmark1);
    
    BookmarkInfo bookmark2;
    bookmark2.id = "search-test-2";
    bookmark2.title = "Romance Novel";
    bookmark2.tags = QStringList() << "romance" << "drama";
    bookmarkManager->addBookmark(bookmark2);
    
    BookmarkInfo bookmark3;
    bookmark3.id = "search-test-3";
    bookmark3.title = "Adventure Story";
    bookmark3.description = "Epic action adventure";
    bookmark3.tags = QStringList() << "adventure" << "action";
    bookmarkManager->addBookmark(bookmark3);
    
    // 测试标题搜索
    QList<BookmarkInfo> results = bookmarkManager->searchBookmarks("Action");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().title, QString("Action Comic"));
    
    // 测试描述搜索
    results = bookmarkManager->searchBookmarks("Epic");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().title, QString("Adventure Story"));
    
    // 测试标签搜索
    results = bookmarkManager->searchBookmarks("romance");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().title, QString("Romance Novel"));
    
    // 测试多结果搜索
    results = bookmarkManager->searchBookmarks("action");
    QCOMPARE(results.size(), 2);
}

void TestBookmarkManager::testFilterByTags()
{
    // 添加测试数据
    BookmarkInfo bookmark1;
    bookmark1.id = "filter-test-1";
    bookmark1.tags = QStringList() << "action" << "superhero";
    bookmarkManager->addBookmark(bookmark1);
    
    BookmarkInfo bookmark2;
    bookmark2.id = "filter-test-2";
    bookmark2.tags = QStringList() << "romance";
    bookmarkManager->addBookmark(bookmark2);
    
    BookmarkInfo bookmark3;
    bookmark3.id = "filter-test-3";
    bookmark3.tags = QStringList() << "action" << "adventure";
    bookmarkManager->addBookmark(bookmark3);
    
    // 测试单标签过滤
    QList<BookmarkInfo> results = bookmarkManager->filterByTags(QStringList() << "action");
    QCOMPARE(results.size(), 2);
    
    // 测试多标签过滤（AND操作）
    results = bookmarkManager->filterByTags(QStringList() << "action" << "superhero");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().id, QString("filter-test-1"));
    
    // 测试不存在的标签
    results = bookmarkManager->filterByTags(QStringList() << "nonexistent");
    QCOMPARE(results.size(), 0);
}

void TestBookmarkManager::testReadingProgress()
{
    QString filePath = "/test/comic.cbz";
    
    // 测试初始状态
    ReadingProgress progress = bookmarkManager->getReadingProgress(filePath);
    QCOMPARE(progress.filePath, QString());
    QCOMPARE(progress.currentPage, 0);
    
    QSignalSpy spy(bookmarkManager, &BookmarkManager::readingProgressUpdated);
    
    // 更新阅读进度
    bookmarkManager->updateReadingProgress(filePath, 10, 100);
    
    QCOMPARE(spy.count(), 1);
    
    progress = bookmarkManager->getReadingProgress(filePath);
    QCOMPARE(progress.filePath, filePath);
    QCOMPARE(progress.currentPage, 10);
    QCOMPARE(progress.totalPages, 100);
    QCOMPARE(progress.progress, 0.1);
    
    // 再次更新
    bookmarkManager->updateReadingProgress(filePath, 50, 100);
    progress = bookmarkManager->getReadingProgress(filePath);
    QCOMPARE(progress.currentPage, 50);
    QCOMPARE(progress.progress, 0.5);
}

void TestBookmarkManager::testImportExport()
{
    // 添加测试数据
    BookmarkInfo bookmark1;
    bookmark1.id = "export-test-1";
    bookmark1.title = "Export Test 1";
    bookmark1.tags = QStringList() << "test";
    bookmarkManager->addBookmark(bookmark1);
    
    BookmarkInfo bookmark2;
    bookmark2.id = "export-test-2";
    bookmark2.title = "Export Test 2";
    bookmark2.tags = QStringList() << "test";
    bookmarkManager->addBookmark(bookmark2);
    
    QString exportFile = "test_bookmarks.json";
    
    // 测试导出
    bool exportResult = bookmarkManager->exportBookmarks(exportFile);
    QVERIFY(exportResult);
    QVERIFY(QFile::exists(exportFile));
    
    // 清理数据
    bookmarkManager->clearAllBookmarks();
    QCOMPARE(bookmarkManager->getAllBookmarks().size(), 0);
    
    // 测试导入
    bool importResult = bookmarkManager->importBookmarks(exportFile);
    QVERIFY(importResult);
    
    QList<BookmarkInfo> importedBookmarks = bookmarkManager->getAllBookmarks();
    QCOMPARE(importedBookmarks.size(), 2);
    
    // 验证导入的数据
    bool found1 = false, found2 = false;
    for (const auto &bookmark : importedBookmarks) {
        if (bookmark.title == "Export Test 1") found1 = true;
        if (bookmark.title == "Export Test 2") found2 = true;
    }
    QVERIFY(found1);
    QVERIFY(found2);
    
    // 清理文件
    QFile::remove(exportFile);
}

void TestBookmarkManager::testAutoCleanup()
{
    // 添加多个书签，超过限制
    for (int i = 0; i < 150; i++) {
        BookmarkInfo bookmark;
        bookmark.id = QString("cleanup-test-%1").arg(i);
        bookmark.title = QString("Bookmark %1").arg(i);
        bookmark.timestamp = QDateTime::currentDateTime().addDays(-i);
        bookmarkManager->addBookmark(bookmark);
    }
    
    QVERIFY(bookmarkManager->getAllBookmarks().size() > 100);
    
    // 触发自动清理
    bookmarkManager->autoCleanupOldBookmarks(100);
    
    // 验证清理结果
    QList<BookmarkInfo> remainingBookmarks = bookmarkManager->getAllBookmarks();
    QVERIFY(remainingBookmarks.size() <= 100);
    
    // 验证保留的是最新的书签
    for (const auto &bookmark : remainingBookmarks) {
        QVERIFY(bookmark.timestamp >= QDateTime::currentDateTime().addDays(-100));
    }
}