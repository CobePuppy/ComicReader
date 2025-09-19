#include "core/parsers/ComicParser.h"
#include "core/utils/FileUtils.h"
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QBuffer>
#include <QImageReader>
#include <QMutexLocker>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <algorithm>

// 静态成员定义
const QStringList ComicParser::SUPPORTED_IMAGE_FORMATS = {
    "jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif", "webp"
};

const QStringList ComicParser::SUPPORTED_COMIC_FORMATS = {
    "cbz", "cbr", "zip", "rar", "7z", "pdf"
};

ComicParser::ComicParser(QObject *parent)
    : QObject(parent)
    , m_format(Unknown)
    , m_parseStatus(NotStarted)
    , m_progress(0.0)
    , m_zipReader(nullptr)
    , m_rarProcess(nullptr)
    , m_cacheEnabled(true)
    , m_maxCacheSize(10)
{
    // 创建临时目录
    m_tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/ComicReader";
    QDir().mkpath(m_tempDir);
    
    // 初始化RAR进程
    m_rarProcess = new QProcess(this);
    connect(m_rarProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ComicParser::onRarProcessFinished);
    connect(m_rarProcess, &QProcess::errorOccurred,
            this, &ComicParser::onRarProcessError);
    
    // 设置RAR工具路径（简化实现）
    m_rarToolPath = QString();
}

ComicParser::~ComicParser()
{
    closeFile();
    
    // 清理临时目录
    QDir tempDir(m_tempDir);
    if (tempDir.exists()) {
        tempDir.removeRecursively();
    }
}

bool ComicParser::openFile(const QString &filePath)
{
    if (filePath == m_filePath && m_parseStatus == Completed) {
        return true; // 文件已经打开
    }
    
    closeFile();
    
    if (!FileUtils::exists(filePath) || !FileUtils::isReadable(filePath)) {
        m_lastError = "文件不存在或无法读取: " + filePath;
        return false;
    }
    
    m_filePath = filePath;
    m_format = detectFormat(filePath);
    
    if (m_format == Unknown) {
        m_lastError = "不支持的文件格式: " + filePath;
        return false;
    }
    
    // 初始化漫画信息
    m_comicInfo = ComicInfo();
    m_comicInfo.filePath = filePath;
    m_comicInfo.fileName = FileUtils::fileName(filePath);
    m_comicInfo.fileSize = FileUtils::fileSize(filePath);
    m_comicInfo.format = FileUtils::suffix(filePath).toUpper();
    
    m_parseStatus = Parsing;
    emit parseStarted();
    
    bool success = false;
    switch (m_format) {
        case CBZ:
        case ZIP:
            success = parseZipFile(filePath);
            break;
        case CBR:
        case RAR:
            success = parseRarFile(filePath);
            break;
        case PDF:
            success = parsePdfFile(filePath);
            break;
        default:
            m_lastError = "未实现的格式支持";
            break;
    }
    
    if (success) {
        m_parseStatus = Completed;
        m_comicInfo.pageCount = m_pageList.size();
        
        // 尝试加载封面
        if (!m_pageList.isEmpty()) {
            ComicPage firstPage = getPage(0);
            if (firstPage.isValid()) {
                m_comicInfo.coverImage = byteArrayToPixmap(firstPage.data);
            }
        }
        
        emit parseCompleted(m_comicInfo);
    } else {
        m_parseStatus = Failed;
        emit parseFailed(m_lastError);
    }
    
    return success;
}

void ComicParser::closeFile()
{
    if (m_zipReader) {
        // TODO: 正确删除ZIP阅读器
        m_zipReader = nullptr;
    }
    
    if (m_rarProcess && m_rarProcess->state() != QProcess::NotRunning) {
        m_rarProcess->kill();
        m_rarProcess->waitForFinished(3000);
    }
    
    clearCache();
    
    m_filePath.clear();
    m_format = Unknown;
    m_parseStatus = NotStarted;
    m_pageList.clear();
    m_comicInfo = ComicInfo();
    m_lastError.clear();
    m_progress = 0.0;
}

bool ComicParser::isFileOpen() const
{
    return m_parseStatus == Completed;
}

ComicParser::ComicFormat ComicParser::detectFormat(const QString &filePath)
{
    QString suffix = FileUtils::suffix(filePath);
    
    if (suffix == "cbz") return CBZ;
    if (suffix == "cbr") return CBR;
    if (suffix == "zip") return ZIP;
    if (suffix == "rar") return RAR;
    if (suffix == "7z") return SevenZ;
    if (suffix == "pdf") return PDF;
    
    return Unknown;
}

bool ComicParser::isSupported(const QString &filePath)
{
    return detectFormat(filePath) != Unknown;
}

QStringList ComicParser::getSupportedFormats()
{
    return SUPPORTED_COMIC_FORMATS;
}

ComicInfo ComicParser::getComicInfo() const
{
    return m_comicInfo;
}

int ComicParser::getPageCount() const
{
    return m_pageList.size();
}

QStringList ComicParser::getPageList() const
{
    return m_pageList;
}

ComicPage ComicParser::getPage(int pageNumber) const
{
    if (pageNumber < 0 || pageNumber >= m_pageList.size()) {
        return ComicPage();
    }
    
    // 检查缓存
    if (m_cacheEnabled && isInCache(pageNumber)) {
        return getFromCache(pageNumber);
    }
    
    ComicPage page;
    page.fileName = m_pageList[pageNumber];
    page.pageNumber = pageNumber;
    
    // 提取页面数据
    switch (m_format) {
        case CBZ:
        case ZIP:
            page.data = extractFromZip(page.fileName);
            break;
        case CBR:
        case RAR:
            page.data = extractFromRar(page.fileName);
            break;
        default:
            return ComicPage();
    }
    
    if (!page.data.isEmpty()) {
        page.size = getImageSize(page.data);
        page.fileSize = page.data.size();
        page.format = FileUtils::suffix(page.fileName).toUpper();
        
        // 添加到缓存
        if (m_cacheEnabled) {
            addToCache(pageNumber, page);
        }
    }
    
    return page;
}

QPixmap ComicParser::getPageImage(int pageNumber) const
{
    ComicPage page = getPage(pageNumber);
    if (page.isValid()) {
        return byteArrayToPixmap(page.data);
    }
    return QPixmap();
}

QByteArray ComicParser::getPageData(int pageNumber) const
{
    ComicPage page = getPage(pageNumber);
    return page.data;
}

// 缓存管理
void ComicParser::enableCache(bool enabled)
{
    m_cacheEnabled = enabled;
    if (!enabled) {
        clearCache();
    }
}

bool ComicParser::isCacheEnabled() const
{
    return m_cacheEnabled;
}

void ComicParser::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_pageCache.clear();
}

void ComicParser::setCacheSize(int maxPages)
{
    m_maxCacheSize = qMax(1, maxPages);
    cleanCache();
}

int ComicParser::getCacheSize() const
{
    return m_maxCacheSize;
}

// 解析ZIP文件
bool ComicParser::parseZipFile(const QString &filePath)
{
    Q_UNUSED(filePath)
    
    // 简化实现 - 这里需要使用第三方ZIP库
    // 为了编译通过，暂时返回false并设置占位符
    m_lastError = "ZIP解析功能开发中...";
    
    // TODO: 实现ZIP文件解析
    // 1. 打开ZIP文件
    // 2. 列出所有图片文件
    // 3. 对文件列表进行自然排序
    // 4. 设置页面列表
    
    // 临时实现 - 创建模拟数据用于测试
    QStringList testPages;
    testPages << "page01.jpg" << "page02.jpg" << "page03.jpg";
    m_pageList = sortPageList(testPages);
    
    return !m_pageList.isEmpty();
}

// 解析RAR文件  
bool ComicParser::parseRarFile(const QString &filePath)
{
    Q_UNUSED(filePath)
    
    m_lastError = "RAR解析功能开发中...";
    
    // TODO: 实现RAR文件解析
    // 需要使用unrar工具或第三方RAR库
    
    return false;
}

// 解析PDF文件
bool ComicParser::parsePdfFile(const QString &filePath)
{
    Q_UNUSED(filePath)
    
    m_lastError = "PDF解析功能开发中...";
    
    // TODO: 实现PDF文件解析
    // 需要使用PDF库如Poppler-Qt
    
    return false;
}

// 页面排序和过滤
QStringList ComicParser::sortPageList(const QStringList &fileList) const
{
    QStringList imageFiles;
    
    // 过滤出图片文件
    for (const QString &fileName : fileList) {
        if (isImageFile(fileName)) {
            imageFiles.append(fileName);
        }
    }
    
    // 自然排序
    std::sort(imageFiles.begin(), imageFiles.end(), [this](const QString &a, const QString &b) {
        return naturalSort(a) < naturalSort(b);
    });
    
    return imageFiles;
}

bool ComicParser::isImageFile(const QString &fileName) const
{
    QString suffix = FileUtils::suffix(fileName);
    return SUPPORTED_IMAGE_FORMATS.contains(suffix, Qt::CaseInsensitive);
}

// 缓存操作
void ComicParser::addToCache(int pageNumber, const ComicPage &page) const
{
    QMutexLocker locker(&m_cacheMutex);
    
    // 清理缓存空间
    while (m_pageCache.size() >= m_maxCacheSize) {
        auto it = m_pageCache.begin();
        m_pageCache.erase(it);
    }
    
    m_pageCache[pageNumber] = page;
}

ComicPage ComicParser::getFromCache(int pageNumber) const
{
    QMutexLocker locker(&m_cacheMutex);
    return m_pageCache.value(pageNumber);
}

bool ComicParser::isInCache(int pageNumber) const
{
    QMutexLocker locker(&m_cacheMutex);
    return m_pageCache.contains(pageNumber);
}

void ComicParser::cleanCache()
{
    QMutexLocker locker(&m_cacheMutex);
    
    while (m_pageCache.size() > m_maxCacheSize) {
        auto it = m_pageCache.begin();
        m_pageCache.erase(it);
    }
}

// ZIP操作占位符
QByteArray ComicParser::extractFromZip(const QString &fileName) const
{
    Q_UNUSED(fileName)
    // TODO: 实现ZIP文件提取
    return QByteArray();
}

QStringList ComicParser::listZipContents() const
{
    // TODO: 实现ZIP内容列表
    return QStringList();
}

// RAR操作占位符
QByteArray ComicParser::extractFromRar(const QString &fileName) const
{
    Q_UNUSED(fileName)
    // TODO: 实现RAR文件提取
    return QByteArray();
}

QStringList ComicParser::listRarContents() const
{
    // TODO: 实现RAR内容列表
    return QStringList();
}

bool ComicParser::isRarToolAvailable() const
{
    return !m_rarToolPath.isEmpty();
}

QString ComicParser::findRarTool() const
{
    // TODO: 查找系统中的RAR工具
    // 常见路径：C:\Program Files\WinRAR\UnRAR.exe
    QStringList possiblePaths = {
        "C:/Program Files/WinRAR/UnRAR.exe",
        "C:/Program Files (x86)/WinRAR/UnRAR.exe",
        "unrar.exe"  // 系统PATH中
    };
    
    for (const QString &path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
    
    return QString();
}

// 进程回调
void ComicParser::onRarProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    // TODO: 处理RAR进程完成
}

void ComicParser::onRarProcessError()
{
    // TODO: 处理RAR进程错误
}

// 工具方法
QString ComicParser::naturalSort(const QString &str) const
{
    // 简单的自然排序实现
    QString result = str.toLower();
    
    // 数字部分补零以便正确排序
    QRegularExpression re("(\\d+)");
    QRegularExpressionMatchIterator it = re.globalMatch(result);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString number = match.captured(1);
        QString paddedNumber = number.rightJustified(10, '0');
        result.replace(match.capturedStart(1), number.length(), paddedNumber);
    }
    
    return result;
}

QPixmap ComicParser::byteArrayToPixmap(const QByteArray &data) const
{
    QPixmap pixmap;
    pixmap.loadFromData(data);
    return pixmap;
}

QSize ComicParser::getImageSize(const QByteArray &data) const
{
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    
    QImageReader reader(&buffer);
    return reader.size();
}

// 状态查询
ComicParser::ParseStatus ComicParser::getParseStatus() const
{
    return m_parseStatus;
}

QString ComicParser::getLastError() const
{
    return m_lastError;
}

double ComicParser::getProgress() const
{
    return m_progress;
}