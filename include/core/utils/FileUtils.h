#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QMimeType>
#include <QUrl>

/**
 * @brief 文件工具类
 * 提供文件操作、路径处理、格式检测等基础功能
 */
class FileUtils
{
public:
    // 文件操作
    static bool exists(const QString &filePath);
    static bool isFile(const QString &filePath);
    static bool isDirectory(const QString &dirPath);
    static bool isReadable(const QString &filePath);
    static bool isWritable(const QString &filePath);
    
    static qint64 fileSize(const QString &filePath);
    static QString fileName(const QString &filePath);
    static QString baseName(const QString &filePath);
    static QString suffix(const QString &filePath);
    static QString dirPath(const QString &filePath);
    static QString absolutePath(const QString &filePath);
    static QString canonicalPath(const QString &filePath);
    
    // 目录操作
    static bool createDir(const QString &dirPath);
    static bool removeDir(const QString &dirPath);
    static bool copyDir(const QString &sourcePath, const QString &targetPath);
    static QStringList listFiles(const QString &dirPath, 
                                 const QStringList &filters = QStringList(),
                                 bool recursive = false);
    static QStringList listDirs(const QString &dirPath, bool recursive = false);
    
    // 文件格式检测
    static bool isComicFile(const QString &filePath);
    static bool isImageFile(const QString &filePath);
    static bool isArchiveFile(const QString &filePath);
    static QString getMimeType(const QString &filePath);
    
    // 支持的格式
    static QStringList getSupportedComicFormats();
    static QStringList getSupportedImageFormats();
    static QStringList getSupportedArchiveFormats();
    
    // 路径处理
    static QString toNativePath(const QString &path);
    static QString fromNativePath(const QString &path);
    static QString combinePath(const QString &basePath, const QString &relativePath);
    static QString relativePath(const QString &basePath, const QString &targetPath);
    
    // URL处理
    static QString urlToLocalFile(const QUrl &url);
    static QUrl localFileToUrl(const QString &filePath);
    static bool isValidUrl(const QString &urlString);
    
    // 文件内容操作
    static QByteArray readFile(const QString &filePath);
    static bool writeFile(const QString &filePath, const QByteArray &data);
    static QString readTextFile(const QString &filePath, const QString &encoding = "UTF-8");
    static bool writeTextFile(const QString &filePath, const QString &text, 
                             const QString &encoding = "UTF-8");
    
    // 临时文件和目录
    static QString createTempFile(const QString &templateName = QString());
    static QString createTempDir(const QString &templateName = QString());
    static void cleanupTempFiles();
    
    // 文件比较
    static bool filesEqual(const QString &filePath1, const QString &filePath2);
    static QString calculateFileHash(const QString &filePath, const QString &algorithm = "MD5");
    
    // 格式化工具
    static QString formatFileSize(qint64 bytes);
    static QString formatFilePath(const QString &filePath, int maxLength = 50);
    
    // 安全性检查
    static bool isSafePath(const QString &path);
    static QString sanitizeFileName(const QString &fileName);
    
private:
    FileUtils() = delete; // 静态工具类，禁止实例化
    
    // 内部辅助方法
    static void copyDirRecursively(const QString &sourcePath, const QString &targetPath);
    static bool isFileTypeSupported(const QString &filePath, const QStringList &supportedFormats);
    
    // 支持的格式列表
    static const QStringList COMIC_FORMATS;
    static const QStringList IMAGE_FORMATS;
    static const QStringList ARCHIVE_FORMATS;
    
    // 临时文件管理
    static QStringList s_tempFiles;
    static QStringList s_tempDirs;
};

#endif // FILEUTILS_H