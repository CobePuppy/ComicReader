#ifndef COMICPARSER_H
#define COMICPARSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QByteArray>
#include <QProcess>

class QZipReader;

/**
 * @brief 漫画文件解析器
 * 支持解析 CBZ, CBR, ZIP, RAR 格式的漫画文件
 */
class ComicParser : public QObject
{
    Q_OBJECT

public:
    enum ComicFormat {
        Unknown,
        CBZ,    // Comic Book Zip
        CBR,    // Comic Book RAR
        ZIP,    // Standard ZIP
        RAR     // Standard RAR
    };

    struct ComicInfo {
        QString filePath;           // 文件路径
        QString title;              // 漫画标题
        ComicFormat format;         // 文件格式
        int pageCount;              // 页面数量
        QStringList pageList;       // 页面文件列表（内部路径）
        QString coverPath;          // 封面图片路径
        qint64 fileSize;           // 文件大小
    };

    explicit ComicParser(QObject *parent = nullptr);
    ~ComicParser();

    // 解析漫画文件
    bool parseComic(const QString &filePath);
    
    // 获取漫画信息
    const ComicInfo& getComicInfo() const { return m_comicInfo; }
    
    // 获取指定页面的图片数据
    QByteArray getPageData(int pageIndex) const;
    
    // 获取指定页面的图片
    QPixmap getPagePixmap(int pageIndex) const;
    
    // 检查文件格式
    static ComicFormat detectFormat(const QString &filePath);
    
    // 格式转换为字符串
    static QString formatToString(ComicFormat format);
    
    // 检查是否为支持的图片格式
    static bool isSupportedImageFormat(const QString &fileName);

signals:
    void parseProgress(int current, int total);
    void parseCompleted(bool success);
    void error(const QString &errorMessage);

private slots:
    void onExtractionFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    // 解析不同格式的文件
    bool parseZipFile(const QString &filePath);
    bool parseRarFile(const QString &filePath);
    
    // 提取页面列表
    void extractPageList(const QStringList &fileList);
    
    // 排序页面（自然排序）
    void sortPages();
    
    // 清理临时数据
    void cleanup();

    ComicInfo m_comicInfo;
    QZipReader *m_zipReader;
    QProcess *m_rarProcess;
    QString m_tempDir;
    
    static QStringList s_supportedImageExtensions;
};

#endif // COMICPARSER_H
