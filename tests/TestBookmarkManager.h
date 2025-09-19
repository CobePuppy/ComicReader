#pragma once

#include <QObject>
#include <QtTest>

class BookmarkManager;

class TestBookmarkManager : public QObject
{
    Q_OBJECT

public:
    TestBookmarkManager() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    
    void testAddBookmark();
    void testRemoveBookmark();
    void testUpdateBookmark();
    void testSearchBookmarks();
    void testFilterByTags();
    void testReadingProgress();
    void testImportExport();
    void testAutoCleanup();

private:
    BookmarkManager *bookmarkManager = nullptr;
};