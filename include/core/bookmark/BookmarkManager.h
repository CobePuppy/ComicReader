#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QTimer>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>

struct BookmarkInfo {
    QString id;              // 书签唯一ID
    QString comicPath;       // 漫画文件路径
    QString comicTitle;      // 漫画标题
    int pageNumber;          // 页面编号（从0开始）
    QString title;           // 书签标题
    QString description;     // 书签描述
    QDateTime dateCreated;   // 创建时间
    QDateTime dateModified;  // 修改时间
    QString thumbnailPath;   // 缩略图路径
    QStringList tags;        // 标签
    bool isQuickBookmark;    // 是否为快速书签
    
    BookmarkInfo() : pageNumber(0), isQuickBookmark(false) {}
    
    bool isValid() const {
        return !id.isEmpty() && !comicPath.isEmpty() && pageNumber >= 0;
    }
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
};

struct ReadingProgress {
    QString comicPath;       // 漫画文件路径
    int currentPage;         // 当前页面
    int totalPages;          // 总页数
    QDateTime lastReadTime;  // 最后阅读时间
    qint64 readingTime;      // 累计阅读时间（秒）
    double progress;         // 阅读进度（0.0-1.0）
    bool isCompleted;        // 是否已完成
    
    ReadingProgress() : currentPage(0), totalPages(0), readingTime(0), progress(0.0), isCompleted(false) {}
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);
};

class BookmarkManager : public QObject
{
    Q_OBJECT

public:
    static BookmarkManager* instance();
    
    // 书签管理
    QString addBookmark(const QString &comicPath, int pageNumber, 
                       const QString &title = QString(), 
                       const QString &description = QString());
    bool removeBookmark(const QString &bookmarkId);
    bool updateBookmark(const QString &bookmarkId, const BookmarkInfo &info);
    BookmarkInfo getBookmark(const QString &bookmarkId) const;
    
    // 批量操作
    QList<BookmarkInfo> getAllBookmarksForComic(const QString &comicPath) const;
    QList<BookmarkInfo> getAllBookmarksWithTag(const QString &tag) const;
    QList<BookmarkInfo> getRecentBookmarks(int count = 10) const;
    QList<BookmarkInfo> getAllBookmarks() const;
    
    // 快速书签
    QString addQuickBookmark(const QString &comicPath, int pageNumber);
    bool removeQuickBookmark(const QString &comicPath, int pageNumber);
    QList<BookmarkInfo> getQuickBookmarks(const QString &comicPath) const;
    
    // 标签管理
    QStringList getAllTags() const;
    bool addTagToBookmark(const QString &bookmarkId, const QString &tag);
    bool removeTagFromBookmark(const QString &bookmarkId, const QString &tag);
    QStringList getTagsForBookmark(const QString &bookmarkId) const;
    
    // 阅读进度管理
    void updateReadingProgress(const QString &comicPath, int currentPage, int totalPages = -1);
    ReadingProgress getReadingProgress(const QString &comicPath) const;
    QList<ReadingProgress> getAllReadingProgress() const;
    QList<ReadingProgress> getRecentReadingProgress(int count = 10) const;
    QList<ReadingProgress> getUnfinishedComics() const;
    QList<ReadingProgress> getCompletedComics() const;
    
    // 搜索功能
    QList<BookmarkInfo> searchBookmarks(const QString &query) const;
    QList<BookmarkInfo> getBookmarksByDateRange(const QDateTime &startDate, const QDateTime &endDate) const;
    
    // 导入导出
    bool exportBookmarks(const QString &filePath) const;
    bool importBookmarks(const QString &filePath);
    bool exportReadingProgress(const QString &filePath) const;
    bool importReadingProgress(const QString &filePath);
    
    // 维护功能
    void cleanupOrphanedBookmarks();  // 清理无效书签
    void cleanupOldProgress(int daysToKeep = 30);  // 清理旧的阅读记录
    void generateThumbnails();  // 生成缩略图
    
    // 统计信息
    int getBookmarkCount() const;
    int getBookmarkCountForComic(const QString &comicPath) const;
    int getTotalComicsWithProgress() const;
    qint64 getTotalReadingTime() const;
    QHash<QString, int> getReadingStatistics() const;

signals:
    void bookmarkAdded(const QString &bookmarkId);
    void bookmarkRemoved(const QString &bookmarkId);
    void bookmarkUpdated(const QString &bookmarkId);
    void readingProgressUpdated(const QString &comicPath);
    void dataChanged();

public slots:
    void saveData();
    void loadData();
    void onAutoSaveTimer();

private:
    explicit BookmarkManager(QObject *parent = nullptr);
    ~BookmarkManager();
    
    void setupAutoSave();
    QString generateBookmarkId() const;
    QString generateThumbnailPath(const QString &comicPath, int pageNumber) const;
    void updateBookmarkThumbnail(BookmarkInfo &bookmark);
    bool isValidComicPath(const QString &comicPath) const;
    void notifyDataChanged();
    
    // 数据存储
    QHash<QString, BookmarkInfo> m_bookmarks;          // 书签数据
    QHash<QString, ReadingProgress> m_readingProgress; // 阅读进度
    QHash<QString, QStringList> m_comicBookmarks;      // 漫画->书签ID映射
    QSet<QString> m_allTags;                           // 所有标签
    
    // 配置
    QString m_dataFilePath;
    QString m_progressFilePath;
    QString m_thumbnailDirectory;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;  // 分钟
    
    // 内部状态
    QTimer *m_autoSaveTimer;
    QMutex m_dataMutex;
    bool m_dataChanged;
    qint64 m_sessionStartTime;
    QHash<QString, qint64> m_sessionReadingTime;  // 本次会话的阅读时间
    
    static BookmarkManager *m_instance;
};

#endif // BOOKMARKMANAGER_H