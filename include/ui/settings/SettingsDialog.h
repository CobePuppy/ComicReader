#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSlider>
#include <QPushButton>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QListWidget>
#include <QTreeWidget>
#include <QScrollArea>
#include <QSplitter>

class ConfigManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // 设置页面管理
    void addSettingsPage(const QString &title, QWidget *page);
    void selectPage(const QString &title);
    
    // 设置应用和重置
    void applySettings();
    void resetToDefaults();
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();
    void settingsApplied();
    void settingsReset();

public slots:
    void onApplyClicked();
    void onResetClicked();
    void onOkClicked();
    void onCancelClicked();

private slots:
    void onPageChanged(int index);
    void onSettingChanged();
    void onBrowseClicked();
    void onColorButtonClicked();
    void onFontButtonClicked();
    void onAddDirectoryClicked();
    void onRemoveDirectoryClicked();
    void onTestConnectionClicked();

private:
    void setupUI();
    void setupGeneralPage();
    void setupReadingPage();
    void setupLibraryPage();
    void setupCachePage();
    void setupNetworkPage();
    void setupAppearancePage();
    void setupAdvancedPage();
    void connectSignals();
    
    void updateColorButtons();
    void updateFontLabels();
    void validateSettings();
    bool hasUnsavedChanges() const;
    
    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QTabWidget *m_tabWidget;
    
    // 按钮
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
    
    // 通用设置页面
    QWidget *m_generalPage;
    QCheckBox *m_startWithSystemCheckBox;
    QCheckBox *m_minimizeToTrayCheckBox;
    QCheckBox *m_rememberWindowStateCheckBox;
    QCheckBox *m_checkUpdatesCheckBox;
    QComboBox *m_languageComboBox;
    QComboBox *m_themeComboBox;
    
    // 阅读设置页面
    QWidget *m_readingPage;
    QComboBox *m_defaultZoomModeComboBox;
    QSpinBox *m_defaultZoomSpinBox;
    QCheckBox *m_smoothScrollingCheckBox;
    QCheckBox *m_invertScrollCheckBox;
    QSpinBox *m_scrollSpeedSpinBox;
    QCheckBox *m_fullscreenHideUICheckBox;
    QCheckBox *m_doubleClickFullscreenCheckBox;
    QComboBox *m_pageTurnModeComboBox;
    QCheckBox *m_preloadPagesCheckBox;
    QSpinBox *m_preloadCountSpinBox;
    QComboBox *m_backgroundColorComboBox;
    QPushButton *m_customBackgroundColorButton;
    
    // 图书馆设置页面
    QWidget *m_libraryPage;
    QListWidget *m_scanDirectoriesListWidget;
    QPushButton *m_addDirectoryButton;
    QPushButton *m_removeDirectoryButton;
    QCheckBox *m_autoScanCheckBox;
    QSpinBox *m_scanIntervalSpinBox;
    QCheckBox *m_recursiveScanCheckBox;
    QComboBox *m_defaultViewModeComboBox;
    QComboBox *m_defaultSortModeComboBox;
    QSpinBox *m_thumbnailSizeSpinBox;
    QCheckBox *m_generateThumbnailsCheckBox;
    QCheckBox *m_rememberReadingPositionCheckBox;
    
    // 缓存设置页面
    QWidget *m_cachePage;
    QSpinBox *m_memoryCacheSizeSpinBox;
    QSpinBox *m_diskCacheSizeSpinBox;
    QLineEdit *m_cacheDirectoryLineEdit;
    QPushButton *m_browseCacheDirectoryButton;
    QCheckBox *m_enableDiskCacheCheckBox;
    QSpinBox *m_cacheCleanupIntervalSpinBox;
    QSpinBox *m_cacheMaxAgeSpinBox;
    QPushButton *m_clearCacheButton;
    QLabel *m_cacheUsageLabel;
    
    // 网络设置页面
    QWidget *m_networkPage;
    QCheckBox *m_useProxyCheckBox;
    QComboBox *m_proxyTypeComboBox;
    QLineEdit *m_proxyHostLineEdit;
    QSpinBox *m_proxyPortSpinBox;
    QLineEdit *m_proxyUsernameLineEdit;
    QLineEdit *m_proxyPasswordLineEdit;
    QPushButton *m_testConnectionButton;
    QSpinBox *m_connectionTimeoutSpinBox;
    QSpinBox *m_downloadTimeoutSpinBox;
    QSpinBox *m_maxConcurrentDownloadsSpinBox;
    QSpinBox *m_retryCountSpinBox;
    QSpinBox *m_speedLimitSpinBox;
    QCheckBox *m_enableSpeedLimitCheckBox;
    
    // 外观设置页面
    QWidget *m_appearancePage;
    QComboBox *m_styleComboBox;
    QCheckBox *m_darkModeCheckBox;
    QSlider *m_opacitySlider;
    QLabel *m_opacityLabel;
    QComboBox *m_fontFamilyComboBox;
    QSpinBox *m_fontSizeSpinBox;
    QPushButton *m_selectFontButton;
    QLabel *m_fontPreviewLabel;
    QPushButton *m_primaryColorButton;
    QPushButton *m_secondaryColorButton;
    QPushButton *m_accentColorButton;
    QCheckBox *m_customColorsCheckBox;
    
    // 高级设置页面
    QWidget *m_advancedPage;
    QCheckBox *m_enableLoggingCheckBox;
    QComboBox *m_logLevelComboBox;
    QLineEdit *m_logDirectoryLineEdit;
    QPushButton *m_browseLogDirectoryButton;
    QSpinBox *m_maxLogFilesSpinBox;
    QSpinBox *m_maxLogFileSizeSpinBox;
    QCheckBox *m_enableDebugModeCheckBox;
    QCheckBox *m_enablePerformanceMonitoringCheckBox;
    QSpinBox *m_maxUndoStepsSpinBox;
    QCheckBox *m_autoSaveSettingsCheckBox;
    QSpinBox *m_autoSaveIntervalSpinBox;
    QPushButton *m_exportSettingsButton;
    QPushButton *m_importSettingsButton;
    QPushButton *m_resetAllSettingsButton;
    
    // 数据
    ConfigManager *m_configManager;
    bool m_hasUnsavedChanges;
    QColor m_primaryColor;
    QColor m_secondaryColor;
    QColor m_accentColor;
    QColor m_customBackgroundColor;
    QFont m_selectedFont;
    
    // 辅助组件
    QButtonGroup *m_colorButtonGroup;
};

#endif // SETTINGSDIALOG_H