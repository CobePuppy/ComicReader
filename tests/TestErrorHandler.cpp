#include "TestErrorHandler.h"
#include "../src/managers/ErrorHandler.h"
#include <QStandardPaths>
#include <QDir>
#include <QSignalSpy>
#include <QThread>

void TestErrorHandler::initTestCase()
{
    // 设置测试目录
    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReaderErrorTest";
    QDir().mkpath(testDir);
    QDir::setCurrent(testDir);
    
    errorHandler = &ErrorHandler::instance();
}

void TestErrorHandler::cleanupTestCase()
{
    // 清理测试文件
    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReaderErrorTest";
    QDir(testDir).removeRecursively();
}

void TestErrorHandler::init()
{
    // 每个测试前清理错误记录
    errorHandler->clearErrors();
}

void TestErrorHandler::testReportError()
{
    QSignalSpy spy(errorHandler, &ErrorHandler::errorReported);
    
    // 报告一个错误
    errorHandler->reportError(ErrorSeverity::Error, ErrorCategory::FileSystem,
                             "Test error message", "Detailed error information");
    
    QCOMPARE(spy.count(), 1);
    
    // 检查错误记录
    QList<ErrorInfo> errors = errorHandler->getAllErrors();
    QCOMPARE(errors.size(), 1);
    
    const ErrorInfo &error = errors.first();
    QCOMPARE(error.severity, ErrorSeverity::Error);
    QCOMPARE(error.category, ErrorCategory::FileSystem);
    QCOMPARE(error.message, QString("Test error message"));
    QCOMPARE(error.details, QString("Detailed error information"));
    QVERIFY(!error.id.isEmpty());
    QVERIFY(error.timestamp.isValid());
}

void TestErrorHandler::testErrorCategorization()
{
    // 报告不同类别的错误
    errorHandler->reportError(ErrorSeverity::Error, ErrorCategory::FileSystem, "File error");
    errorHandler->reportError(ErrorSeverity::Warning, ErrorCategory::Network, "Network warning");
    errorHandler->reportError(ErrorSeverity::Critical, ErrorCategory::Memory, "Memory critical");
    errorHandler->reportError(ErrorSeverity::Info, ErrorCategory::UI, "UI info");
    
    // 按类别过滤
    QList<ErrorInfo> fileErrors = errorHandler->getErrorsByCategory(ErrorCategory::FileSystem);
    QCOMPARE(fileErrors.size(), 1);
    QCOMPARE(fileErrors.first().message, QString("File error"));
    
    QList<ErrorInfo> networkErrors = errorHandler->getErrorsByCategory(ErrorCategory::Network);
    QCOMPARE(networkErrors.size(), 1);
    QCOMPARE(networkErrors.first().message, QString("Network warning"));
    
    QList<ErrorInfo> memoryErrors = errorHandler->getErrorsByCategory(ErrorCategory::Memory);
    QCOMPARE(memoryErrors.size(), 1);
    QCOMPARE(memoryErrors.first().message, QString("Memory critical"));
    
    QList<ErrorInfo> uiErrors = errorHandler->getErrorsByCategory(ErrorCategory::UI);
    QCOMPARE(uiErrors.size(), 1);
    QCOMPARE(uiErrors.first().message, QString("UI info"));
}

void TestErrorHandler::testErrorFiltering()
{
    // 添加不同严重程度的错误
    errorHandler->reportError(ErrorSeverity::Info, ErrorCategory::FileSystem, "Info message");
    errorHandler->reportError(ErrorSeverity::Warning, ErrorCategory::Network, "Warning message");
    errorHandler->reportError(ErrorSeverity::Error, ErrorCategory::Archive, "Error message");
    errorHandler->reportError(ErrorSeverity::Critical, ErrorCategory::Memory, "Critical message");
    
    // 按严重程度过滤
    QList<ErrorInfo> infoErrors = errorHandler->getErrorsBySeverity(ErrorSeverity::Info);
    QCOMPARE(infoErrors.size(), 1);
    QCOMPARE(infoErrors.first().message, QString("Info message"));
    
    QList<ErrorInfo> warningErrors = errorHandler->getErrorsBySeverity(ErrorSeverity::Warning);
    QCOMPARE(warningErrors.size(), 1);
    QCOMPARE(warningErrors.first().message, QString("Warning message"));
    
    QList<ErrorInfo> errorErrors = errorHandler->getErrorsBySeverity(ErrorSeverity::Error);
    QCOMPARE(errorErrors.size(), 1);
    QCOMPARE(errorErrors.first().message, QString("Error message"));
    
    QList<ErrorInfo> criticalErrors = errorHandler->getErrorsBySeverity(ErrorSeverity::Critical);
    QCOMPARE(criticalErrors.size(), 1);
    QCOMPARE(criticalErrors.first().message, QString("Critical message"));
}

void TestErrorHandler::testErrorStatistics()
{
    // 添加各种错误
    errorHandler->reportError(ErrorSeverity::Info, ErrorCategory::FileSystem, "Info 1");
    errorHandler->reportError(ErrorSeverity::Info, ErrorCategory::Network, "Info 2");
    errorHandler->reportError(ErrorSeverity::Warning, ErrorCategory::FileSystem, "Warning 1");
    errorHandler->reportError(ErrorSeverity::Error, ErrorCategory::Archive, "Error 1");
    errorHandler->reportError(ErrorSeverity::Critical, ErrorCategory::Memory, "Critical 1");
    
    ErrorStatistics stats = errorHandler->getStatistics();
    
    // 验证按严重程度统计
    QCOMPARE(stats.bySeverity[ErrorSeverity::Info], 2);
    QCOMPARE(stats.bySeverity[ErrorSeverity::Warning], 1);
    QCOMPARE(stats.bySeverity[ErrorSeverity::Error], 1);
    QCOMPARE(stats.bySeverity[ErrorSeverity::Critical], 1);
    
    // 验证按类别统计
    QCOMPARE(stats.byCategory[ErrorCategory::FileSystem], 2);
    QCOMPARE(stats.byCategory[ErrorCategory::Network], 1);
    QCOMPARE(stats.byCategory[ErrorCategory::Archive], 1);
    QCOMPARE(stats.byCategory[ErrorCategory::Memory], 1);
    
    // 验证总数
    QCOMPARE(stats.totalCount, 5);
}

void TestErrorHandler::testErrorCleanup()
{
    // 添加大量错误
    for (int i = 0; i < 150; i++) {
        errorHandler->reportError(ErrorSeverity::Info, ErrorCategory::FileSystem,
                                 QString("Test error %1").arg(i));
        QThread::msleep(1); // 确保时间戳不同
    }
    
    QCOMPARE(errorHandler->getAllErrors().size(), 150);
    
    // 执行清理，保留最新的100个
    errorHandler->cleanupOldErrors(100);
    
    QList<ErrorInfo> remainingErrors = errorHandler->getAllErrors();
    QVERIFY(remainingErrors.size() <= 100);
    
    // 验证保留的是最新的错误
    for (int i = 0; i < remainingErrors.size() - 1; i++) {
        QVERIFY(remainingErrors[i].timestamp >= remainingErrors[i + 1].timestamp);
    }
}

void TestErrorHandler::testErrorNotification()
{
    // 测试系统通知是否启用
    errorHandler->setNotificationEnabled(true);
    QVERIFY(errorHandler->isNotificationEnabled());
    
    errorHandler->setNotificationEnabled(false);
    QVERIFY(!errorHandler->isNotificationEnabled());
    
    // 测试通知级别设置
    errorHandler->setNotificationLevel(ErrorSeverity::Warning);
    QCOMPARE(errorHandler->getNotificationLevel(), ErrorSeverity::Warning);
    
    // 测试信号发送
    errorHandler->setNotificationEnabled(true);
    errorHandler->setNotificationLevel(ErrorSeverity::Error);
    
    QSignalSpy notificationSpy(errorHandler, &ErrorHandler::notificationRequested);
    
    // 报告低于通知级别的错误，不应该发送通知
    errorHandler->reportError(ErrorSeverity::Warning, ErrorCategory::UI, "Warning message");
    QCOMPARE(notificationSpy.count(), 0);
    
    // 报告达到通知级别的错误，应该发送通知
    errorHandler->reportError(ErrorSeverity::Error, ErrorCategory::FileSystem, "Error message");
    QCOMPARE(notificationSpy.count(), 1);
    
    // 报告更高级别的错误，应该发送通知
    errorHandler->reportError(ErrorSeverity::Critical, ErrorCategory::Memory, "Critical message");
    QCOMPARE(notificationSpy.count(), 2);
}

void TestErrorHandler::testErrorPersistence()
{
    // 添加错误
    errorHandler->reportError(ErrorSeverity::Error, ErrorCategory::FileSystem,
                             "Persistent error", "This should be saved");
    
    QString logFile = "test_error.log";
    
    // 导出错误到文件
    bool exported = errorHandler->exportErrorsToFile(logFile);
    QVERIFY(exported);
    QVERIFY(QFile::exists(logFile));
    
    // 验证文件内容
    QFile file(logFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    
    QString content = file.readAll();
    QVERIFY(content.contains("Persistent error"));
    QVERIFY(content.contains("This should be saved"));
    QVERIFY(content.contains("FileSystem"));
    QVERIFY(content.contains("Error"));
    
    file.close();
    
    // 清理测试文件
    QFile::remove(logFile);
}