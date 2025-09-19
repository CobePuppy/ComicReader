#include "BookmarkManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QUuid>
#include <QDebug>
#include <QApplication>
#include <QPixmap>
#include <QImageReader>
#include <QMutexLocker>
#include <algorithm>

BookmarkManager* BookmarkManager::m_instance = nullptr;

// BookmarkInfo 实现
QJsonObject BookmarkInfo::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["comicPath"] = comicPath;
    obj["comicTitle"] = comicTitle;
    obj["pageNumber"] = pageNumber;
    obj["title"] = title;
    obj["description"] = description;
    obj["dateCreated"] = dateCreated.toString(Qt::ISODate);
    obj["dateModified"] = dateModified.toString(Qt::ISODate);
    obj["thumbnailPath"] = thumbnailPath;
    obj["isQuickBookmark"] = isQuickBookmark;
    
    QJsonArray tagArray;
    for (const QString &tag : tags) {
        tagArray.append(tag);
    }
    obj["tags"] = tagArray;
    
    return obj;
}

void BookmarkInfo::fromJson(const QJsonObject &json)
{
    id = json["id"].toString();
    comicPath = json["comicPath"].toString();
    comicTitle = json["comicTitle"].toString();
    pageNumber = json["pageNumber"].toInt();
    title = json["title"].toString();
    description = json["description"].toString();
    dateCreated = QDateTime::fromString(json["dateCreated"].toString(), Qt::ISODate);
    dateModified = QDateTime::fromString(json["dateModified"].toString(), Qt::ISODate);
    thumbnailPath = json["thumbnailPath"].toString();
    isQuickBookmark = json["isQuickBookmark"].toBool();
    
    tags.clear();
    QJsonArray tagArray = json["tags"].toArray();
    for (const QJsonValue &value : tagArray) {
        tags.append(value.toString());
    }
}

// ReadingProgress 实现
QJsonObject ReadingProgress::toJson() const
{
    QJsonObject obj;
    obj["comicPath"] = comicPath;
    obj["currentPage"] = currentPage;
    obj["totalPages"] = totalPages;
    obj["lastReadTime"] = lastReadTime.toString(Qt::ISODate);
    obj["readingTime"] = readingTime;
    obj["progress"] = progress;
    obj["isCompleted"] = isCompleted;
    return obj;
}

void ReadingProgress::fromJson(const QJsonObject &json)
{
    comicPath = json["comicPath"].toString();
    currentPage = json["currentPage"].toInt();
    totalPages = json["totalPages"].toInt();
    lastReadTime = QDateTime::fromString(json["lastReadTime"].toString(), Qt::ISODate);
    readingTime = json["readingTime"].toInt();
    progress = json["progress"].toDouble();
    isCompleted = json["isCompleted"].toBool();
}

// BookmarkManager 实现
BookmarkManager::BookmarkManager(QObject *parent)
    : QObject(parent)
    , m_autoSaveEnabled(true)
    , m_autoSaveInterval(5)
    , m_dataChanged(false)
    , m_sessionStartTime(QDateTime::currentMSecsSinceEpoch())
{
    // 设置数据目录
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    m_dataFilePath = dataDir + "/bookmarks.json";
    m_progressFilePath = dataDir + "/reading_progress.json";
    m_thumbnailDirectory = dataDir + "/thumbnails";
    QDir().mkpath(m_thumbnailDirectory);
    
    // 设置自动保存定时器
    setupAutoSave();
    
    // 加载数据
    loadData();
}

BookmarkManager::~BookmarkManager()
{
    saveData();
}

BookmarkManager* BookmarkManager::instance()
{
    if (!m_instance) {
        m_instance = new BookmarkManager();
    }
    return m_instance;
}

void BookmarkManager::setupAutoSave()
{
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &BookmarkManager::onAutoSaveTimer);
    
    if (m_autoSaveEnabled) {
        m_autoSaveTimer->start(m_autoSaveInterval * 60 * 1000); // 转换为毫秒
    }
}

QString BookmarkManager::generateBookmarkId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString BookmarkManager::generateThumbnailPath(const QString &comicPath, int pageNumber) const
{
    QFileInfo fileInfo(comicPath);
    QString baseName = fileInfo.baseName();
    QString fileName = QString("%1_page_%2.jpg").arg(baseName).arg(pageNumber);
    return QDir(m_thumbnailDirectory).absoluteFilePath(fileName);
}

bool BookmarkManager::isValidComicPath(const QString &comicPath) const
{
    QFileInfo fileInfo(comicPath);
    return fileInfo.exists() && fileInfo.isFile();
}

void BookmarkManager::notifyDataChanged()
{
    m_dataChanged = true;
    emit dataChanged();
}

QString BookmarkManager::addBookmark(const QString &comicPath, int pageNumber, 
                                   const QString &title, const QString &description)
{
    if (!isValidComicPath(comicPath) || pageNumber < 0) {
        qWarning() << "Invalid comic path or page number:" << comicPath << pageNumber;
        return QString();
    }
    
    QMutexLocker locker(&m_dataMutex);
    
    QString bookmarkId = generateBookmarkId();
    BookmarkInfo bookmark;
    bookmark.id = bookmarkId;
    bookmark.comicPath = comicPath;
    bookmark.comicTitle = QFileInfo(comicPath).baseName();
    bookmark.pageNumber = pageNumber;
    bookmark.title = title.isEmpty() ? QString("第 %1 页").arg(pageNumber + 1) : title;
    bookmark.description = description;
    bookmark.dateCreated = QDateTime::currentDateTime();
    bookmark.dateModified = bookmark.dateCreated;
    bookmark.thumbnailPath = generateThumbnailPath(comicPath, pageNumber);
    bookmark.isQuickBookmark = false;
    
    m_bookmarks[bookmarkId] = bookmark;
    
    // 更新漫画->书签映射
    m_comicBookmarks[comicPath].append(bookmarkId);
    
    // 生成缩略图（异步）
    updateBookmarkThumbnail(m_bookmarks[bookmarkId]);
    
    notifyDataChanged();
    emit bookmarkAdded(bookmarkId);
    
    return bookmarkId;
}

bool BookmarkManager::removeBookmark(const QString &bookmarkId)
{
    QMutexLocker locker(&m_dataMutex);
    
    auto it = m_bookmarks.find(bookmarkId);
    if (it == m_bookmarks.end()) {
        return false;
    }
    
    BookmarkInfo bookmark = it.value();
    
    // 删除缩略图文件
    if (!bookmark.thumbnailPath.isEmpty()) {
        QFile::remove(bookmark.thumbnailPath);
    }
    
    // 从漫画->书签映射中移除
    if (m_comicBookmarks.contains(bookmark.comicPath)) {
        m_comicBookmarks[bookmark.comicPath].removeAll(bookmarkId);
        if (m_comicBookmarks[bookmark.comicPath].isEmpty()) {
            m_comicBookmarks.remove(bookmark.comicPath);
        }
    }
    
    // 移除书签
    m_bookmarks.erase(it);
    
    notifyDataChanged();
    emit bookmarkRemoved(bookmarkId);
    
    return true;
}

bool BookmarkManager::updateBookmark(const QString &bookmarkId, const BookmarkInfo &info)
{
    QMutexLocker locker(&m_dataMutex);
    
    auto it = m_bookmarks.find(bookmarkId);
    if (it == m_bookmarks.end()) {
        return false;
    }
    
    BookmarkInfo updatedInfo = info;
    updatedInfo.id = bookmarkId;  // 确保ID不变
    updatedInfo.dateModified = QDateTime::currentDateTime();
    
    // 更新标签集合
    for (const QString &tag : updatedInfo.tags) {
        m_allTags.insert(tag);
    }
    
    m_bookmarks[bookmarkId] = updatedInfo;
    
    notifyDataChanged();
    emit bookmarkUpdated(bookmarkId);
    
    return true;
}

BookmarkInfo BookmarkManager::getBookmark(const QString &bookmarkId) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_bookmarks.value(bookmarkId);
}

QList<BookmarkInfo> BookmarkManager::getAllBookmarksForComic(const QString &comicPath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<BookmarkInfo> result;
    QStringList bookmarkIds = m_comicBookmarks.value(comicPath);
    
    for (const QString &id : bookmarkIds) {
        if (m_bookmarks.contains(id)) {
            result.append(m_bookmarks[id]);
        }
    }
    
    // 按页面编号排序
    std::sort(result.begin(), result.end(), 
              [](const BookmarkInfo &a, const BookmarkInfo &b) {
                  return a.pageNumber < b.pageNumber;
              });
    
    return result;
}

QList<BookmarkInfo> BookmarkManager::getAllBookmarksWithTag(const QString &tag) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<BookmarkInfo> result;
    for (const BookmarkInfo &bookmark : m_bookmarks.values()) {
        if (bookmark.tags.contains(tag)) {
            result.append(bookmark);
        }
    }
    
    // 按创建时间排序
    std::sort(result.begin(), result.end(), 
              [](const BookmarkInfo &a, const BookmarkInfo &b) {
                  return a.dateCreated > b.dateCreated;
              });
    
    return result;
}

QList<BookmarkInfo> BookmarkManager::getRecentBookmarks(int count) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<BookmarkInfo> allBookmarks = m_bookmarks.values();
    
    // 按创建时间排序
    std::sort(allBookmarks.begin(), allBookmarks.end(), 
              [](const BookmarkInfo &a, const BookmarkInfo &b) {
                  return a.dateCreated > b.dateCreated;
              });
    
    return allBookmarks.mid(0, count);
}

QList<BookmarkInfo> BookmarkManager::getAllBookmarks() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_bookmarks.values();
}

QString BookmarkManager::addQuickBookmark(const QString &comicPath, int pageNumber)
{
    QString bookmarkId = addBookmark(comicPath, pageNumber, 
                                   QString("快速书签 - 第 %1 页").arg(pageNumber + 1));
    
    if (!bookmarkId.isEmpty()) {
        QMutexLocker locker(&m_dataMutex);
        m_bookmarks[bookmarkId].isQuickBookmark = true;
    }
    
    return bookmarkId;
}

bool BookmarkManager::removeQuickBookmark(const QString &comicPath, int pageNumber)
{
    QMutexLocker locker(&m_dataMutex);
    
    // 查找对应的快速书签
    for (auto it = m_bookmarks.begin(); it != m_bookmarks.end(); ++it) {
        const BookmarkInfo &bookmark = it.value();
        if (bookmark.comicPath == comicPath && 
            bookmark.pageNumber == pageNumber && 
            bookmark.isQuickBookmark) {
            locker.unlock();
            return removeBookmark(it.key());
        }
    }
    
    return false;
}

QList<BookmarkInfo> BookmarkManager::getQuickBookmarks(const QString &comicPath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<BookmarkInfo> result;
    for (const BookmarkInfo &bookmark : m_bookmarks.values()) {
        if (bookmark.comicPath == comicPath && bookmark.isQuickBookmark) {
            result.append(bookmark);
        }
    }
    
    // 按页面编号排序
    std::sort(result.begin(), result.end(), 
              [](const BookmarkInfo &a, const BookmarkInfo &b) {
                  return a.pageNumber < b.pageNumber;
              });
    
    return result;
}

QStringList BookmarkManager::getAllTags() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_allTags.values();
}

bool BookmarkManager::addTagToBookmark(const QString &bookmarkId, const QString &tag)
{
    QMutexLocker locker(&m_dataMutex);
    
    auto it = m_bookmarks.find(bookmarkId);
    if (it == m_bookmarks.end()) {
        return false;
    }
    
    if (!it->tags.contains(tag)) {
        it->tags.append(tag);
        it->dateModified = QDateTime::currentDateTime();
        m_allTags.insert(tag);
        
        locker.unlock();
        notifyDataChanged();
        emit bookmarkUpdated(bookmarkId);
    }
    
    return true;
}

bool BookmarkManager::removeTagFromBookmark(const QString &bookmarkId, const QString &tag)
{
    QMutexLocker locker(&m_dataMutex);
    
    auto it = m_bookmarks.find(bookmarkId);
    if (it == m_bookmarks.end()) {
        return false;
    }
    
    if (it->tags.removeAll(tag) > 0) {
        it->dateModified = QDateTime::currentDateTime();
        
        locker.unlock();
        notifyDataChanged();
        emit bookmarkUpdated(bookmarkId);
        return true;
    }
    
    return false;
}

QStringList BookmarkManager::getTagsForBookmark(const QString &bookmarkId) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_bookmarks.value(bookmarkId).tags;
}

void BookmarkManager::updateReadingProgress(const QString &comicPath, int currentPage, int totalPages)
{
    if (!isValidComicPath(comicPath) || currentPage < 0) {
        return;
    }
    
    QMutexLocker locker(&m_dataMutex);
    
    ReadingProgress &progress = m_readingProgress[comicPath];
    progress.comicPath = comicPath;
    progress.currentPage = currentPage;
    
    if (totalPages > 0) {
        progress.totalPages = totalPages;
        progress.progress = static_cast<double>(currentPage) / totalPages;
        progress.isCompleted = (currentPage >= totalPages - 1);
    }
    
    progress.lastReadTime = QDateTime::currentDateTime();
    
    // 更新会话阅读时间
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_sessionReadingTime.contains(comicPath)) {
        qint64 sessionTime = (currentTime - m_sessionStartTime) / 1000;
        m_sessionReadingTime[comicPath] = sessionTime;
        progress.readingTime += sessionTime;
    } else {
        m_sessionReadingTime[comicPath] = 0;
    }
    
    locker.unlock();
    notifyDataChanged();
    emit readingProgressUpdated(comicPath);
}

ReadingProgress BookmarkManager::getReadingProgress(const QString &comicPath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_readingProgress.value(comicPath);
}

QList<ReadingProgress> BookmarkManager::getAllReadingProgress() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_readingProgress.values();
}

QList<ReadingProgress> BookmarkManager::getRecentReadingProgress(int count) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<ReadingProgress> allProgress = m_readingProgress.values();
    
    // 按最后阅读时间排序
    std::sort(allProgress.begin(), allProgress.end(), 
              [](const ReadingProgress &a, const ReadingProgress &b) {
                  return a.lastReadTime > b.lastReadTime;
              });
    
    return allProgress.mid(0, count);
}

QList<ReadingProgress> BookmarkManager::getUnfinishedComics() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<ReadingProgress> result;
    for (const ReadingProgress &progress : m_readingProgress.values()) {
        if (!progress.isCompleted) {
            result.append(progress);
        }
    }
    
    // 按最后阅读时间排序
    std::sort(result.begin(), result.end(), 
              [](const ReadingProgress &a, const ReadingProgress &b) {
                  return a.lastReadTime > b.lastReadTime;
              });
    
    return result;
}

QList<ReadingProgress> BookmarkManager::getCompletedComics() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<ReadingProgress> result;
    for (const ReadingProgress &progress : m_readingProgress.values()) {
        if (progress.isCompleted) {
            result.append(progress);
        }
    }
    
    // 按完成时间排序
    std::sort(result.begin(), result.end(), 
              [](const ReadingProgress &a, const ReadingProgress &b) {
                  return a.lastReadTime > b.lastReadTime;
              });
    
    return result;
}

QList<BookmarkInfo> BookmarkManager::searchBookmarks(const QString &query) const
{
    if (query.isEmpty()) {
        return QList<BookmarkInfo>();
    }
    
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<BookmarkInfo> result;
    QString lowerQuery = query.toLower();
    
    for (const BookmarkInfo &bookmark : m_bookmarks.values()) {
        bool matches = false;
        
        // 搜索标题
        if (bookmark.title.toLower().contains(lowerQuery)) {
            matches = true;
        }
        // 搜索描述
        else if (bookmark.description.toLower().contains(lowerQuery)) {
            matches = true;
        }
        // 搜索漫画标题
        else if (bookmark.comicTitle.toLower().contains(lowerQuery)) {
            matches = true;
        }
        // 搜索标签
        else {
            for (const QString &tag : bookmark.tags) {
                if (tag.toLower().contains(lowerQuery)) {
                    matches = true;
                    break;
                }
            }
        }
        
        if (matches) {
            result.append(bookmark);
        }
    }
    
    // 按相关性排序（简单实现：按创建时间）
    std::sort(result.begin(), result.end(), 
              [](const BookmarkInfo &a, const BookmarkInfo &b) {
                  return a.dateCreated > b.dateCreated;
              });
    
    return result;
}

QList<BookmarkInfo> BookmarkManager::getBookmarksByDateRange(const QDateTime &startDate, const QDateTime &endDate) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QList<BookmarkInfo> result;
    for (const BookmarkInfo &bookmark : m_bookmarks.values()) {
        if (bookmark.dateCreated >= startDate && bookmark.dateCreated <= endDate) {
            result.append(bookmark);
        }
    }
    
    // 按创建时间排序
    std::sort(result.begin(), result.end(), 
              [](const BookmarkInfo &a, const BookmarkInfo &b) {
                  return a.dateCreated > b.dateCreated;
              });
    
    return result;
}

bool BookmarkManager::exportBookmarks(const QString &filePath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QJsonObject rootObj;
    QJsonArray bookmarksArray;
    
    for (const BookmarkInfo &bookmark : m_bookmarks.values()) {
        bookmarksArray.append(bookmark.toJson());
    }
    
    rootObj["bookmarks"] = bookmarksArray;
    rootObj["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["version"] = "1.0";
    
    QJsonDocument doc(rootObj);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool BookmarkManager::importBookmarks(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in bookmarks file:" << filePath;
        return false;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray bookmarksArray = rootObj["bookmarks"].toArray();
    
    QMutexLocker locker(&m_dataMutex);
    
    int importedCount = 0;
    for (const QJsonValue &value : bookmarksArray) {
        BookmarkInfo bookmark;
        bookmark.fromJson(value.toObject());
        
        if (bookmark.isValid() && !m_bookmarks.contains(bookmark.id)) {
            m_bookmarks[bookmark.id] = bookmark;
            
            // 更新漫画->书签映射
            m_comicBookmarks[bookmark.comicPath].append(bookmark.id);
            
            // 更新标签集合
            for (const QString &tag : bookmark.tags) {
                m_allTags.insert(tag);
            }
            
            importedCount++;
        }
    }
    
    locker.unlock();
    
    if (importedCount > 0) {
        notifyDataChanged();
        qDebug() << "Imported" << importedCount << "bookmarks";
    }
    
    return importedCount > 0;
}

bool BookmarkManager::exportReadingProgress(const QString &filePath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QJsonObject rootObj;
    QJsonArray progressArray;
    
    for (const ReadingProgress &progress : m_readingProgress.values()) {
        progressArray.append(progress.toJson());
    }
    
    rootObj["readingProgress"] = progressArray;
    rootObj["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["version"] = "1.0";
    
    QJsonDocument doc(rootObj);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool BookmarkManager::importReadingProgress(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in reading progress file:" << filePath;
        return false;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray progressArray = rootObj["readingProgress"].toArray();
    
    QMutexLocker locker(&m_dataMutex);
    
    int importedCount = 0;
    for (const QJsonValue &value : progressArray) {
        ReadingProgress progress;
        progress.fromJson(value.toObject());
        
        if (!progress.comicPath.isEmpty()) {
            m_readingProgress[progress.comicPath] = progress;
            importedCount++;
        }
    }
    
    locker.unlock();
    
    if (importedCount > 0) {
        notifyDataChanged();
        qDebug() << "Imported" << importedCount << "reading progress records";
    }
    
    return importedCount > 0;
}

void BookmarkManager::cleanupOrphanedBookmarks()
{
    QMutexLocker locker(&m_dataMutex);
    
    QStringList orphanedIds;
    for (auto it = m_bookmarks.begin(); it != m_bookmarks.end(); ++it) {
        const BookmarkInfo &bookmark = it.value();
        if (!isValidComicPath(bookmark.comicPath)) {
            orphanedIds.append(it.key());
        }
    }
    
    locker.unlock();
    
    for (const QString &id : orphanedIds) {
        removeBookmark(id);
    }
    
    if (!orphanedIds.isEmpty()) {
        qDebug() << "Cleaned up" << orphanedIds.size() << "orphaned bookmarks";
    }
}

void BookmarkManager::cleanupOldProgress(int daysToKeep)
{
    QMutexLocker locker(&m_dataMutex);
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-daysToKeep);
    QStringList toRemove;
    
    for (auto it = m_readingProgress.begin(); it != m_readingProgress.end(); ++it) {
        const ReadingProgress &progress = it.value();
        if (progress.lastReadTime < cutoffDate && !isValidComicPath(progress.comicPath)) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString &comicPath : toRemove) {
        m_readingProgress.remove(comicPath);
    }
    
    locker.unlock();
    
    if (!toRemove.isEmpty()) {
        notifyDataChanged();
        qDebug() << "Cleaned up" << toRemove.size() << "old reading progress records";
    }
}

void BookmarkManager::generateThumbnails()
{
    QMutexLocker locker(&m_dataMutex);
    
    for (auto &bookmark : m_bookmarks) {
        if (bookmark.thumbnailPath.isEmpty() || !QFile::exists(bookmark.thumbnailPath)) {
            updateBookmarkThumbnail(bookmark);
        }
    }
}

void BookmarkManager::updateBookmarkThumbnail(BookmarkInfo &bookmark)
{
    // 这里应该调用漫画阅读器的接口来生成缩略图
    // 暂时留空，等漫画阅读器实现后再补充
    // TODO: 实现缩略图生成逻辑
}

int BookmarkManager::getBookmarkCount() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_bookmarks.size();
}

int BookmarkManager::getBookmarkCountForComic(const QString &comicPath) const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_comicBookmarks.value(comicPath).size();
}

int BookmarkManager::getTotalComicsWithProgress() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    return m_readingProgress.size();
}

qint64 BookmarkManager::getTotalReadingTime() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    qint64 totalTime = 0;
    for (const ReadingProgress &progress : m_readingProgress.values()) {
        totalTime += progress.readingTime;
    }
    
    return totalTime;
}

QHash<QString, int> BookmarkManager::getReadingStatistics() const
{
    QMutexLocker locker(const_cast<QMutex*>(&m_dataMutex));
    
    QHash<QString, int> stats;
    stats["totalBookmarks"] = m_bookmarks.size();
    stats["totalComics"] = m_readingProgress.size();
    stats["completedComics"] = 0;
    stats["unfinishedComics"] = 0;
    
    for (const ReadingProgress &progress : m_readingProgress.values()) {
        if (progress.isCompleted) {
            stats["completedComics"]++;
        } else {
            stats["unfinishedComics"]++;
        }
    }
    
    return stats;
}

void BookmarkManager::saveData()
{
    if (!m_dataChanged) {
        return;
    }
    
    QMutexLocker locker(&m_dataMutex);
    
    // 保存书签数据
    QJsonObject bookmarksRoot;
    QJsonArray bookmarksArray;
    
    for (const BookmarkInfo &bookmark : m_bookmarks.values()) {
        bookmarksArray.append(bookmark.toJson());
    }
    
    bookmarksRoot["bookmarks"] = bookmarksArray;
    bookmarksRoot["saveTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    bookmarksRoot["version"] = "1.0";
    
    QJsonDocument bookmarksDoc(bookmarksRoot);
    
    QFile bookmarksFile(m_dataFilePath);
    if (bookmarksFile.open(QIODevice::WriteOnly)) {
        bookmarksFile.write(bookmarksDoc.toJson());
        bookmarksFile.close();
    } else {
        qWarning() << "Failed to save bookmarks to:" << m_dataFilePath;
    }
    
    // 保存阅读进度数据
    QJsonObject progressRoot;
    QJsonArray progressArray;
    
    for (const ReadingProgress &progress : m_readingProgress.values()) {
        progressArray.append(progress.toJson());
    }
    
    progressRoot["readingProgress"] = progressArray;
    progressRoot["saveTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    progressRoot["version"] = "1.0";
    
    QJsonDocument progressDoc(progressRoot);
    
    QFile progressFile(m_progressFilePath);
    if (progressFile.open(QIODevice::WriteOnly)) {
        progressFile.write(progressDoc.toJson());
        progressFile.close();
    } else {
        qWarning() << "Failed to save reading progress to:" << m_progressFilePath;
    }
    
    m_dataChanged = false;
    qDebug() << "Bookmark data saved successfully";
}

void BookmarkManager::loadData()
{
    QMutexLocker locker(&m_dataMutex);
    
    // 加载书签数据
    QFile bookmarksFile(m_dataFilePath);
    if (bookmarksFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(bookmarksFile.readAll());
        if (!doc.isNull()) {
            QJsonObject rootObj = doc.object();
            QJsonArray bookmarksArray = rootObj["bookmarks"].toArray();
            
            m_bookmarks.clear();
            m_comicBookmarks.clear();
            m_allTags.clear();
            
            for (const QJsonValue &value : bookmarksArray) {
                BookmarkInfo bookmark;
                bookmark.fromJson(value.toObject());
                
                if (bookmark.isValid()) {
                    m_bookmarks[bookmark.id] = bookmark;
                    m_comicBookmarks[bookmark.comicPath].append(bookmark.id);
                    
                    for (const QString &tag : bookmark.tags) {
                        m_allTags.insert(tag);
                    }
                }
            }
            
            qDebug() << "Loaded" << m_bookmarks.size() << "bookmarks";
        }
        bookmarksFile.close();
    }
    
    // 加载阅读进度数据
    QFile progressFile(m_progressFilePath);
    if (progressFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(progressFile.readAll());
        if (!doc.isNull()) {
            QJsonObject rootObj = doc.object();
            QJsonArray progressArray = rootObj["readingProgress"].toArray();
            
            m_readingProgress.clear();
            
            for (const QJsonValue &value : progressArray) {
                ReadingProgress progress;
                progress.fromJson(value.toObject());
                
                if (!progress.comicPath.isEmpty()) {
                    m_readingProgress[progress.comicPath] = progress;
                }
            }
            
            qDebug() << "Loaded" << m_readingProgress.size() << "reading progress records";
        }
        progressFile.close();
    }
    
    m_dataChanged = false;
}

void BookmarkManager::onAutoSaveTimer()
{
    if (m_dataChanged) {
        saveData();
    }
}