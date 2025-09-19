#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(COMIC_READER)

// 错误严重程度
enum class ErrorSeverity {
    Info,       // 信息
    Warning,    // 警告
    Error,      // 错误
    Critical    // 严重错误
};

// 错误类别
enum class ErrorCategory {
    FileIO,         // 文件输入输出
    Network,        // 网络相关
    Parsing,        // 解析相关
    Cache,          // 缓存相关
    Configuration,  // 配置相关
    UI,             // 用户界面
    Memory,         // 内存相关
    Permission,     // 权限相关
    Unknown         // 未知错误
};

// 错误信息结构
struct ErrorInfo {
    QString id;                 // 错误唯一ID
    ErrorSeverity severity;     // 严重程度
    ErrorCategory category;     // 错误类别
    QString code;               // 错误代码
    QString title;              // 错误标题
    QString message;            // 错误详细信息
    QString context;            // 错误上下文
    QDateTime timestamp;        // 发生时间
    QString file;               // 源文件名
    int line;                   // 源文件行号
    QString function;           // 函数名
    QHash<QString, QString> extra; // 额外信息
    
    ErrorInfo() : severity(ErrorSeverity::Error), category(ErrorCategory::Unknown), line(0) {}
    
    QString toString() const;
    QString toHtml() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
};

// 错误处理器
class ErrorHandler : public QObject
{
    Q_OBJECT

public:
    static ErrorHandler* instance();
    
    // 错误报告
    void reportError(ErrorSeverity severity, ErrorCategory category,
                     const QString &code, const QString &title, const QString &message,
                     const QString &context = QString(), const QString &file = QString(),
                     int line = 0, const QString &function = QString());
    
    void reportInfo(const QString &code, const QString &title, const QString &message,
                    const QString &context = QString());
    void reportWarning(const QString &code, const QString &title, const QString &message,
                       const QString &context = QString());
    void reportError(const QString &code, const QString &title, const QString &message,
                     const QString &context = QString());
    void reportCritical(const QString &code, const QString &title, const QString &message,
                        const QString &context = QString());
    
    // 便捷宏支持
    void reportErrorWithLocation(ErrorSeverity severity, ErrorCategory category,
                                const QString &code, const QString &title, const QString &message,
                                const QString &context, const char *file, int line, const char *function);
    
    // 错误查询
    QList<ErrorInfo> getAllErrors() const;
    QList<ErrorInfo> getErrorsByCategory(ErrorCategory category) const;
    QList<ErrorInfo> getErrorsBySeverity(ErrorSeverity severity) const;
    QList<ErrorInfo> getRecentErrors(int count = 10) const;
    ErrorInfo getError(const QString &id) const;
    
    // 错误统计
    int getErrorCount() const;
    int getErrorCount(ErrorSeverity severity) const;
    int getErrorCount(ErrorCategory category) const;
    QHash<ErrorCategory, int> getCategoryStatistics() const;
    QHash<ErrorSeverity, int> getSeverityStatistics() const;
    
    // 配置
    void setMaxErrorCount(int maxCount);
    void setAutoCleanupEnabled(bool enabled);
    void setAutoCleanupInterval(int minutes);
    void setLogToFile(bool enabled, const QString &filePath = QString());
    void setShowDialogs(bool enabled);
    void setShowNotifications(bool enabled);
    
    // 清理和维护
    void clearErrors();
    void clearErrors(ErrorCategory category);
    void clearErrors(ErrorSeverity severity);
    void clearOldErrors(int daysToKeep = 7);
    
    // 导入导出
    bool exportErrors(const QString &filePath) const;
    bool importErrors(const QString &filePath);
    
    // 用户反馈
    void showErrorDialog(const ErrorInfo &error);
    void showErrorSummary();
    void showErrorLog();

signals:
    void errorReported(const ErrorInfo &error);
    void errorsCleared();
    void errorStatisticsChanged();

public slots:
    void onAutoCleanupTimer();

private:
    explicit ErrorHandler(QObject *parent = nullptr);
    ~ErrorHandler();
    
    QString generateErrorId() const;
    void addError(const ErrorInfo &error);
    void logToFile(const ErrorInfo &error);
    void showNotification(const ErrorInfo &error);
    void enforceMaxErrorCount();
    
    static QString severityToString(ErrorSeverity severity);
    static QString categoryToString(ErrorCategory category);
    static ErrorSeverity stringToSeverity(const QString &str);
    static ErrorCategory stringToCategory(const QString &str);
    
    // 数据存储
    QHash<QString, ErrorInfo> m_errors;
    QMutex m_mutex;
    
    // 配置
    int m_maxErrorCount;
    bool m_autoCleanupEnabled;
    int m_autoCleanupInterval;
    bool m_logToFile;
    QString m_logFilePath;
    bool m_showDialogs;
    bool m_showNotifications;
    
    // 内部组件
    QTimer *m_autoCleanupTimer;
    
    static ErrorHandler *m_instance;
};

// 便捷宏定义
#define REPORT_ERROR(category, code, title, message) \
    ErrorHandler::instance()->reportErrorWithLocation( \
        ErrorSeverity::Error, category, code, title, message, QString(), __FILE__, __LINE__, __FUNCTION__)

#define REPORT_WARNING(category, code, title, message) \
    ErrorHandler::instance()->reportErrorWithLocation( \
        ErrorSeverity::Warning, category, code, title, message, QString(), __FILE__, __LINE__, __FUNCTION__)

#define REPORT_INFO(category, code, title, message) \
    ErrorHandler::instance()->reportErrorWithLocation( \
        ErrorSeverity::Info, category, code, title, message, QString(), __FILE__, __LINE__, __FUNCTION__)

#define REPORT_CRITICAL(category, code, title, message) \
    ErrorHandler::instance()->reportErrorWithLocation( \
        ErrorSeverity::Critical, category, code, title, message, QString(), __FILE__, __LINE__, __FUNCTION__)

#define REPORT_ERROR_WITH_CONTEXT(category, code, title, message, context) \
    ErrorHandler::instance()->reportErrorWithLocation( \
        ErrorSeverity::Error, category, code, title, message, context, __FILE__, __LINE__, __FUNCTION__)

// 异常类
class ComicReaderException : public std::exception
{
public:
    ComicReaderException(ErrorCategory category, const QString &code, 
                        const QString &title, const QString &message,
                        const QString &context = QString())
        : m_category(category), m_code(code), m_title(title), m_message(message), m_context(context)
    {
        m_fullMessage = QString("%1: %2").arg(title, message).toUtf8();
    }
    
    const char* what() const noexcept override {
        return m_fullMessage.constData();
    }
    
    ErrorCategory category() const { return m_category; }
    QString code() const { return m_code; }
    QString title() const { return m_title; }
    QString message() const { return m_message; }
    QString context() const { return m_context; }

private:
    ErrorCategory m_category;
    QString m_code;
    QString m_title;
    QString m_message;
    QString m_context;
    QByteArray m_fullMessage;
};

// 特定异常类
class FileIOException : public ComicReaderException
{
public:
    FileIOException(const QString &code, const QString &title, const QString &message, const QString &filePath = QString())
        : ComicReaderException(ErrorCategory::FileIO, code, title, message, filePath), m_filePath(filePath) {}
    
    QString filePath() const { return m_filePath; }

private:
    QString m_filePath;
};

class NetworkException : public ComicReaderException
{
public:
    NetworkException(const QString &code, const QString &title, const QString &message, const QString &url = QString())
        : ComicReaderException(ErrorCategory::Network, code, title, message, url), m_url(url) {}
    
    QString url() const { return m_url; }

private:
    QString m_url;
};

class ParsingException : public ComicReaderException
{
public:
    ParsingException(const QString &code, const QString &title, const QString &message, const QString &filePath = QString())
        : ComicReaderException(ErrorCategory::Parsing, code, title, message, filePath), m_filePath(filePath) {}
    
    QString filePath() const { return m_filePath; }

private:
    QString m_filePath;
};

#endif // ERRORHANDLER_H