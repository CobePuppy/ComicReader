#include "../../include/core/ComicParser.h"
#include "../../include/utils/FileUtils.h"
#include "../../include/utils/error/ErrorHandler.h"
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QProcess>
#include <QPixmap>
#include <QBuffer>
#include <QImageReader>
#include <QStandardPaths>
#include <QDebug>
#include <QCollator>
#include <QZipReader>

QStringList ComicParser::s_supportedImageExtensions = 
    QStringList() << "jpg" << "jpeg" << "png" << "gif" << "bmp" << "webp" << "tiff" << "svg";

ComicParser::ComicParser(QObject *parent)
    : QObject(parent)
    , m_zipReader(nullptr)
    , m_rarProcess(nullptr)
{
    // 初始化漫画信息
    m_comicInfo = ComicInfo();
}

ComicParser::~ComicParser()
{
    cleanup();
}

bool ComicParser::parseComic(const QString &filePath)
{
    cleanup();
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QString errorMsg = tr("文件不存在: %1").arg(filePath);
        REPORT_ERROR_WITH_CONTEXT(ErrorCategory::FileIO, "FILE_NOT_FOUND", 
                                  "文件不存在", errorMsg, filePath);
        emit error(errorMsg);
        return false;
    }
    
    // 检测文件格式
    ComicFormat format = detectFormat(filePath);
    if (format == Unknown) {
        QString errorMsg = tr("不支持的文件格式: %1").arg(filePath);
        REPORT_ERROR_WITH_CONTEXT(ErrorCategory::Parsing, "UNSUPPORTED_FORMAT", 
                                  "不支持的文件格式", errorMsg, filePath);
        emit error(errorMsg);
        return false;
    }
    
    // 初始化漫画信息
    m_comicInfo.filePath = filePath;
    m_comicInfo.title = fileInfo.baseName();
    m_comicInfo.format = format;
    m_comicInfo.fileSize = fileInfo.size();
    m_comicInfo.pageCount = 0;
    m_comicInfo.pageList.clear();
    
    // 根据格式解析文件
    bool success = false;
    switch (format) {
        case CBZ:
        case ZIP:
            success = parseZipFile(filePath);
            break;
        case CBR:
        case RAR:
            success = parseRarFile(filePath);
            break;
        default:
            emit error(tr("未知的文件格式"));
            return false;
    }
    
    if (success) {
        sortPages();
        m_comicInfo.pageCount = m_comicInfo.pageList.size();
        
        // 设置封面
        if (!m_comicInfo.pageList.isEmpty()) {
            m_comicInfo.coverPath = m_comicInfo.pageList.first();
        }
    }
    
    emit parseCompleted(success);
    return success;
}

bool ComicParser::parseZipFile(const QString &filePath)
{
    try {
        m_zipReader = new QZipReader(filePath);
        
        if (!m_zipReader->isReadable()) {
            QString errorMsg = tr("无法读取ZIP文件: %1").arg(filePath);
            REPORT_ERROR_WITH_CONTEXT(ErrorCategory::FileIO, "ZIP_READ_ERROR", 
                                      "ZIP文件读取失败", errorMsg, filePath);
            emit error(errorMsg);
            return false;
        }
        
        QStringList fileList;
        const auto entries = m_zipReader->fileInfoList();
        
        for (const auto &entry : entries) {
            if (!entry.isDir && isSupportedImageFormat(entry.filePath)) {
                fileList.append(entry.filePath);
            }
        }
        
        if (fileList.isEmpty()) {
            QString errorMsg = tr("ZIP文件中未找到图片文件");
            REPORT_ERROR_WITH_CONTEXT(ErrorCategory::Parsing, "NO_IMAGES_FOUND", 
                                      "未找到图片文件", errorMsg, filePath);
            emit error(errorMsg);
            return false;
        }
        
        extractPageList(fileList);
        return true;
        
    } catch (const std::exception &e) {
        emit error(tr("解析ZIP文件时发生错误: %1").arg(e.what()));
        return false;
    }
}

bool ComicParser::parseRarFile(const QString &filePath)
{
    // 检查是否安装了 unrar 工具
    QProcess rarCheck;
    rarCheck.start("unrar", QStringList() << "t" << filePath);
    rarCheck.waitForFinished(5000);
    
    if (rarCheck.error() == QProcess::FailedToStart) {
        emit error(tr("需要安装 unrar 工具来解析RAR文件"));
        return false;
    }
    
    // 创建临时目录
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        emit error(tr("无法创建临时目录"));
        return false;
    }
    
    m_tempDir = tempDir.path();
    
    // 列出RAR文件内容
    m_rarProcess = new QProcess(this);
    connect(m_rarProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ComicParser::onExtractionFinished);
    
    QStringList args;
    args << "lb" << filePath; // lb: 列出文件，不显示额外信息
    
    m_rarProcess->start("unrar", args);
    
    if (!m_rarProcess->waitForFinished(10000)) {
        emit error(tr("RAR文件解析超时"));
        return false;
    }
    
    if (m_rarProcess->exitCode() != 0) {
        emit error(tr("无法读取RAR文件内容"));
        return false;
    }
    
    QString output = m_rarProcess->readAllStandardOutput();
    QStringList fileList = output.split('\n', Qt::SkipEmptyParts);
    
    // 过滤图片文件
    QStringList imageFiles;
    for (const QString &fileName : fileList) {
        QString trimmed = fileName.trimmed();
        if (isSupportedImageFormat(trimmed)) {
            imageFiles.append(trimmed);
        }
    }
    
    if (imageFiles.isEmpty()) {
        emit error(tr("RAR文件中未找到图片文件"));
        return false;
    }
    
    extractPageList(imageFiles);
    return true;
}

void ComicParser::extractPageList(const QStringList &fileList)
{
    m_comicInfo.pageList = fileList;
    
    // 发送进度信号
    int total = fileList.size();
    for (int i = 0; i < total; ++i) {
        emit parseProgress(i + 1, total);
    }
}

void ComicParser::sortPages()
{
    // 使用自然排序算法，确保 page1.jpg, page2.jpg, ..., page10.jpg 的正确顺序
    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    
    std::sort(m_comicInfo.pageList.begin(), m_comicInfo.pageList.end(),
              [&collator](const QString &a, const QString &b) {
                  return collator.compare(a, b) < 0;
              });
}

QByteArray ComicParser::getPageData(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_comicInfo.pageList.size()) {
        return QByteArray();
    }
    
    const QString &pagePath = m_comicInfo.pageList.at(pageIndex);
    
    switch (m_comicInfo.format) {
        case CBZ:
        case ZIP: {
            if (m_zipReader && m_zipReader->exists(pagePath)) {
                return m_zipReader->fileData(pagePath);
            }
            break;
        }
        case CBR:
        case RAR: {
            // 对于RAR文件，需要临时提取文件
            QProcess rarExtract;
            QStringList args;
            args << "p" << "-inul" << m_comicInfo.filePath << pagePath;
            
            rarExtract.start("unrar", args);
            if (rarExtract.waitForFinished(5000)) {
                return rarExtract.readAllStandardOutput();
            }
            break;
        }
        default:
            break;
    }
    
    return QByteArray();
}

QPixmap ComicParser::getPagePixmap(int pageIndex) const
{
    QByteArray imageData = getPageData(pageIndex);
    if (imageData.isEmpty()) {
        return QPixmap();
    }
    
    QPixmap pixmap;
    pixmap.loadFromData(imageData);
    return pixmap;
}

ComicParser::ComicFormat ComicParser::detectFormat(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "cbz") {
        return CBZ;
    } else if (suffix == "cbr") {
        return CBR;
    } else if (suffix == "zip") {
        return ZIP;
    } else if (suffix == "rar") {
        return RAR;
    }
    
    return Unknown;
}

QString ComicParser::formatToString(ComicFormat format)
{
    switch (format) {
        case CBZ: return "CBZ";
        case CBR: return "CBR"; 
        case ZIP: return "ZIP";
        case RAR: return "RAR";
        default: return "Unknown";
    }
}

bool ComicParser::isSupportedImageFormat(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    QString suffix = fileInfo.suffix().toLower();
    return s_supportedImageExtensions.contains(suffix);
}

void ComicParser::onExtractionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    
    // RAR处理完成的回调
    // 这里可以添加额外的处理逻辑
}

void ComicParser::cleanup()
{
    if (m_zipReader) {
        delete m_zipReader;
        m_zipReader = nullptr;
    }
    
    if (m_rarProcess) {
        if (m_rarProcess->state() != QProcess::NotRunning) {
            m_rarProcess->kill();
            m_rarProcess->waitForFinished(3000);
        }
        m_rarProcess->deleteLater();
        m_rarProcess = nullptr;
    }
    
    if (!m_tempDir.isEmpty()) {
        QDir dir(m_tempDir);
        if (dir.exists()) {
            dir.removeRecursively();
        }
        m_tempDir.clear();
    }
}
