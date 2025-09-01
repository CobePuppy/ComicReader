#include <QtTest>
#include "../include/utils/FileUtils.h"

class TestFileUtils : public QObject
{
    Q_OBJECT

private slots:
    void testIsComicFile();
    void testIsImageFile();
    void testFormatFileSize();
    void testGenerateSafeFileName();
};

void TestFileUtils::testIsComicFile()
{
    QVERIFY(FileUtils::isComicFile("test.cbz"));
    QVERIFY(FileUtils::isComicFile("test.CBR"));
    QVERIFY(FileUtils::isComicFile("test.zip"));
    QVERIFY(!FileUtils::isComicFile("test.txt"));
    QVERIFY(!FileUtils::isComicFile("test.jpg"));
}

void TestFileUtils::testIsImageFile()
{
    QVERIFY(FileUtils::isImageFile("test.jpg"));
    QVERIFY(FileUtils::isImageFile("test.PNG"));
    QVERIFY(FileUtils::isImageFile("test.gif"));
    QVERIFY(!FileUtils::isImageFile("test.txt"));
    QVERIFY(!FileUtils::isImageFile("test.cbz"));
}

void TestFileUtils::testFormatFileSize()
{
    QCOMPARE(FileUtils::formatFileSize(0), QString("0 B"));
    QCOMPARE(FileUtils::formatFileSize(1023), QString("1023 B"));
    QCOMPARE(FileUtils::formatFileSize(1024), QString("1.00 KB"));
    QCOMPARE(FileUtils::formatFileSize(1048576), QString("1.00 MB"));
}

void TestFileUtils::testGenerateSafeFileName()
{
    QCOMPARE(FileUtils::generateSafeFileName("normal.txt"), QString("normal.txt"));
    QCOMPARE(FileUtils::generateSafeFileName("file<>:name.txt"), QString("file___name.txt"));
    QCOMPARE(FileUtils::generateSafeFileName("  spaced  "), QString("spaced"));
}

QTEST_MAIN(TestFileUtils)
#include "TestFileUtils.moc"
