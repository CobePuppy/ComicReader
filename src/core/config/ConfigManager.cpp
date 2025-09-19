#include "core/config/ConfigManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

// 静态成员定义
ConfigManager* ConfigManager::m_instance = nullptr;

// 默认值定义
const QSize ConfigManager::DEFAULT_WINDOW_SIZE = QSize(1200, 800);
const QPoint ConfigManager::DEFAULT_WINDOW_POSITION = QPoint(100, 100);
const int ConfigManager::DEFAULT_PAGE_FIT_MODE = 0; // 适应窗口
const double ConfigManager::DEFAULT_ZOOM_LEVEL = 1.0;
const int ConfigManager::DEFAULT_READING_DIRECTION = 0; // 从左到右
const QString ConfigManager::DEFAULT_LANGUAGE = "zh_CN";
const QString ConfigManager::DEFAULT_THEME = "default";
const int ConfigManager::DEFAULT_MAX_CACHE_SIZE = 500; // MB
const int ConfigManager::DEFAULT_PRELOAD_COUNT = 3;
const int ConfigManager::MAX_RECENT_FILES = 10;

ConfigManager* ConfigManager::instance()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    
    if (!m_instance) {
        m_instance = new ConfigManager();
    }
    return m_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
{
    // 创建配置文件路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    // 初始化QSettings
    m_settings = new QSettings(configPath + "/config.ini", QSettings::IniFormat, this);
    
    initDefaults();
    loadSettings();
}

ConfigManager::~ConfigManager()
{
    saveSettings();
}

void ConfigManager::initDefaults()
{
    // 如果是第一次运行，设置默认值
    if (m_settings->allKeys().isEmpty()) {
        setWindowSize(DEFAULT_WINDOW_SIZE);
        setWindowPosition(DEFAULT_WINDOW_POSITION);
        setMaximized(false);
        setFullscreen(false);
        
        setPageFitMode(DEFAULT_PAGE_FIT_MODE);
        setZoomLevel(DEFAULT_ZOOM_LEVEL);
        setReadingDirection(DEFAULT_READING_DIRECTION);
        setDoublePageMode(false);
        
        setLanguage(DEFAULT_LANGUAGE);
        setTheme(DEFAULT_THEME);
        
        // 设置默认缓存目录
        QString defaultCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        setCacheDirectory(defaultCacheDir);
        setMaxCacheSize(DEFAULT_MAX_CACHE_SIZE);
        setPreloadEnabled(true);
        setPreloadCount(DEFAULT_PRELOAD_COUNT);
        
        sync();
    }
}

void ConfigManager::loadSettings()
{
    // 配置已经通过QSettings自动加载
}

void ConfigManager::saveSettings()
{
    if (m_settings) {
        m_settings->sync();
    }
}

// 窗口设置
QSize ConfigManager::getWindowSize() const
{
    return m_settings->value("Window/Size", DEFAULT_WINDOW_SIZE).toSize();
}

void ConfigManager::setWindowSize(const QSize &size)
{
    m_settings->setValue("Window/Size", size);
    emit configChanged("Window/Size", size);
}

QPoint ConfigManager::getWindowPosition() const
{
    return m_settings->value("Window/Position", DEFAULT_WINDOW_POSITION).toPoint();
}

void ConfigManager::setWindowPosition(const QPoint &position)
{
    m_settings->setValue("Window/Position", position);
    emit configChanged("Window/Position", position);
}

bool ConfigManager::isMaximized() const
{
    return m_settings->value("Window/Maximized", false).toBool();
}

void ConfigManager::setMaximized(bool maximized)
{
    m_settings->setValue("Window/Maximized", maximized);
    emit configChanged("Window/Maximized", maximized);
}

bool ConfigManager::isFullscreen() const
{
    return m_settings->value("Window/Fullscreen", false).toBool();
}

void ConfigManager::setFullscreen(bool fullscreen)
{
    m_settings->setValue("Window/Fullscreen", fullscreen);
    emit configChanged("Window/Fullscreen", fullscreen);
}

// 阅读设置
int ConfigManager::getPageFitMode() const
{
    return m_settings->value("Reading/PageFitMode", DEFAULT_PAGE_FIT_MODE).toInt();
}

void ConfigManager::setPageFitMode(int mode)
{
    m_settings->setValue("Reading/PageFitMode", mode);
    emit configChanged("Reading/PageFitMode", mode);
}

double ConfigManager::getZoomLevel() const
{
    return m_settings->value("Reading/ZoomLevel", DEFAULT_ZOOM_LEVEL).toDouble();
}

void ConfigManager::setZoomLevel(double level)
{
    m_settings->setValue("Reading/ZoomLevel", level);
    emit configChanged("Reading/ZoomLevel", level);
}

int ConfigManager::getReadingDirection() const
{
    return m_settings->value("Reading/Direction", DEFAULT_READING_DIRECTION).toInt();
}

void ConfigManager::setReadingDirection(int direction)
{
    m_settings->setValue("Reading/Direction", direction);
    emit configChanged("Reading/Direction", direction);
}

bool ConfigManager::isDoublePageMode() const
{
    return m_settings->value("Reading/DoublePageMode", false).toBool();
}

void ConfigManager::setDoublePageMode(bool enabled)
{
    m_settings->setValue("Reading/DoublePageMode", enabled);
    emit configChanged("Reading/DoublePageMode", enabled);
}

// 应用设置
QString ConfigManager::getLanguage() const
{
    return m_settings->value("Application/Language", DEFAULT_LANGUAGE).toString();
}

void ConfigManager::setLanguage(const QString &language)
{
    m_settings->setValue("Application/Language", language);
    emit configChanged("Application/Language", language);
    emit languageChanged(language);
}

QString ConfigManager::getTheme() const
{
    return m_settings->value("Application/Theme", DEFAULT_THEME).toString();
}

void ConfigManager::setTheme(const QString &theme)
{
    m_settings->setValue("Application/Theme", theme);
    emit configChanged("Application/Theme", theme);
    emit themeChanged(theme);
}

QStringList ConfigManager::getRecentFiles() const
{
    return m_settings->value("Application/RecentFiles").toStringList();
}

void ConfigManager::addRecentFile(const QString &filePath)
{
    QStringList recent = getRecentFiles();
    
    // 移除已存在的文件路径
    recent.removeAll(filePath);
    
    // 添加到列表开头
    recent.prepend(filePath);
    
    // 限制最大数量
    while (recent.size() > MAX_RECENT_FILES) {
        recent.removeLast();
    }
    
    m_settings->setValue("Application/RecentFiles", recent);
    emit configChanged("Application/RecentFiles", recent);
}

void ConfigManager::removeRecentFile(const QString &filePath)
{
    QStringList recent = getRecentFiles();
    recent.removeAll(filePath);
    m_settings->setValue("Application/RecentFiles", recent);
    emit configChanged("Application/RecentFiles", recent);
}

void ConfigManager::clearRecentFiles()
{
    m_settings->setValue("Application/RecentFiles", QStringList());
    emit configChanged("Application/RecentFiles", QStringList());
}

// 缓存设置
QString ConfigManager::getCacheDirectory() const
{
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return m_settings->value("Cache/Directory", defaultDir).toString();
}

void ConfigManager::setCacheDirectory(const QString &path)
{
    m_settings->setValue("Cache/Directory", path);
    emit configChanged("Cache/Directory", path);
}

int ConfigManager::getMaxCacheSize() const
{
    return m_settings->value("Cache/MaxSize", DEFAULT_MAX_CACHE_SIZE).toInt();
}

void ConfigManager::setMaxCacheSize(int sizeInMB)
{
    m_settings->setValue("Cache/MaxSize", sizeInMB);
    emit configChanged("Cache/MaxSize", sizeInMB);
}

bool ConfigManager::isPreloadEnabled() const
{
    return m_settings->value("Cache/PreloadEnabled", true).toBool();
}

void ConfigManager::setPreloadEnabled(bool enabled)
{
    m_settings->setValue("Cache/PreloadEnabled", enabled);
    emit configChanged("Cache/PreloadEnabled", enabled);
}

int ConfigManager::getPreloadCount() const
{
    return m_settings->value("Cache/PreloadCount", DEFAULT_PRELOAD_COUNT).toInt();
}

void ConfigManager::setPreloadCount(int count)
{
    m_settings->setValue("Cache/PreloadCount", count);
    emit configChanged("Cache/PreloadCount", count);
}

// 通用方法
QVariant ConfigManager::getValue(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
    emit configChanged(key, value);
}

void ConfigManager::resetToDefaults()
{
    m_settings->clear();
    initDefaults();
    emit configChanged("", QVariant()); // 通知所有配置已重置
}

void ConfigManager::sync()
{
    m_settings->sync();
}
