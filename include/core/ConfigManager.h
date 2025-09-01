#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager* instance();
    
    // 设置和获取配置
    void setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    // 常用配置项的便捷方法
    QString getDownloadPath() const;
    void setDownloadPath(const QString &path);
    
    QString getCachePath() const;
    void setCachePath(const QString &path);
    
    int getMaxDownloads() const;
    void setMaxDownloads(int count);
    
    int getMaxDownloadSpeed() const;
    void setMaxDownloadSpeed(int speed);
    
    QString getUserAgent() const;
    void setUserAgent(const QString &userAgent);
    
    int getTimeout() const;
    void setTimeout(int timeout);
    
    bool isAutoStart() const;
    void setAutoStart(bool enabled);
    
    QString getTheme() const;
    void setTheme(const QString &theme);
    
    QString getLanguage() const;
    void setLanguage(const QString &language);
    
    // 代理设置
    bool isProxyEnabled() const;
    void setProxyEnabled(bool enabled);
    
    QString getProxyHost() const;
    void setProxyHost(const QString &host);
    
    int getProxyPort() const;
    void setProxyPort(int port);
    
    // 阅读器设置
    bool isFullscreenOnStart() const;
    void setFullscreenOnStart(bool enabled);
    
    QString getPageLayout() const;
    void setPageLayout(const QString &layout);
    
    QString getScalingMode() const;
    void setScalingMode(const QString &mode);
    
    int getDefaultZoom() const;
    void setDefaultZoom(int zoom);
    
    // 保存和加载配置
    void saveConfig();
    void loadConfig();
    void resetToDefaults();

signals:
    void configChanged(const QString &key, const QVariant &value);

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();
    
    void setDefaultValues();

    static ConfigManager *m_instance;
    QSettings *m_settings;
};

#endif // CONFIGMANAGER_H
