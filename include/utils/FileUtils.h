#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QStringList>
#include <QFileInfo>

class FileUtils
{
public:
    // 检查文件是否为支持的漫画格式
    static bool isComicFile(const QString &filePath);
    
    // 获取支持的漫画文件扩展名
    static QStringList getSupportedComicExtensions();
    
    // 检查文件是否为图片格式
    static bool isImageFile(const QString &filePath);
    
    // 获取支持的图片文件扩展名
    static QStringList getSupportedImageExtensions();
    
    // 格式化文件大小
    static QString formatFileSize(qint64 size);
    
    // 创建目录（如果不存在）
    static bool ensureDirectoryExists(const QString &dirPath);
    
    // 获取文件的MIME类型
    static QString getMimeType(const QString &filePath);
    
    // 生成安全的文件名
    static QString generateSafeFileName(const QString &fileName);

private:
    FileUtils() = delete; // 静态工具类，禁止实例化
};

#endif // FILEUTILS_H
