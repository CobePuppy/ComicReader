#include "core/utils/FileUtils.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTextStream>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QRegularExpression>
#include <QDebug>

// 静态成员定义
const QStringList FileUtils::COMIC_FORMATS = {
    "cbz", "cbr", "cb7", "cbt", "cba",
    "zip", "rar", "7z", "tar", "pdf"
};

const QStringList FileUtils::IMAGE_FORMATS = {
    "jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif",
    "webp", "svg", "ico", "pbm", "pgm", "ppm", "xbm", "xpm"
};

const QStringList FileUtils::ARCHIVE_FORMATS = {
    "zip", "rar", "7z", "tar", "gz", "bz2", "xz", "lzma"
};

QStringList FileUtils::s_tempFiles;
QStringList FileUtils::s_tempDirs;

// 文件操作
bool FileUtils::exists(const QString &filePath)
{
    return QFileInfo::exists(filePath);
}

bool FileUtils::isFile(const QString &filePath)
{
    return QFileInfo(filePath).isFile();
}

bool FileUtils::isDirectory(const QString &dirPath)
{
    return QFileInfo(dirPath).isDir();
}

bool FileUtils::isReadable(const QString &filePath)
{
    return QFileInfo(filePath).isReadable();
}

bool FileUtils::isWritable(const QString &filePath)
{
    return QFileInfo(filePath).isWritable();
}

qint64 FileUtils::fileSize(const QString &filePath)
{
    return QFileInfo(filePath).size();
}

QString FileUtils::fileName(const QString &filePath)
{
    return QFileInfo(filePath).fileName();
}

QString FileUtils::baseName(const QString &filePath)
{
    return QFileInfo(filePath).baseName();
}

QString FileUtils::suffix(const QString &filePath)
{
    return QFileInfo(filePath).suffix().toLower();
}

QString FileUtils::dirPath(const QString &filePath)
{
    return QFileInfo(filePath).absolutePath();
}

QString FileUtils::absolutePath(const QString &filePath)
{
    return QFileInfo(filePath).absoluteFilePath();
}

QString FileUtils::canonicalPath(const QString &filePath)
{
    return QFileInfo(filePath).canonicalFilePath();
}

// 目录操作
bool FileUtils::createDir(const QString &dirPath)
{
    return QDir().mkpath(dirPath);
}

bool FileUtils::removeDir(const QString &dirPath)
{
    QDir dir(dirPath);
    return dir.removeRecursively();
}

bool FileUtils::copyDir(const QString &sourcePath, const QString &targetPath)
{
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        return false;
    }
    
    QDir targetDir(targetPath);
    if (!targetDir.exists()) {
        if (!createDir(targetPath)) {
            return false;
        }
    }
    
    copyDirRecursively(sourcePath, targetPath);
    return true;
}

QStringList FileUtils::listFiles(const QString &dirPath, 
                                 const QStringList &filters, 
                                 bool recursive)
{
    QStringList result;
    QDir dir(dirPath);
    
    if (!dir.exists()) {
        return result;
    }
    
    QDir::Filters dirFilters = QDir::Files | QDir::NoDotAndDotDot;
    
    // 设置文件过滤器
    if (!filters.isEmpty()) {
        dir.setNameFilters(filters);
    }
    
    // 获取当前目录的文件
    QFileInfoList fileList = dir.entryInfoList(dirFilters);
    for (const QFileInfo &fileInfo : fileList) {
        result.append(fileInfo.absoluteFilePath());
    }
    
    // 递归获取子目录文件
    if (recursive) {
        QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &dirInfo : dirList) {
            result.append(listFiles(dirInfo.absoluteFilePath(), filters, true));
        }
    }
    
    return result;
}

QStringList FileUtils::listDirs(const QString &dirPath, bool recursive)
{
    QStringList result;
    QDir dir(dirPath);
    
    if (!dir.exists()) {
        return result;
    }
    
    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &dirInfo : dirList) {
        result.append(dirInfo.absoluteFilePath());
        
        if (recursive) {
            result.append(listDirs(dirInfo.absoluteFilePath(), true));
        }
    }
    
    return result;
}

// 文件格式检测
bool FileUtils::isComicFile(const QString &filePath)
{
    return isFileTypeSupported(filePath, COMIC_FORMATS);
}

bool FileUtils::isImageFile(const QString &filePath)
{
    return isFileTypeSupported(filePath, IMAGE_FORMATS);
}

bool FileUtils::isArchiveFile(const QString &filePath)
{
    return isFileTypeSupported(filePath, ARCHIVE_FORMATS);
}

QString FileUtils::getMimeType(const QString &filePath)
{
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(filePath);
    return mimeType.name();
}

// 支持的格式
QStringList FileUtils::getSupportedComicFormats()
{
    return COMIC_FORMATS;
}

QStringList FileUtils::getSupportedImageFormats()
{
    return IMAGE_FORMATS;
}

QStringList FileUtils::getSupportedArchiveFormats()
{
    return ARCHIVE_FORMATS;
}

// 路径处理
QString FileUtils::toNativePath(const QString &path)
{
    return QDir::toNativeSeparators(path);
}

QString FileUtils::fromNativePath(const QString &path)
{
    return QDir::fromNativeSeparators(path);
}

QString FileUtils::combinePath(const QString &basePath, const QString &relativePath)
{
    return QDir(basePath).absoluteFilePath(relativePath);
}

QString FileUtils::relativePath(const QString &basePath, const QString &targetPath)
{
    QDir baseDir(basePath);
    return baseDir.relativeFilePath(targetPath);
}

// URL处理
QString FileUtils::urlToLocalFile(const QUrl &url)
{
    return url.toLocalFile();
}

QUrl FileUtils::localFileToUrl(const QString &filePath)
{
    return QUrl::fromLocalFile(filePath);
}

bool FileUtils::isValidUrl(const QString &urlString)
{
    QUrl url(urlString);
    return url.isValid() && !url.scheme().isEmpty();
}

// 文件内容操作
QByteArray FileUtils::readFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath;
        return QByteArray();
    }
    
    return file.readAll();
}

bool FileUtils::writeFile(const QString &filePath, const QByteArray &data)
{
    // 确保目录存在
    QString dir = dirPath(filePath);
    if (!createDir(dir)) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入文件:" << filePath;
        return false;
    }
    
    return file.write(data) == data.size();
}

QString FileUtils::readTextFile(const QString &filePath, const QString &encoding)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文本文件:" << filePath;
        return QString();
    }
    
    QTextStream in(&file);
    // Qt6中简化编码处理，默认使用UTF-8
    if (encoding.toUpper() != "UTF-8") {
        qWarning() << "警告：Qt6中建议使用UTF-8编码，当前指定编码:" << encoding;
    }
    return in.readAll();
}

bool FileUtils::writeTextFile(const QString &filePath, const QString &text, 
                             const QString &encoding)
{
    // 确保目录存在
    QString dir = dirPath(filePath);
    if (!createDir(dir)) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法写入文本文件:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    // Qt6中简化编码处理，默认使用UTF-8
    if (encoding.toUpper() != "UTF-8") {
        qWarning() << "警告：Qt6中建议使用UTF-8编码，当前指定编码:" << encoding;
    }
    out << text;
    
    return true;
}

// 临时文件和目录
QString FileUtils::createTempFile(const QString &templateName)
{
    QString tempTemplate = templateName.isEmpty() ? 
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReader_XXXXXX" :
        templateName;
    
    QTemporaryFile tempFile(tempTemplate);
    tempFile.setAutoRemove(false);
    
    if (tempFile.open()) {
        QString filePath = tempFile.fileName();
        s_tempFiles.append(filePath);
        return filePath;
    }
    
    return QString();
}

QString FileUtils::createTempDir(const QString &templateName)
{
    QString tempTemplate = templateName.isEmpty() ? 
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReader_XXXXXX" :
        templateName;
    
    QTemporaryDir tempDir(tempTemplate);
    tempDir.setAutoRemove(false);
    
    if (tempDir.isValid()) {
        QString dirPath = tempDir.path();
        s_tempDirs.append(dirPath);
        return dirPath;
    }
    
    return QString();
}

void FileUtils::cleanupTempFiles()
{
    // 清理临时文件
    for (const QString &filePath : s_tempFiles) {
        QFile::remove(filePath);
    }
    s_tempFiles.clear();
    
    // 清理临时目录
    for (const QString &dirPath : s_tempDirs) {
        removeDir(dirPath);
    }
    s_tempDirs.clear();
}

// 文件比较
bool FileUtils::filesEqual(const QString &filePath1, const QString &filePath2)
{
    if (filePath1 == filePath2) {
        return true;
    }
    
    QFile file1(filePath1);
    QFile file2(filePath2);
    
    if (!file1.open(QIODevice::ReadOnly) || !file2.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    if (file1.size() != file2.size()) {
        return false;
    }
    
    const int bufferSize = 4096;
    char buffer1[bufferSize];
    char buffer2[bufferSize];
    
    while (!file1.atEnd() && !file2.atEnd()) {
        qint64 read1 = file1.read(buffer1, bufferSize);
        qint64 read2 = file2.read(buffer2, bufferSize);
        
        if (read1 != read2 || memcmp(buffer1, buffer2, static_cast<size_t>(read1)) != 0) {
            return false;
        }
    }
    
    return file1.atEnd() && file2.atEnd();
}

QString FileUtils::calculateFileHash(const QString &filePath, const QString &algorithm)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash::Algorithm hashAlgorithm = QCryptographicHash::Md5;
    if (algorithm.toUpper() == "SHA1") {
        hashAlgorithm = QCryptographicHash::Sha1;
    } else if (algorithm.toUpper() == "SHA256") {
        hashAlgorithm = QCryptographicHash::Sha256;
    }
    
    QCryptographicHash hash(hashAlgorithm);
    hash.addData(&file);
    
    return hash.result().toHex();
}

// 格式化工具
QString FileUtils::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;
    
    if (bytes >= TB) {
        return QString::number(static_cast<double>(bytes) / TB, 'f', 2) + " TB";
    } else if (bytes >= GB) {
        return QString::number(static_cast<double>(bytes) / GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(static_cast<double>(bytes) / MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(static_cast<double>(bytes) / KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}

QString FileUtils::formatFilePath(const QString &filePath, int maxLength)
{
    if (filePath.length() <= maxLength) {
        return filePath;
    }
    
    QString fileName = FileUtils::fileName(filePath);
    QString dirPath = FileUtils::dirPath(filePath);
    
    int availableLength = maxLength - fileName.length() - 3; // 3 for "..."
    
    if (availableLength > 0 && dirPath.length() > availableLength) {
        dirPath = "..." + dirPath.right(availableLength);
    } else if (availableLength <= 0) {
        // 文件名太长，截断文件名
        return fileName.left(maxLength - 3) + "...";
    }
    
    return combinePath(dirPath, fileName);
}

// 安全性检查
bool FileUtils::isSafePath(const QString &path)
{
    // 检查路径是否包含危险字符或模式
    QStringList dangerousPatterns = {
        "..", "//", "\\\\", "<", ">", ":", "\"", "|", "?", "*"
    };
    
    for (const QString &pattern : dangerousPatterns) {
        if (path.contains(pattern)) {
            return false;
        }
    }
    
    return true;
}

QString FileUtils::sanitizeFileName(const QString &fileName)
{
    QString sanitized = fileName;
    
    // 替换非法字符
    QRegularExpression illegalChars("[<>:\"/\\\\|?*]");
    sanitized.replace(illegalChars, "_");
    
    // 移除前后空格
    sanitized = sanitized.trimmed();
    
    // 确保不为空且不是保留名称
    if (sanitized.isEmpty() || sanitized == "." || sanitized == "..") {
        sanitized = "untitled";
    }
    
    return sanitized;
}

// 内部辅助方法
void FileUtils::copyDirRecursively(const QString &sourcePath, const QString &targetPath)
{
    QDir sourceDir(sourcePath);
    QDir targetDir(targetPath);
    
    // 复制文件
    QFileInfoList files = sourceDir.entryInfoList(QDir::Files);
    for (const QFileInfo &fileInfo : files) {
        QString targetFile = targetDir.absoluteFilePath(fileInfo.fileName());
        QFile::copy(fileInfo.absoluteFilePath(), targetFile);
    }
    
    // 递归复制子目录
    QFileInfoList dirs = sourceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &dirInfo : dirs) {
        QString targetSubDir = targetDir.absoluteFilePath(dirInfo.fileName());
        createDir(targetSubDir);
        copyDirRecursively(dirInfo.absoluteFilePath(), targetSubDir);
    }
}

bool FileUtils::isFileTypeSupported(const QString &filePath, const QStringList &supportedFormats)
{
    QString fileSuffix = suffix(filePath);
    return supportedFormats.contains(fileSuffix, Qt::CaseInsensitive);
}