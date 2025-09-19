#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QSize>
#include <QPoint>
#include <QString>
#include <QStringList>

/**
 * @brief 配置管理器类
 * 负责管理应用程序的所有配置信息，包括窗口设置、阅读偏好、缓存设置等
 */
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return ConfigManager实例
     */
    static ConfigManager* instance();

    /**
     * @brief 析构函数
     */
    ~ConfigManager();

    // 窗口设置
    QSize getWindowSize() const;
    void setWindowSize(const QSize &size);
    
    QPoint getWindowPosition() const;
    void setWindowPosition(const QPoint &position);
    
    bool isMaximized() const;
    void setMaximized(bool maximized);
    
    bool isFullscreen() const;
    void setFullscreen(bool fullscreen);

    // 阅读设置
    int getPageFitMode() const;
    void setPageFitMode(int mode);
    
    double getZoomLevel() const;
    void setZoomLevel(double level);
    
    int getReadingDirection() const;
    void setReadingDirection(int direction);
    
    bool isDoublePageMode() const;
    void setDoublePageMode(bool enabled);

    // 应用设置
    QString getLanguage() const;
    void setLanguage(const QString &language);
    
    QString getTheme() const;
    void setTheme(const QString &theme);
    
    QStringList getRecentFiles() const;
    void addRecentFile(const QString &filePath);
    void removeRecentFile(const QString &filePath);
    void clearRecentFiles();

    // 缓存设置
    QString getCacheDirectory() const;
    void setCacheDirectory(const QString &path);
    
    int getMaxCacheSize() const;
    void setMaxCacheSize(int sizeInMB);
    
    bool isPreloadEnabled() const;
    void setPreloadEnabled(bool enabled);
    
    int getPreloadCount() const;
    void setPreloadCount(int count);

    // 通用方法
    QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);
    
    void resetToDefaults();
    void sync();

signals:
    void configChanged(const QString &key, const QVariant &value);
    void themeChanged(const QString &theme);
    void languageChanged(const QString &language);

private:
    explicit ConfigManager(QObject *parent = nullptr);
    
    void initDefaults();
    void loadSettings();
    void saveSettings();

    static ConfigManager *m_instance;
    QSettings *m_settings;
    
    // 默认值
    static const QSize DEFAULT_WINDOW_SIZE;
    static const QPoint DEFAULT_WINDOW_POSITION;
    static const int DEFAULT_PAGE_FIT_MODE;
    static const double DEFAULT_ZOOM_LEVEL;
    static const int DEFAULT_READING_DIRECTION;
    static const QString DEFAULT_LANGUAGE;
    static const QString DEFAULT_THEME;
    static const int DEFAULT_MAX_CACHE_SIZE;
    static const int DEFAULT_PRELOAD_COUNT;
    static const int MAX_RECENT_FILES;
};

#endif // CONFIGMANAGER_H