#include "../../include/utils/FileUtils.h"
#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpression>

bool FileUtils::isComicFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    QStringList comicExtensions = getSupportedComicExtensions();
    return comicExtensions.contains(extension);
}

QStringList FileUtils::getSupportedComicExtensions()
{
    return QStringList() << "cbz" << "cbr" << "zip" << "rar" << "7z";
}

bool FileUtils::isImageFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    QStringList imageExtensions = getSupportedImageExtensions();
    return imageExtensions.contains(extension);
}

QStringList FileUtils::getSupportedImageExtensions()
{
    return QStringList() << "jpg" << "jpeg" << "png" << "gif" << "bmp" 
                        << "webp" << "tiff" << "svg" << "ico";
}

QString FileUtils::formatFileSize(qint64 size)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;
    
    if (size >= TB) {
        return QString::number(size / double(TB), 'f', 2) + " TB";
    } else if (size >= GB) {
        return QString::number(size / double(GB), 'f', 2) + " GB";
    } else if (size >= MB) {
        return QString::number(size / double(MB), 'f', 2) + " MB";
    } else if (size >= KB) {
        return QString::number(size / double(KB), 'f', 2) + " KB";
    } else {
        return QString::number(size) + " B";
    }
}

bool FileUtils::ensureDirectoryExists(const QString &dirPath)
{
    QDir dir;
    return dir.mkpath(dirPath);
}

QString FileUtils::getMimeType(const QString &filePath)
{
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);
    return mimeType.name();
}

QString FileUtils::generateSafeFileName(const QString &fileName)
{
    QString safeName = fileName;
    
    // 移除或替换不安全的字符
    QRegularExpression invalidChars("[<>:\"/\\\\|?*]");
    safeName.replace(invalidChars, "_");
    
    // 移除前后空格
    safeName = safeName.trimmed();
    
    // 如果文件名为空或只包含点，使用默认名称
    if (safeName.isEmpty() || safeName == "." || safeName == "..") {
        safeName = "untitled";
    }
    
    // 限制文件名长度（Windows下最大255个字符）
    if (safeName.length() > 255) {
        safeName = safeName.left(255);
    }
    
    return safeName;
}
