#include "../../include/core/ConfigManager.h"
#include <QStandardPaths>
#include <QDir>

ConfigManager* ConfigManager::m_instance = nullptr;

ConfigManager* ConfigManager::instance()
{
    if (!m_instance) {
        m_instance = new ConfigManager();
    }
    return m_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings("ComicReader", "ComicReader", this))
{
    loadConfig();
}

ConfigManager::~ConfigManager()
{
    saveConfig();
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
    emit configChanged(key, value);
}

QVariant ConfigManager::getValue(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

QString ConfigManager::getDownloadPath() const
{
    return getValue("Download/Path", 
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/ComicReader").toString();
}

void ConfigManager::setDownloadPath(const QString &path)
{
    setValue("Download/Path", path);
}

QString ConfigManager::getCachePath() const
{
    return getValue("Cache/Path", 
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString();
}

void ConfigManager::setCachePath(const QString &path)
{
    setValue("Cache/Path", path);
}

int ConfigManager::getMaxDownloads() const
{
    return getValue("Download/MaxConcurrent", 3).toInt();
}

void ConfigManager::setMaxDownloads(int count)
{
    setValue("Download/MaxConcurrent", count);
}

int ConfigManager::getMaxDownloadSpeed() const
{
    return getValue("Download/MaxSpeed", 0).toInt();
}

void ConfigManager::setMaxDownloadSpeed(int speed)
{
    setValue("Download/MaxSpeed", speed);
}

QString ConfigManager::getUserAgent() const
{
    return getValue("Network/UserAgent", "ComicReader/1.0").toString();
}

void ConfigManager::setUserAgent(const QString &userAgent)
{
    setValue("Network/UserAgent", userAgent);
}

int ConfigManager::getTimeout() const
{
    return getValue("Network/Timeout", 30).toInt();
}

void ConfigManager::setTimeout(int timeout)
{
    setValue("Network/Timeout", timeout);
}

bool ConfigManager::isAutoStart() const
{
    return getValue("General/AutoStart", false).toBool();
}

void ConfigManager::setAutoStart(bool enabled)
{
    setValue("General/AutoStart", enabled);
}

QString ConfigManager::getTheme() const
{
    return getValue("General/Theme", "default").toString();
}

void ConfigManager::setTheme(const QString &theme)
{
    setValue("General/Theme", theme);
}

QString ConfigManager::getLanguage() const
{
    return getValue("General/Language", "zh_CN").toString();
}

void ConfigManager::setLanguage(const QString &language)
{
    setValue("General/Language", language);
}

bool ConfigManager::isProxyEnabled() const
{
    return getValue("Network/ProxyEnabled", false).toBool();
}

void ConfigManager::setProxyEnabled(bool enabled)
{
    setValue("Network/ProxyEnabled", enabled);
}

QString ConfigManager::getProxyHost() const
{
    return getValue("Network/ProxyHost", "").toString();
}

void ConfigManager::setProxyHost(const QString &host)
{
    setValue("Network/ProxyHost", host);
}

int ConfigManager::getProxyPort() const
{
    return getValue("Network/ProxyPort", 8080).toInt();
}

void ConfigManager::setProxyPort(int port)
{
    setValue("Network/ProxyPort", port);
}

bool ConfigManager::isFullscreenOnStart() const
{
    return getValue("Reader/FullscreenOnStart", false).toBool();
}

void ConfigManager::setFullscreenOnStart(bool enabled)
{
    setValue("Reader/FullscreenOnStart", enabled);
}

QString ConfigManager::getPageLayout() const
{
    return getValue("Reader/PageLayout", "single").toString();
}

void ConfigManager::setPageLayout(const QString &layout)
{
    setValue("Reader/PageLayout", layout);
}

QString ConfigManager::getScalingMode() const
{
    return getValue("Reader/ScalingMode", "fit_window").toString();
}

void ConfigManager::setScalingMode(const QString &mode)
{
    setValue("Reader/ScalingMode", mode);
}

int ConfigManager::getDefaultZoom() const
{
    return getValue("Reader/DefaultZoom", 100).toInt();
}

void ConfigManager::setDefaultZoom(int zoom)
{
    setValue("Reader/DefaultZoom", zoom);
}

void ConfigManager::saveConfig()
{
    m_settings->sync();
}

void ConfigManager::loadConfig()
{
    // 确保配置目录存在
    QString downloadPath = getDownloadPath();
    QDir().mkpath(downloadPath);
    
    QString cachePath = getCachePath();
    QDir().mkpath(cachePath);
}

void ConfigManager::resetToDefaults()
{
    m_settings->clear();
    setDefaultValues();
    loadConfig();
}

void ConfigManager::setDefaultValues()
{
    // 设置默认值
    setDownloadPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/ComicReader");
    setCachePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    setMaxDownloads(3);
    setMaxDownloadSpeed(0);
    setUserAgent("ComicReader/1.0");
    setTimeout(30);
    setAutoStart(false);
    setTheme("default");
    setLanguage("zh_CN");
    setProxyEnabled(false);
    setProxyHost("");
    setProxyPort(8080);
    setFullscreenOnStart(false);
    setPageLayout("single");
    setScalingMode("fit_window");
    setDefaultZoom(100);
}
