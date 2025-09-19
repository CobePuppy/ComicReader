#pragma once

#include <QObject>
#include <QtTest>

class ErrorHandler;

class TestErrorHandler : public QObject
{
    Q_OBJECT

public:
    TestErrorHandler() = default;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    
    void testReportError();
    void testErrorCategorization();
    void testErrorFiltering();
    void testErrorStatistics();
    void testErrorCleanup();
    void testErrorNotification();
    void testErrorPersistence();

private:
    ErrorHandler *errorHandler = nullptr;
};