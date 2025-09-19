#include "ErrorHandler.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QUuid>
#include <QMutexLocker>
#include <QDebug>
#include <QTextStream>
#include <QSystemTrayIcon>
#include <algorithm>

Q_LOGGING_CATEGORY(COMIC_READER, "comicreader")

ErrorHandler* ErrorHandler::m_instance = nullptr;

// ErrorInfo 实现
QString ErrorInfo::toString() const
{
    return QString("[%1] %2: %3 - %4")
           .arg(timestamp.toString("yyyy-MM-dd hh:mm:ss"))
           .arg(code)
           .arg(title)
           .arg(message);
}

QString ErrorInfo::toHtml() const
{
    QString severityColor;
    switch (severity) {
    case ErrorSeverity::Info:
        severityColor = "#0099FF";
        break;
    case ErrorSeverity::Warning:
        severityColor = "#FF9900";
        break;
    case ErrorSeverity::Error:
        severityColor = "#FF3300";
        break;
    case ErrorSeverity::Critical:
        severityColor = "#CC0000";
        break;
    }
    
    QString html = QString(
        "<div style='border-left: 4px solid %1; padding: 10px; margin: 5px 0;'>"
        "<h3 style='color: %1; margin: 0;'>%2</h3>"
        "<p><strong>代码:</strong> %3</p>"
        "<p><strong>时间:</strong> %4</p>"
        "<p><strong>消息:</strong> %5</p>"
    ).arg(severityColor, title, code, timestamp.toString("yyyy-MM-dd hh:mm:ss"), message);
    
    if (!context.isEmpty()) {
        html += QString("<p><strong>上下文:</strong> %1</p>").arg(context);
    }
    
    if (!file.isEmpty()) {
        html += QString("<p><strong>位置:</strong> %1:%2 in %3</p>").arg(file).arg(line).arg(function);
    }
    
    html += "</div>";
    
    return html;
}

QJsonObject ErrorInfo::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["severity"] = static_cast<int>(severity);
    obj["category"] = static_cast<int>(category);
    obj["code"] = code;
    obj["title"] = title;
    obj["message"] = message;
    obj["context"] = context;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    obj["file"] = file;
    obj["line"] = line;
    obj["function"] = function;
    
    QJsonObject extraObj;
    for (auto it = extra.begin(); it != extra.end(); ++it) {
        extraObj[it.key()] = it.value();
    }
    obj["extra"] = extraObj;
    
    return obj;
}

void ErrorInfo::fromJson(const QJsonObject &json)
{
    id = json["id"].toString();
    severity = static_cast<ErrorSeverity>(json["severity"].toInt());
    category = static_cast<ErrorCategory>(json["category"].toInt());
    code = json["code"].toString();
    title = json["title"].toString();
    message = json["message"].toString();
    context = json["context"].toString();
    timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    file = json["file"].toString();
    line = json["line"].toInt();
    function = json["function"].toString();
    
    extra.clear();
    QJsonObject extraObj = json["extra"].toObject();
    for (auto it = extraObj.begin(); it != extraObj.end(); ++it) {
        extra[it.key()] = it.value().toString();
    }
}

// ErrorHandler 实现
ErrorHandler::ErrorHandler(QObject *parent)
    : QObject(parent)
    , m_maxErrorCount(1000)
    , m_autoCleanupEnabled(true)
    , m_autoCleanupInterval(60) // 60分钟
    , m_logToFile(true)
    , m_showDialogs(true)
    , m_showNotifications(true)
{
    // 设置日志文件路径
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_logFilePath = dataDir + "/error.log";
    
    // 设置自动清理定时器
    m_autoCleanupTimer = new QTimer(this);
    connect(m_autoCleanupTimer, &QTimer::timeout, this, &ErrorHandler::onAutoCleanupTimer);
    
    if (m_autoCleanupEnabled) {
        m_autoCleanupTimer->start(m_autoCleanupInterval * 60 * 1000);
    }
}

ErrorHandler::~ErrorHandler()
{
    // 保存错误日志
    if (m_logToFile && !m_errors.isEmpty()) {
        exportErrors(m_logFilePath.replace(".log", "_final.json"));
    }
}

ErrorHandler* ErrorHandler::instance()
{
    if (!m_instance) {
        m_instance = new ErrorHandler();
    }
    return m_instance;
}

void ErrorHandler::reportError(ErrorSeverity severity, ErrorCategory category,
                              const QString &code, const QString &title, const QString &message,
                              const QString &context, const QString &file,
                              int line, const QString &function)
{
    ErrorInfo error;
    error.id = generateErrorId();
    error.severity = severity;
    error.category = category;
    error.code = code;
    error.title = title;
    error.message = message;
    error.context = context;
    error.timestamp = QDateTime::currentDateTime();
    error.file = file;
    error.line = line;
    error.function = function;
    
    addError(error);
}

void ErrorHandler::reportInfo(const QString &code, const QString &title, const QString &message,
                             const QString &context)
{
    reportError(ErrorSeverity::Info, ErrorCategory::Unknown, code, title, message, context);
}

void ErrorHandler::reportWarning(const QString &code, const QString &title, const QString &message,
                                const QString &context)
{
    reportError(ErrorSeverity::Warning, ErrorCategory::Unknown, code, title, message, context);
}

void ErrorHandler::reportError(const QString &code, const QString &title, const QString &message,
                              const QString &context)
{
    reportError(ErrorSeverity::Error, ErrorCategory::Unknown, code, title, message, context);
}

void ErrorHandler::reportCritical(const QString &code, const QString &title, const QString &message,
                                 const QString &context)
{
    reportError(ErrorSeverity::Critical, ErrorCategory::Unknown, code, title, message, context);
}

void ErrorHandler::reportErrorWithLocation(ErrorSeverity severity, ErrorCategory category,
                                          const QString &code, const QString &title, const QString &message,
                                          const QString &context, const char *file, int line, const char *function)
{
    QString fileName = file ? QFileInfo(QString::fromUtf8(file)).fileName() : QString();
    QString funcName = function ? QString::fromUtf8(function) : QString();
    
    reportError(severity, category, code, title, message, context, fileName, line, funcName);
}

QList<ErrorInfo> ErrorHandler::getAllErrors() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    QList<ErrorInfo> errors = m_errors.values();
    
    // 按时间排序（最新的在前面）
    std::sort(errors.begin(), errors.end(), [](const ErrorInfo &a, const ErrorInfo &b) {
        return a.timestamp > b.timestamp;
    });
    
    return errors;
}

QList<ErrorInfo> ErrorHandler::getErrorsByCategory(ErrorCategory category) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    QList<ErrorInfo> result;
    for (const ErrorInfo &error : m_errors.values()) {
        if (error.category == category) {
            result.append(error);
        }
    }
    
    // 按时间排序
    std::sort(result.begin(), result.end(), [](const ErrorInfo &a, const ErrorInfo &b) {
        return a.timestamp > b.timestamp;
    });
    
    return result;
}

QList<ErrorInfo> ErrorHandler::getErrorsBySeverity(ErrorSeverity severity) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    QList<ErrorInfo> result;
    for (const ErrorInfo &error : m_errors.values()) {
        if (error.severity == severity) {
            result.append(error);
        }
    }
    
    // 按时间排序
    std::sort(result.begin(), result.end(), [](const ErrorInfo &a, const ErrorInfo &b) {
        return a.timestamp > b.timestamp;
    });
    
    return result;
}

QList<ErrorInfo> ErrorHandler::getRecentErrors(int count) const
{
    QList<ErrorInfo> allErrors = getAllErrors();
    return allErrors.mid(0, count);
}

ErrorInfo ErrorHandler::getError(const QString &id) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    return m_errors.value(id);
}

int ErrorHandler::getErrorCount() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    return m_errors.size();
}

int ErrorHandler::getErrorCount(ErrorSeverity severity) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    int count = 0;
    for (const ErrorInfo &error : m_errors.values()) {
        if (error.severity == severity) {
            count++;
        }
    }
    return count;
}

int ErrorHandler::getErrorCount(ErrorCategory category) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    int count = 0;
    for (const ErrorInfo &error : m_errors.values()) {
        if (error.category == category) {
            count++;
        }
    }
    return count;
}

QHash<ErrorCategory, int> ErrorHandler::getCategoryStatistics() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    QHash<ErrorCategory, int> stats;
    for (const ErrorInfo &error : m_errors.values()) {
        stats[error.category]++;
    }
    return stats;
}

QHash<ErrorSeverity, int> ErrorHandler::getSeverityStatistics() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    QHash<ErrorSeverity, int> stats;
    for (const ErrorInfo &error : m_errors.values()) {
        stats[error.severity]++;
    }
    return stats;
}

void ErrorHandler::setMaxErrorCount(int maxCount)
{
    QMutexLocker locker(&m_mutex);
    m_maxErrorCount = maxCount;
    locker.unlock();
    
    enforceMaxErrorCount();
}

void ErrorHandler::setAutoCleanupEnabled(bool enabled)
{
    m_autoCleanupEnabled = enabled;
    
    if (enabled && !m_autoCleanupTimer->isActive()) {
        m_autoCleanupTimer->start(m_autoCleanupInterval * 60 * 1000);
    } else if (!enabled && m_autoCleanupTimer->isActive()) {
        m_autoCleanupTimer->stop();
    }
}

void ErrorHandler::setAutoCleanupInterval(int minutes)
{
    m_autoCleanupInterval = minutes;
    
    if (m_autoCleanupEnabled) {
        m_autoCleanupTimer->start(m_autoCleanupInterval * 60 * 1000);
    }
}

void ErrorHandler::setLogToFile(bool enabled, const QString &filePath)
{
    m_logToFile = enabled;
    
    if (!filePath.isEmpty()) {
        m_logFilePath = filePath;
    }
}

void ErrorHandler::setShowDialogs(bool enabled)
{
    m_showDialogs = enabled;
}

void ErrorHandler::setShowNotifications(bool enabled)
{
    m_showNotifications = enabled;
}

void ErrorHandler::clearErrors()
{
    QMutexLocker locker(&m_mutex);
    m_errors.clear();
    locker.unlock();
    
    emit errorsCleared();
    emit errorStatisticsChanged();
}

void ErrorHandler::clearErrors(ErrorCategory category)
{
    QMutexLocker locker(&m_mutex);
    
    QStringList toRemove;
    for (auto it = m_errors.begin(); it != m_errors.end(); ++it) {
        if (it.value().category == category) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString &id : toRemove) {
        m_errors.remove(id);
    }
    
    locker.unlock();
    
    if (!toRemove.isEmpty()) {
        emit errorsCleared();
        emit errorStatisticsChanged();
    }
}

void ErrorHandler::clearErrors(ErrorSeverity severity)
{
    QMutexLocker locker(&m_mutex);
    
    QStringList toRemove;
    for (auto it = m_errors.begin(); it != m_errors.end(); ++it) {
        if (it.value().severity == severity) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString &id : toRemove) {
        m_errors.remove(id);
    }
    
    locker.unlock();
    
    if (!toRemove.isEmpty()) {
        emit errorsCleared();
        emit errorStatisticsChanged();
    }
}

void ErrorHandler::clearOldErrors(int daysToKeep)
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-daysToKeep);
    QStringList toRemove;
    
    for (auto it = m_errors.begin(); it != m_errors.end(); ++it) {
        if (it.value().timestamp < cutoffDate) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString &id : toRemove) {
        m_errors.remove(id);
    }
    
    locker.unlock();
    
    if (!toRemove.isEmpty()) {
        emit errorsCleared();
        emit errorStatisticsChanged();
        qCDebug(COMIC_READER) << "Cleaned up" << toRemove.size() << "old errors";
    }
}

bool ErrorHandler::exportErrors(const QString &filePath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_mutex));
    
    QJsonObject rootObj;
    QJsonArray errorsArray;
    
    for (const ErrorInfo &error : m_errors.values()) {
        errorsArray.append(error.toJson());
    }
    
    rootObj["errors"] = errorsArray;
    rootObj["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["version"] = "1.0";
    
    QJsonDocument doc(rootObj);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(COMIC_READER) << "Failed to open error export file:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool ErrorHandler::importErrors(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(COMIC_READER) << "Failed to open error import file:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qCWarning(COMIC_READER) << "Invalid JSON in error import file:" << filePath;
        return false;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray errorsArray = rootObj["errors"].toArray();
    
    QMutexLocker locker(&m_mutex);
    
    int importedCount = 0;
    for (const QJsonValue &value : errorsArray) {
        ErrorInfo error;
        error.fromJson(value.toObject());
        
        if (!error.id.isEmpty() && !m_errors.contains(error.id)) {
            m_errors[error.id] = error;
            importedCount++;
        }
    }
    
    locker.unlock();
    
    if (importedCount > 0) {
        emit errorStatisticsChanged();
        qCDebug(COMIC_READER) << "Imported" << importedCount << "errors";
    }
    
    return importedCount > 0;
}

void ErrorHandler::showErrorDialog(const ErrorInfo &error)
{
    if (!m_showDialogs) {
        return;
    }
    
    QMessageBox::Icon icon;
    switch (error.severity) {
    case ErrorSeverity::Info:
        icon = QMessageBox::Information;
        break;
    case ErrorSeverity::Warning:
        icon = QMessageBox::Warning;
        break;
    case ErrorSeverity::Error:
        icon = QMessageBox::Critical;
        break;
    case ErrorSeverity::Critical:
        icon = QMessageBox::Critical;
        break;
    }
    
    QString detailedText;
    if (!error.context.isEmpty()) {
        detailedText += QString("上下文: %1\n").arg(error.context);
    }
    if (!error.file.isEmpty()) {
        detailedText += QString("位置: %1:%2 in %3\n").arg(error.file).arg(error.line).arg(error.function);
    }
    detailedText += QString("时间: %1\n").arg(error.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
    detailedText += QString("代码: %1").arg(error.code);
    
    QMessageBox msgBox;
    msgBox.setIcon(icon);
    msgBox.setWindowTitle("错误 - ComicReader");
    msgBox.setText(error.title);
    msgBox.setInformativeText(error.message);
    msgBox.setDetailedText(detailedText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void ErrorHandler::showErrorSummary()
{
    QHash<ErrorSeverity, int> severityStats = getSeverityStatistics();
    QHash<ErrorCategory, int> categoryStats = getCategoryStatistics();
    
    QString summary = "错误统计摘要:\n\n";
    
    summary += "按严重程度:\n";
    summary += QString("  信息: %1\n").arg(severityStats.value(ErrorSeverity::Info, 0));
    summary += QString("  警告: %1\n").arg(severityStats.value(ErrorSeverity::Warning, 0));
    summary += QString("  错误: %1\n").arg(severityStats.value(ErrorSeverity::Error, 0));
    summary += QString("  严重: %1\n").arg(severityStats.value(ErrorSeverity::Critical, 0));
    
    summary += "\n按类别:\n";
    for (auto it = categoryStats.begin(); it != categoryStats.end(); ++it) {
        summary += QString("  %1: %2\n").arg(categoryToString(it.key())).arg(it.value());
    }
    
    QMessageBox::information(nullptr, "错误统计 - ComicReader", summary);
}

void ErrorHandler::showErrorLog()
{
    // 这里应该打开错误日志窗口
    // TODO: 实现错误日志窗口
    qCDebug(COMIC_READER) << "Error log window requested";
}

void ErrorHandler::onAutoCleanupTimer()
{
    clearOldErrors(7); // 保留7天内的错误
    enforceMaxErrorCount();
}

QString ErrorHandler::generateErrorId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ErrorHandler::addError(const ErrorInfo &error)
{
    QMutexLocker locker(&m_mutex);
    m_errors[error.id] = error;
    locker.unlock();
    
    // 记录到日志
    if (m_logToFile) {
        logToFile(error);
    }
    
    // 显示通知
    if (m_showNotifications) {
        showNotification(error);
    }
    
    // 对于严重错误，显示对话框
    if (error.severity == ErrorSeverity::Critical && m_showDialogs) {
        showErrorDialog(error);
    }
    
    // 强制执行最大错误数量限制
    enforceMaxErrorCount();
    
    // 发出信号
    emit errorReported(error);
    emit errorStatisticsChanged();
    
    // Qt日志系统输出
    switch (error.severity) {
    case ErrorSeverity::Info:
        qCInfo(COMIC_READER) << error.toString();
        break;
    case ErrorSeverity::Warning:
        qCWarning(COMIC_READER) << error.toString();
        break;
    case ErrorSeverity::Error:
        qCCritical(COMIC_READER) << error.toString();
        break;
    case ErrorSeverity::Critical:
        qCCritical(COMIC_READER) << "CRITICAL:" << error.toString();
        break;
    }
}

void ErrorHandler::logToFile(const ErrorInfo &error)
{
    QFile file(m_logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << QString("[%1] [%2] [%3] %4: %5 - %6")
                  .arg(error.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                  .arg(severityToString(error.severity))
                  .arg(categoryToString(error.category))
                  .arg(error.code)
                  .arg(error.title)
                  .arg(error.message);
        
        if (!error.context.isEmpty()) {
            stream << " | Context: " << error.context;
        }
        
        if (!error.file.isEmpty()) {
            stream << QString(" | Location: %1:%2 in %3").arg(error.file).arg(error.line).arg(error.function);
        }
        
        stream << "\n";
    }
}

void ErrorHandler::showNotification(const ErrorInfo &error)
{
    // 只为警告和错误显示通知
    if (error.severity == ErrorSeverity::Info) {
        return;
    }
    
    // 检查系统托盘是否可用
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    
    QSystemTrayIcon::MessageIcon icon;
    switch (error.severity) {
    case ErrorSeverity::Warning:
        icon = QSystemTrayIcon::Warning;
        break;
    case ErrorSeverity::Error:
    case ErrorSeverity::Critical:
        icon = QSystemTrayIcon::Critical;
        break;
    default:
        icon = QSystemTrayIcon::Information;
        break;
    }
    
    // 这里需要访问主窗口的系统托盘图标
    // TODO: 实现通知显示
}

void ErrorHandler::enforceMaxErrorCount()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_errors.size() <= m_maxErrorCount) {
        return;
    }
    
    // 获取所有错误并按时间排序
    QList<ErrorInfo> allErrors = m_errors.values();
    std::sort(allErrors.begin(), allErrors.end(), [](const ErrorInfo &a, const ErrorInfo &b) {
        return a.timestamp < b.timestamp; // 最旧的在前面
    });
    
    // 删除最旧的错误
    int toRemove = m_errors.size() - m_maxErrorCount;
    for (int i = 0; i < toRemove; ++i) {
        m_errors.remove(allErrors[i].id);
    }
    
    locker.unlock();
    
    qCDebug(COMIC_READER) << "Removed" << toRemove << "old errors to enforce max count limit";
}

QString ErrorHandler::severityToString(ErrorSeverity severity)
{
    switch (severity) {
    case ErrorSeverity::Info:
        return "信息";
    case ErrorSeverity::Warning:
        return "警告";
    case ErrorSeverity::Error:
        return "错误";
    case ErrorSeverity::Critical:
        return "严重";
    default:
        return "未知";
    }
}

QString ErrorHandler::categoryToString(ErrorCategory category)
{
    switch (category) {
    case ErrorCategory::FileIO:
        return "文件IO";
    case ErrorCategory::Network:
        return "网络";
    case ErrorCategory::Parsing:
        return "解析";
    case ErrorCategory::Cache:
        return "缓存";
    case ErrorCategory::Configuration:
        return "配置";
    case ErrorCategory::UI:
        return "界面";
    case ErrorCategory::Memory:
        return "内存";
    case ErrorCategory::Permission:
        return "权限";
    default:
        return "未知";
    }
}

ErrorSeverity ErrorHandler::stringToSeverity(const QString &str)
{
    if (str == "信息") return ErrorSeverity::Info;
    if (str == "警告") return ErrorSeverity::Warning;
    if (str == "错误") return ErrorSeverity::Error;
    if (str == "严重") return ErrorSeverity::Critical;
    return ErrorSeverity::Error;
}

ErrorCategory ErrorHandler::stringToCategory(const QString &str)
{
    if (str == "文件IO") return ErrorCategory::FileIO;
    if (str == "网络") return ErrorCategory::Network;
    if (str == "解析") return ErrorCategory::Parsing;
    if (str == "缓存") return ErrorCategory::Cache;
    if (str == "配置") return ErrorCategory::Configuration;
    if (str == "界面") return ErrorCategory::UI;
    if (str == "内存") return ErrorCategory::Memory;
    if (str == "权限") return ErrorCategory::Permission;
    return ErrorCategory::Unknown;
}