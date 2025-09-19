#ifndef COMICPARSER_H
#define COMICPARSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QByteArray>
#include <QIODevice>
#include <QMutex>
#include <QMap>
#include <QProcess>

// 前向声明
class QZipReader;

/**
 * @brief 漫画页面信息结构
 */
struct ComicPage
{
    QString fileName;       // 文件名
    int pageNumber;         // 页面编号
    QByteArray data;        // 图片数据
    QSize size;            // 图片尺寸
    QString format;        // 图片格式
    qint64 fileSize;       // 文件大小
    
    ComicPage() : pageNumber(-1), fileSize(0) {}
    
    bool isValid() const { 
        return !fileName.isEmpty() && pageNumber >= 0 && !data.isEmpty(); 
    }
};

/**
 * @brief 漫画信息结构
 */
struct ComicInfo
{
    QString filePath;           // 文件路径
    QString fileName;           // 文件名
    QString title;              // 标题
    QString series;             // 系列
    QString author;             // 作者
    QString publisher;          // 出版商
    QString summary;            // 摘要
    QStringList genres;         // 类型
    int pageCount;              // 页面数量
    qint64 fileSize;           // 文件大小
    QString format;            // 文件格式
    QPixmap coverImage;        // 封面图
    
    ComicInfo() : pageCount(0), fileSize(0) {}
    
    bool isValid() const { 
        return !filePath.isEmpty() && pageCount > 0; 
    }
};

/**
 * @brief 漫画解析器类
 * 支持CBZ、CBR、ZIP、RAR格式的漫画文件解析
 */
class ComicParser : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 支持的漫画格式
     */
    enum ComicFormat {
        Unknown = 0,
        CBZ,        // Comic Book ZIP
        CBR,        // Comic Book RAR
        ZIP,        // Standard ZIP
        RAR,        // Standard RAR
        SevenZ,     // 7-Zip
        PDF         // PDF (basic support)
    };

    /**
     * @brief 解析状态
     */
    enum ParseStatus {
        NotStarted = 0,
        Parsing,
        Completed,
        Failed
    };

    explicit ComicParser(QObject *parent = nullptr);
    ~ComicParser();

    // 文件操作
    bool openFile(const QString &filePath);
    void closeFile();
    bool isFileOpen() const;
    
    // 格式检测
    static ComicFormat detectFormat(const QString &filePath);
    static bool isSupported(const QString &filePath);
    static QStringList getSupportedFormats();
    
    // 漫画信息
    ComicInfo getComicInfo() const;
    int getPageCount() const;
    QStringList getPageList() const;
    
    // 页面操作
    ComicPage getPage(int pageNumber) const;
    QPixmap getPageImage(int pageNumber) const;
    QByteArray getPageData(int pageNumber) const;
    
    // 批量操作
    QList<ComicPage> getPages(int startPage, int count) const;
    QList<QPixmap> getPageImages(int startPage, int count) const;
    
    // 缓存管理
    void enableCache(bool enabled);
    bool isCacheEnabled() const;
    void clearCache();
    void setCacheSize(int maxPages);
    int getCacheSize() const;
    
    // 异步操作
    void parseAsync(const QString &filePath);
    void loadPageAsync(int pageNumber);
    void preloadPages(int startPage, int count);
    
    // 状态查询
    ParseStatus getParseStatus() const;
    QString getLastError() const;
    double getProgress() const;

signals:
    void parseStarted();
    void parseProgress(int current, int total);
    void parseCompleted(const ComicInfo &info);
    void parseFailed(const QString &error);
    
    void pageLoaded(int pageNumber, const ComicPage &page);
    void pageLoadFailed(int pageNumber, const QString &error);
    
    void preloadProgress(int loaded, int total);
    void preloadCompleted();

private slots:
    void onRarProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onRarProcessError();

private:
    // 格式特定的解析方法
    bool parseZipFile(const QString &filePath);
    bool parseRarFile(const QString &filePath);
    bool parsePdfFile(const QString &filePath);
    
    // 页面排序和过滤
    QStringList sortPageList(const QStringList &fileList) const;
    bool isImageFile(const QString &fileName) const;
    
    // 缓存管理
    void addToCache(int pageNumber, const ComicPage &page) const;
    ComicPage getFromCache(int pageNumber) const;
    bool isInCache(int pageNumber) const;
    void cleanCache();
    
    // RAR工具查找
    QString findRarTool() const;
    
    // ZIP文件处理
    QByteArray extractFromZip(const QString &fileName) const;
    QStringList listZipContents() const;
    
    // RAR文件处理
    QByteArray extractFromRar(const QString &fileName) const;
    QStringList listRarContents() const;
    bool isRarToolAvailable() const;
    
    // 元数据解析
    void parseComicInfo();
    void parseComicInfoXml(const QByteArray &xmlData);
    
    // 工具方法
    QString naturalSort(const QString &str) const;
    QPixmap byteArrayToPixmap(const QByteArray &data) const;
    QSize getImageSize(const QByteArray &data) const;
    
    // 成员变量
    QString m_filePath;
    ComicFormat m_format;
    ComicInfo m_comicInfo;
    QStringList m_pageList;
    
    // 解析状态
    ParseStatus m_parseStatus;
    QString m_lastError;
    double m_progress;
    
    // ZIP文件支持
    QZipReader *m_zipReader;
    
    // RAR文件支持
    QProcess *m_rarProcess;
    QString m_rarToolPath;
    QString m_tempDir;
    
    // 缓存系统
    bool m_cacheEnabled;
    int m_maxCacheSize;
    mutable QMap<int, ComicPage> m_pageCache;
    mutable QMutex m_cacheMutex;
    
    // 支持的图片格式
    static const QStringList SUPPORTED_IMAGE_FORMATS;
    static const QStringList SUPPORTED_COMIC_FORMATS;
};

#endif // COMICPARSER_H