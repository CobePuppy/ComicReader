#include "../../../include/ui/settings/SettingsDialog.h"
#include "../../../include/core/ConfigManager.h"
#include "../../../include/core/cache/CacheManager.h"
#include "../../../include/network/DownloadManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkProxy>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QPalette>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_tabWidget(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_applyButton(nullptr)
    , m_resetButton(nullptr)
    , m_configManager(ConfigManager::instance())
    , m_hasUnsavedChanges(false)
    , m_primaryColor(Qt::blue)
    , m_secondaryColor(Qt::lightGray)
    , m_accentColor(Qt::darkBlue)
    , m_customBackgroundColor(Qt::white)
    , m_colorButtonGroup(nullptr)
{
    setWindowTitle("设置");
    setWindowIcon(QIcon(":/icons/toolbar/settings"));
    setModal(true);
    resize(800, 600);
    
    setupUI();
    connectSignals();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    if (m_colorButtonGroup) {
        delete m_colorButtonGroup;
    }
}

void SettingsDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget();
    
    // 设置各个页面
    setupGeneralPage();
    setupReadingPage();
    setupLibraryPage();
    setupCachePage();
    setupNetworkPage();
    setupAppearancePage();
    setupAdvancedPage();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // 创建按钮
    m_buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("重置为默认值");
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addStretch();
    
    m_okButton = new QPushButton("确定");
    m_okButton->setDefault(true);
    m_cancelButton = new QPushButton("取消");
    m_applyButton = new QPushButton("应用");
    m_applyButton->setEnabled(false);
    
    m_buttonLayout->addWidget(m_okButton);
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_applyButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void SettingsDialog::setupGeneralPage()
{
    m_generalPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_generalPage);
    
    // 启动设置组
    QGroupBox *startupGroup = new QGroupBox("启动设置");
    QVBoxLayout *startupLayout = new QVBoxLayout(startupGroup);
    
    m_startWithSystemCheckBox = new QCheckBox("随系统启动");
    m_startWithSystemCheckBox->setToolTip("在系统启动时自动启动ComicReader");
    startupLayout->addWidget(m_startWithSystemCheckBox);
    
    m_minimizeToTrayCheckBox = new QCheckBox("最小化到系统托盘");
    m_minimizeToTrayCheckBox->setToolTip("关闭窗口时最小化到系统托盘而不是退出程序");
    startupLayout->addWidget(m_minimizeToTrayCheckBox);
    
    m_rememberWindowStateCheckBox = new QCheckBox("记住窗口位置和大小");
    m_rememberWindowStateCheckBox->setToolTip("在下次启动时恢复窗口的位置和大小");
    startupLayout->addWidget(m_rememberWindowStateCheckBox);
    
    layout->addWidget(startupGroup);
    
    // 更新设置组
    QGroupBox *updateGroup = new QGroupBox("更新设置");
    QVBoxLayout *updateLayout = new QVBoxLayout(updateGroup);
    
    m_checkUpdatesCheckBox = new QCheckBox("自动检查更新");
    m_checkUpdatesCheckBox->setToolTip("定期检查是否有新版本可用");
    updateLayout->addWidget(m_checkUpdatesCheckBox);
    
    layout->addWidget(updateGroup);
    
    // 语言和主题设置组
    QGroupBox *localizationGroup = new QGroupBox("语言和主题");
    QFormLayout *localizationLayout = new QFormLayout(localizationGroup);
    
    m_languageComboBox = new QComboBox();
    m_languageComboBox->addItem("简体中文", "zh_CN");
    m_languageComboBox->addItem("English", "en_US");
    m_languageComboBox->addItem("日本語", "ja_JP");
    m_languageComboBox->setToolTip("选择界面语言");
    localizationLayout->addRow("界面语言:", m_languageComboBox);
    
    m_themeComboBox = new QComboBox();
    m_themeComboBox->addItem("系统默认", "system");
    m_themeComboBox->addItem("浅色主题", "light");
    m_themeComboBox->addItem("深色主题", "dark");
    m_themeComboBox->addItem("自定义", "custom");
    m_themeComboBox->setToolTip("选择应用程序主题");
    localizationLayout->addRow("主题:", m_themeComboBox);
    
    layout->addWidget(localizationGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_generalPage, "通用");
}

void SettingsDialog::setupReadingPage()
{
    m_readingPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_readingPage);
    
    // 缩放设置组
    QGroupBox *zoomGroup = new QGroupBox("缩放设置");
    QFormLayout *zoomLayout = new QFormLayout(zoomGroup);
    
    m_defaultZoomModeComboBox = new QComboBox();
    m_defaultZoomModeComboBox->addItem("适合窗口", 0);
    m_defaultZoomModeComboBox->addItem("适合宽度", 1);
    m_defaultZoomModeComboBox->addItem("适合高度", 2);
    m_defaultZoomModeComboBox->addItem("实际大小", 3);
    m_defaultZoomModeComboBox->addItem("自定义", 4);
    zoomLayout->addRow("默认缩放模式:", m_defaultZoomModeComboBox);
    
    m_defaultZoomSpinBox = new QSpinBox();
    m_defaultZoomSpinBox->setRange(10, 500);
    m_defaultZoomSpinBox->setSuffix("%");
    m_defaultZoomSpinBox->setValue(100);
    m_defaultZoomSpinBox->setEnabled(false);
    zoomLayout->addRow("自定义缩放比例:", m_defaultZoomSpinBox);
    
    layout->addWidget(zoomGroup);
    
    // 滚动设置组
    QGroupBox *scrollGroup = new QGroupBox("滚动设置");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollGroup);
    
    m_smoothScrollingCheckBox = new QCheckBox("启用平滑滚动");
    m_smoothScrollingCheckBox->setToolTip("使用平滑动画进行滚动");
    scrollLayout->addWidget(m_smoothScrollingCheckBox);
    
    m_invertScrollCheckBox = new QCheckBox("反转鼠标滚轮方向");
    m_invertScrollCheckBox->setToolTip("反转鼠标滚轮的滚动方向");
    scrollLayout->addWidget(m_invertScrollCheckBox);
    
    QHBoxLayout *scrollSpeedLayout = new QHBoxLayout();
    scrollSpeedLayout->addWidget(new QLabel("滚动速度:"));
    m_scrollSpeedSpinBox = new QSpinBox();
    m_scrollSpeedSpinBox->setRange(1, 10);
    m_scrollSpeedSpinBox->setValue(3);
    scrollSpeedLayout->addWidget(m_scrollSpeedSpinBox);
    scrollSpeedLayout->addStretch();
    scrollLayout->addLayout(scrollSpeedLayout);
    
    layout->addWidget(scrollGroup);
    
    // 全屏设置组
    QGroupBox *fullscreenGroup = new QGroupBox("全屏设置");
    QVBoxLayout *fullscreenLayout = new QVBoxLayout(fullscreenGroup);
    
    m_fullscreenHideUICheckBox = new QCheckBox("全屏时隐藏界面元素");
    m_fullscreenHideUICheckBox->setToolTip("在全屏模式下隐藏工具栏和菜单栏");
    fullscreenLayout->addWidget(m_fullscreenHideUICheckBox);
    
    m_doubleClickFullscreenCheckBox = new QCheckBox("双击进入/退出全屏");
    m_doubleClickFullscreenCheckBox->setToolTip("双击图像区域切换全屏模式");
    fullscreenLayout->addWidget(m_doubleClickFullscreenCheckBox);
    
    layout->addWidget(fullscreenGroup);
    
    // 翻页设置组
    QGroupBox *pageGroup = new QGroupBox("翻页设置");
    QFormLayout *pageLayout = new QFormLayout(pageGroup);
    
    m_pageTurnModeComboBox = new QComboBox();
    m_pageTurnModeComboBox->addItem("点击翻页", 0);
    m_pageTurnModeComboBox->addItem("键盘翻页", 1);
    m_pageTurnModeComboBox->addItem("鼠标拖拽翻页", 2);
    m_pageTurnModeComboBox->addItem("自动翻页", 3);
    pageLayout->addRow("翻页模式:", m_pageTurnModeComboBox);
    
    layout->addWidget(pageGroup);
    
    // 预加载设置组
    QGroupBox *preloadGroup = new QGroupBox("预加载设置");
    QVBoxLayout *preloadLayout = new QVBoxLayout(preloadGroup);
    
    m_preloadPagesCheckBox = new QCheckBox("启用页面预加载");
    m_preloadPagesCheckBox->setToolTip("提前加载下一页以提高阅读体验");
    preloadLayout->addWidget(m_preloadPagesCheckBox);
    
    QHBoxLayout *preloadCountLayout = new QHBoxLayout();
    preloadCountLayout->addWidget(new QLabel("预加载页数:"));
    m_preloadCountSpinBox = new QSpinBox();
    m_preloadCountSpinBox->setRange(1, 10);
    m_preloadCountSpinBox->setValue(2);
    m_preloadCountSpinBox->setEnabled(false);
    preloadCountLayout->addWidget(m_preloadCountSpinBox);
    preloadCountLayout->addStretch();
    preloadLayout->addLayout(preloadCountLayout);
    
    layout->addWidget(preloadGroup);
    
    // 背景设置组
    QGroupBox *backgroundGroup = new QGroupBox("背景设置");
    QFormLayout *backgroundLayout = new QFormLayout(backgroundGroup);
    
    m_backgroundColorComboBox = new QComboBox();
    m_backgroundColorComboBox->addItem("白色", "white");
    m_backgroundColorComboBox->addItem("黑色", "black");
    m_backgroundColorComboBox->addItem("灰色", "gray");
    m_backgroundColorComboBox->addItem("自定义", "custom");
    backgroundLayout->addRow("背景颜色:", m_backgroundColorComboBox);
    
    m_customBackgroundColorButton = new QPushButton("选择颜色");
    m_customBackgroundColorButton->setEnabled(false);
    backgroundLayout->addRow("自定义颜色:", m_customBackgroundColorButton);
    
    layout->addWidget(backgroundGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_readingPage, "阅读");
}

void SettingsDialog::setupLibraryPage()
{
    m_libraryPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_libraryPage);
    
    // 扫描目录设置组
    QGroupBox *scanGroup = new QGroupBox("扫描目录");
    QVBoxLayout *scanLayout = new QVBoxLayout(scanGroup);
    
    QHBoxLayout *dirListLayout = new QHBoxLayout();
    m_scanDirectoriesListWidget = new QListWidget();
    m_scanDirectoriesListWidget->setMinimumHeight(120);
    dirListLayout->addWidget(m_scanDirectoriesListWidget);
    
    QVBoxLayout *dirButtonLayout = new QVBoxLayout();
    m_addDirectoryButton = new QPushButton("添加目录");
    m_removeDirectoryButton = new QPushButton("移除目录");
    m_removeDirectoryButton->setEnabled(false);
    dirButtonLayout->addWidget(m_addDirectoryButton);
    dirButtonLayout->addWidget(m_removeDirectoryButton);
    dirButtonLayout->addStretch();
    dirListLayout->addLayout(dirButtonLayout);
    
    scanLayout->addLayout(dirListLayout);
    
    m_autoScanCheckBox = new QCheckBox("自动扫描新文件");
    m_autoScanCheckBox->setToolTip("监控指定目录的文件变化并自动添加新漫画");
    scanLayout->addWidget(m_autoScanCheckBox);
    
    QHBoxLayout *scanIntervalLayout = new QHBoxLayout();
    scanIntervalLayout->addWidget(new QLabel("扫描间隔:"));
    m_scanIntervalSpinBox = new QSpinBox();
    m_scanIntervalSpinBox->setRange(1, 60);
    m_scanIntervalSpinBox->setSuffix(" 分钟");
    m_scanIntervalSpinBox->setValue(5);
    m_scanIntervalSpinBox->setEnabled(false);
    scanIntervalLayout->addWidget(m_scanIntervalSpinBox);
    scanIntervalLayout->addStretch();
    scanLayout->addLayout(scanIntervalLayout);
    
    m_recursiveScanCheckBox = new QCheckBox("递归扫描子目录");
    m_recursiveScanCheckBox->setToolTip("扫描指定目录下的所有子目录");
    scanLayout->addWidget(m_recursiveScanCheckBox);
    
    layout->addWidget(scanGroup);
    
    // 显示设置组
    QGroupBox *displayGroup = new QGroupBox("显示设置");
    QFormLayout *displayLayout = new QFormLayout(displayGroup);
    
    m_defaultViewModeComboBox = new QComboBox();
    m_defaultViewModeComboBox->addItem("列表视图", 0);
    m_defaultViewModeComboBox->addItem("网格视图", 1);
    m_defaultViewModeComboBox->addItem("详细视图", 2);
    displayLayout->addRow("默认视图模式:", m_defaultViewModeComboBox);
    
    m_defaultSortModeComboBox = new QComboBox();
    m_defaultSortModeComboBox->addItem("按名称", 0);
    m_defaultSortModeComboBox->addItem("按添加时间", 1);
    m_defaultSortModeComboBox->addItem("按最近阅读", 2);
    m_defaultSortModeComboBox->addItem("按文件大小", 3);
    m_defaultSortModeComboBox->addItem("按评分", 4);
    displayLayout->addRow("默认排序方式:", m_defaultSortModeComboBox);
    
    m_thumbnailSizeSpinBox = new QSpinBox();
    m_thumbnailSizeSpinBox->setRange(64, 512);
    m_thumbnailSizeSpinBox->setSuffix(" 像素");
    m_thumbnailSizeSpinBox->setValue(128);
    displayLayout->addRow("缩略图大小:", m_thumbnailSizeSpinBox);
    
    layout->addWidget(displayGroup);
    
    // 缩略图设置组
    QGroupBox *thumbnailGroup = new QGroupBox("缩略图设置");
    QVBoxLayout *thumbnailLayout = new QVBoxLayout(thumbnailGroup);
    
    m_generateThumbnailsCheckBox = new QCheckBox("自动生成缩略图");
    m_generateThumbnailsCheckBox->setToolTip("为新添加的漫画自动生成缩略图");
    thumbnailLayout->addWidget(m_generateThumbnailsCheckBox);
    
    layout->addWidget(thumbnailGroup);
    
    // 阅读记录设置组
    QGroupBox *historyGroup = new QGroupBox("阅读记录");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    
    m_rememberReadingPositionCheckBox = new QCheckBox("记住阅读位置");
    m_rememberReadingPositionCheckBox->setToolTip("自动保存每本漫画的阅读进度");
    historyLayout->addWidget(m_rememberReadingPositionCheckBox);
    
    layout->addWidget(historyGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_libraryPage, "图书馆");
}

void SettingsDialog::setupCachePage()
{
    m_cachePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_cachePage);
    
    // 内存缓存设置组
    QGroupBox *memoryCacheGroup = new QGroupBox("内存缓存");
    QFormLayout *memoryCacheLayout = new QFormLayout(memoryCacheGroup);
    
    m_memoryCacheSizeSpinBox = new QSpinBox();
    m_memoryCacheSizeSpinBox->setRange(16, 2048);
    m_memoryCacheSizeSpinBox->setSuffix(" MB");
    m_memoryCacheSizeSpinBox->setValue(100);
    m_memoryCacheSizeSpinBox->setToolTip("用于缓存图片的内存大小");
    memoryCacheLayout->addRow("内存缓存大小:", m_memoryCacheSizeSpinBox);
    
    layout->addWidget(memoryCacheGroup);
    
    // 磁盘缓存设置组
    QGroupBox *diskCacheGroup = new QGroupBox("磁盘缓存");
    QVBoxLayout *diskCacheLayout = new QVBoxLayout(diskCacheGroup);
    
    m_enableDiskCacheCheckBox = new QCheckBox("启用磁盘缓存");
    m_enableDiskCacheCheckBox->setToolTip("将处理过的图片保存到磁盘以提高加载速度");
    diskCacheLayout->addWidget(m_enableDiskCacheCheckBox);
    
    QFormLayout *diskFormLayout = new QFormLayout();
    
    m_diskCacheSizeSpinBox = new QSpinBox();
    m_diskCacheSizeSpinBox->setRange(100, 10240);
    m_diskCacheSizeSpinBox->setSuffix(" MB");
    m_diskCacheSizeSpinBox->setValue(500);
    m_diskCacheSizeSpinBox->setEnabled(false);
    diskFormLayout->addRow("磁盘缓存大小:", m_diskCacheSizeSpinBox);
    
    QHBoxLayout *cacheDirLayout = new QHBoxLayout();
    m_cacheDirectoryLineEdit = new QLineEdit();
    m_cacheDirectoryLineEdit->setReadOnly(true);
    m_cacheDirectoryLineEdit->setEnabled(false);
    m_browseCacheDirectoryButton = new QPushButton("浏览");
    m_browseCacheDirectoryButton->setEnabled(false);
    cacheDirLayout->addWidget(m_cacheDirectoryLineEdit);
    cacheDirLayout->addWidget(m_browseCacheDirectoryButton);
    diskFormLayout->addRow("缓存目录:", cacheDirLayout);
    
    diskCacheLayout->addLayout(diskFormLayout);
    layout->addWidget(diskCacheGroup);
    
    // 缓存清理设置组
    QGroupBox *cleanupGroup = new QGroupBox("缓存清理");
    QFormLayout *cleanupLayout = new QFormLayout(cleanupGroup);
    
    m_cacheCleanupIntervalSpinBox = new QSpinBox();
    m_cacheCleanupIntervalSpinBox->setRange(5, 60);
    m_cacheCleanupIntervalSpinBox->setSuffix(" 分钟");
    m_cacheCleanupIntervalSpinBox->setValue(30);
    m_cacheCleanupIntervalSpinBox->setToolTip("自动清理过期缓存的时间间隔");
    cleanupLayout->addRow("清理间隔:", m_cacheCleanupIntervalSpinBox);
    
    m_cacheMaxAgeSpinBox = new QSpinBox();
    m_cacheMaxAgeSpinBox->setRange(1, 30);
    m_cacheMaxAgeSpinBox->setSuffix(" 天");
    m_cacheMaxAgeSpinBox->setValue(7);
    m_cacheMaxAgeSpinBox->setToolTip("缓存文件的最大保存时间");
    cleanupLayout->addRow("缓存保留时间:", m_cacheMaxAgeSpinBox);
    
    layout->addWidget(cleanupGroup);
    
    // 缓存状态和操作
    QGroupBox *statusGroup = new QGroupBox("缓存状态");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    m_cacheUsageLabel = new QLabel("正在计算缓存使用情况...");
    statusLayout->addWidget(m_cacheUsageLabel);
    
    m_clearCacheButton = new QPushButton("清空所有缓存");
    m_clearCacheButton->setToolTip("删除所有缓存文件以释放磁盘空间");
    statusLayout->addWidget(m_clearCacheButton);
    
    layout->addWidget(statusGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_cachePage, "缓存");
}

void SettingsDialog::setupNetworkPage()
{
    m_networkPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_networkPage);
    
    // 代理设置组
    QGroupBox *proxyGroup = new QGroupBox("代理设置");
    QVBoxLayout *proxyLayout = new QVBoxLayout(proxyGroup);
    
    m_useProxyCheckBox = new QCheckBox("使用代理服务器");
    m_useProxyCheckBox->setToolTip("通过代理服务器进行网络连接");
    proxyLayout->addWidget(m_useProxyCheckBox);
    
    QFormLayout *proxyFormLayout = new QFormLayout();
    
    m_proxyTypeComboBox = new QComboBox();
    m_proxyTypeComboBox->addItem("HTTP", QNetworkProxy::HttpProxy);
    m_proxyTypeComboBox->addItem("SOCKS5", QNetworkProxy::Socks5Proxy);
    m_proxyTypeComboBox->setEnabled(false);
    proxyFormLayout->addRow("代理类型:", m_proxyTypeComboBox);
    
    m_proxyHostLineEdit = new QLineEdit();
    m_proxyHostLineEdit->setPlaceholderText("例如: proxy.example.com");
    m_proxyHostLineEdit->setEnabled(false);
    proxyFormLayout->addRow("代理主机:", m_proxyHostLineEdit);
    
    m_proxyPortSpinBox = new QSpinBox();
    m_proxyPortSpinBox->setRange(1, 65535);
    m_proxyPortSpinBox->setValue(8080);
    m_proxyPortSpinBox->setEnabled(false);
    proxyFormLayout->addRow("端口:", m_proxyPortSpinBox);
    
    m_proxyUsernameLineEdit = new QLineEdit();
    m_proxyUsernameLineEdit->setPlaceholderText("用户名（可选）");
    m_proxyUsernameLineEdit->setEnabled(false);
    proxyFormLayout->addRow("用户名:", m_proxyUsernameLineEdit);
    
    m_proxyPasswordLineEdit = new QLineEdit();
    m_proxyPasswordLineEdit->setEchoMode(QLineEdit::Password);
    m_proxyPasswordLineEdit->setPlaceholderText("密码（可选）");
    m_proxyPasswordLineEdit->setEnabled(false);
    proxyFormLayout->addRow("密码:", m_proxyPasswordLineEdit);
    
    m_testConnectionButton = new QPushButton("测试连接");
    m_testConnectionButton->setEnabled(false);
    proxyFormLayout->addRow("", m_testConnectionButton);
    
    proxyLayout->addLayout(proxyFormLayout);
    layout->addWidget(proxyGroup);
    
    // 连接设置组
    QGroupBox *connectionGroup = new QGroupBox("连接设置");
    QFormLayout *connectionLayout = new QFormLayout(connectionGroup);
    
    m_connectionTimeoutSpinBox = new QSpinBox();
    m_connectionTimeoutSpinBox->setRange(5, 120);
    m_connectionTimeoutSpinBox->setSuffix(" 秒");
    m_connectionTimeoutSpinBox->setValue(30);
    m_connectionTimeoutSpinBox->setToolTip("建立网络连接的超时时间");
    connectionLayout->addRow("连接超时:", m_connectionTimeoutSpinBox);
    
    m_downloadTimeoutSpinBox = new QSpinBox();
    m_downloadTimeoutSpinBox->setRange(10, 300);
    m_downloadTimeoutSpinBox->setSuffix(" 秒");
    m_downloadTimeoutSpinBox->setValue(60);
    m_downloadTimeoutSpinBox->setToolTip("下载文件的超时时间");
    connectionLayout->addRow("下载超时:", m_downloadTimeoutSpinBox);
    
    layout->addWidget(connectionGroup);
    
    // 下载设置组
    QGroupBox *downloadGroup = new QGroupBox("下载设置");
    QFormLayout *downloadLayout = new QFormLayout(downloadGroup);
    
    m_maxConcurrentDownloadsSpinBox = new QSpinBox();
    m_maxConcurrentDownloadsSpinBox->setRange(1, 10);
    m_maxConcurrentDownloadsSpinBox->setValue(3);
    m_maxConcurrentDownloadsSpinBox->setToolTip("同时进行的最大下载数");
    downloadLayout->addRow("最大并发下载:", m_maxConcurrentDownloadsSpinBox);
    
    m_retryCountSpinBox = new QSpinBox();
    m_retryCountSpinBox->setRange(0, 10);
    m_retryCountSpinBox->setValue(3);
    m_retryCountSpinBox->setToolTip("下载失败时的重试次数");
    downloadLayout->addRow("重试次数:", m_retryCountSpinBox);
    
    QHBoxLayout *speedLimitLayout = new QHBoxLayout();
    m_enableSpeedLimitCheckBox = new QCheckBox("限制下载速度");
    speedLimitLayout->addWidget(m_enableSpeedLimitCheckBox);
    
    m_speedLimitSpinBox = new QSpinBox();
    m_speedLimitSpinBox->setRange(1, 10240);
    m_speedLimitSpinBox->setSuffix(" KB/s");
    m_speedLimitSpinBox->setValue(1024);
    m_speedLimitSpinBox->setEnabled(false);
    speedLimitLayout->addWidget(m_speedLimitSpinBox);
    speedLimitLayout->addStretch();
    
    downloadLayout->addRow("速度限制:", speedLimitLayout);
    
    layout->addWidget(downloadGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_networkPage, "网络");
}

void SettingsDialog::setupAppearancePage()
{
    m_appearancePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_appearancePage);
    
    // 样式设置组
    QGroupBox *styleGroup = new QGroupBox("样式设置");
    QFormLayout *styleLayout = new QFormLayout(styleGroup);
    
    m_styleComboBox = new QComboBox();
    QStringList styles = QStyleFactory::keys();
    m_styleComboBox->addItems(styles);
    m_styleComboBox->setToolTip("选择界面样式");
    styleLayout->addRow("界面样式:", m_styleComboBox);
    
    m_darkModeCheckBox = new QCheckBox("启用深色模式");
    m_darkModeCheckBox->setToolTip("使用深色主题界面");
    styleLayout->addRow("", m_darkModeCheckBox);
    
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(50, 100);
    m_opacitySlider->setValue(100);
    m_opacityLabel = new QLabel("100%");
    m_opacityLabel->setMinimumWidth(50);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(m_opacityLabel);
    styleLayout->addRow("窗口透明度:", opacityLayout);
    
    layout->addWidget(styleGroup);
    
    // 字体设置组
    QGroupBox *fontGroup = new QGroupBox("字体设置");
    QFormLayout *fontLayout = new QFormLayout(fontGroup);
    
    m_fontFamilyComboBox = new QComboBox();
    QFontDatabase fontDB;
    m_fontFamilyComboBox->addItems(fontDB.families());
    m_fontFamilyComboBox->setCurrentText(QApplication::font().family());
    fontLayout->addRow("字体族:", m_fontFamilyComboBox);
    
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(QApplication::font().pointSize());
    fontLayout->addRow("字体大小:", m_fontSizeSpinBox);
    
    m_selectFontButton = new QPushButton("选择字体");
    fontLayout->addRow("", m_selectFontButton);
    
    m_fontPreviewLabel = new QLabel("字体预览 Font Preview フォントプレビュー");
    m_fontPreviewLabel->setFrameStyle(QFrame::Box);
    m_fontPreviewLabel->setAlignment(Qt::AlignCenter);
    m_fontPreviewLabel->setMinimumHeight(50);
    fontLayout->addRow("预览:", m_fontPreviewLabel);
    
    layout->addWidget(fontGroup);
    
    // 颜色设置组
    QGroupBox *colorGroup = new QGroupBox("颜色设置");
    QVBoxLayout *colorLayout = new QVBoxLayout(colorGroup);
    
    m_customColorsCheckBox = new QCheckBox("使用自定义颜色");
    m_customColorsCheckBox->setToolTip("启用自定义界面颜色方案");
    colorLayout->addWidget(m_customColorsCheckBox);
    
    QFormLayout *colorFormLayout = new QFormLayout();
    
    m_primaryColorButton = new QPushButton();
    m_primaryColorButton->setFixedSize(50, 30);
    m_primaryColorButton->setEnabled(false);
    colorFormLayout->addRow("主色调:", m_primaryColorButton);
    
    m_secondaryColorButton = new QPushButton();
    m_secondaryColorButton->setFixedSize(50, 30);
    m_secondaryColorButton->setEnabled(false);
    colorFormLayout->addRow("辅助色:", m_secondaryColorButton);
    
    m_accentColorButton = new QPushButton();
    m_accentColorButton->setFixedSize(50, 30);
    m_accentColorButton->setEnabled(false);
    colorFormLayout->addRow("强调色:", m_accentColorButton);
    
    colorLayout->addLayout(colorFormLayout);
    layout->addWidget(colorGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_appearancePage, "外观");
}

void SettingsDialog::setupAdvancedPage()
{
    m_advancedPage = new QWidget();
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    
    // 日志设置组
    QGroupBox *loggingGroup = new QGroupBox("日志设置");
    QVBoxLayout *loggingLayout = new QVBoxLayout(loggingGroup);
    
    m_enableLoggingCheckBox = new QCheckBox("启用日志记录");
    m_enableLoggingCheckBox->setToolTip("记录程序运行日志用于调试");
    loggingLayout->addWidget(m_enableLoggingCheckBox);
    
    QFormLayout *logFormLayout = new QFormLayout();
    
    m_logLevelComboBox = new QComboBox();
    m_logLevelComboBox->addItem("调试", "debug");
    m_logLevelComboBox->addItem("信息", "info");
    m_logLevelComboBox->addItem("警告", "warning");
    m_logLevelComboBox->addItem("错误", "error");
    m_logLevelComboBox->setCurrentIndex(1);
    m_logLevelComboBox->setEnabled(false);
    logFormLayout->addRow("日志级别:", m_logLevelComboBox);
    
    QHBoxLayout *logDirLayout = new QHBoxLayout();
    m_logDirectoryLineEdit = new QLineEdit();
    m_logDirectoryLineEdit->setReadOnly(true);
    m_logDirectoryLineEdit->setEnabled(false);
    m_browseLogDirectoryButton = new QPushButton("浏览");
    m_browseLogDirectoryButton->setEnabled(false);
    logDirLayout->addWidget(m_logDirectoryLineEdit);
    logDirLayout->addWidget(m_browseLogDirectoryButton);
    logFormLayout->addRow("日志目录:", logDirLayout);
    
    m_maxLogFilesSpinBox = new QSpinBox();
    m_maxLogFilesSpinBox->setRange(1, 100);
    m_maxLogFilesSpinBox->setValue(10);
    m_maxLogFilesSpinBox->setEnabled(false);
    logFormLayout->addRow("最大日志文件数:", m_maxLogFilesSpinBox);
    
    m_maxLogFileSizeSpinBox = new QSpinBox();
    m_maxLogFileSizeSpinBox->setRange(1, 100);
    m_maxLogFileSizeSpinBox->setSuffix(" MB");
    m_maxLogFileSizeSpinBox->setValue(10);
    m_maxLogFileSizeSpinBox->setEnabled(false);
    logFormLayout->addRow("最大日志文件大小:", m_maxLogFileSizeSpinBox);
    
    loggingLayout->addLayout(logFormLayout);
    layout->addWidget(loggingGroup);
    
    // 调试设置组
    QGroupBox *debugGroup = new QGroupBox("调试设置");
    QVBoxLayout *debugLayout = new QVBoxLayout(debugGroup);
    
    m_enableDebugModeCheckBox = new QCheckBox("启用调试模式");
    m_enableDebugModeCheckBox->setToolTip("显示详细的调试信息");
    debugLayout->addWidget(m_enableDebugModeCheckBox);
    
    m_enablePerformanceMonitoringCheckBox = new QCheckBox("启用性能监控");
    m_enablePerformanceMonitoringCheckBox->setToolTip("监控程序性能指标");
    debugLayout->addWidget(m_enablePerformanceMonitoringCheckBox);
    
    layout->addWidget(debugGroup);
    
    // 其他设置组
    QGroupBox *miscGroup = new QGroupBox("其他设置");
    QFormLayout *miscLayout = new QFormLayout(miscGroup);
    
    m_maxUndoStepsSpinBox = new QSpinBox();
    m_maxUndoStepsSpinBox->setRange(1, 100);
    m_maxUndoStepsSpinBox->setValue(20);
    m_maxUndoStepsSpinBox->setToolTip("撤销操作的最大步数");
    miscLayout->addRow("最大撤销步数:", m_maxUndoStepsSpinBox);
    
    QHBoxLayout *autoSaveLayout = new QHBoxLayout();
    m_autoSaveSettingsCheckBox = new QCheckBox("自动保存设置");
    autoSaveLayout->addWidget(m_autoSaveSettingsCheckBox);
    
    m_autoSaveIntervalSpinBox = new QSpinBox();
    m_autoSaveIntervalSpinBox->setRange(1, 60);
    m_autoSaveIntervalSpinBox->setSuffix(" 分钟");
    m_autoSaveIntervalSpinBox->setValue(5);
    m_autoSaveIntervalSpinBox->setEnabled(false);
    autoSaveLayout->addWidget(m_autoSaveIntervalSpinBox);
    autoSaveLayout->addStretch();
    miscLayout->addRow("自动保存:", autoSaveLayout);
    
    layout->addWidget(miscGroup);
    
    // 导入导出设置组
    QGroupBox *importExportGroup = new QGroupBox("导入/导出设置");
    QHBoxLayout *importExportLayout = new QHBoxLayout(importExportGroup);
    
    m_exportSettingsButton = new QPushButton("导出设置");
    m_exportSettingsButton->setToolTip("将当前设置导出到文件");
    importExportLayout->addWidget(m_exportSettingsButton);
    
    m_importSettingsButton = new QPushButton("导入设置");
    m_importSettingsButton->setToolTip("从文件导入设置");
    importExportLayout->addWidget(m_importSettingsButton);
    
    importExportLayout->addStretch();
    
    m_resetAllSettingsButton = new QPushButton("重置所有设置");
    m_resetAllSettingsButton->setToolTip("将所有设置恢复为默认值");
    m_resetAllSettingsButton->setStyleSheet("QPushButton { color: red; }");
    importExportLayout->addWidget(m_resetAllSettingsButton);
    
    layout->addWidget(importExportGroup);
    
    layout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    
    QVBoxLayout *advancedLayout = new QVBoxLayout(m_advancedPage);
    advancedLayout->addWidget(scrollArea);
    
    m_tabWidget->addTab(m_advancedPage, "高级");
}

void SettingsDialog::connectSignals()
{
    // 按钮信号
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    
    // 标签页变化
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &SettingsDialog::onPageChanged);
    
    // 通用页面信号
    connect(m_startWithSystemCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_minimizeToTrayCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_rememberWindowStateCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_checkUpdatesCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    
    // 阅读页面信号
    connect(m_defaultZoomModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_defaultZoomSpinBox->setEnabled(index == 4); // 自定义模式
        onSettingChanged();
    });
    connect(m_defaultZoomSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_smoothScrollingCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_invertScrollCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_scrollSpeedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_fullscreenHideUICheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_doubleClickFullscreenCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_pageTurnModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_preloadPagesCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_preloadCountSpinBox->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_preloadCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_backgroundColorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_customBackgroundColorButton->setEnabled(m_backgroundColorComboBox->currentData().toString() == "custom");
        onSettingChanged();
    });
    connect(m_customBackgroundColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    
    // 图书馆页面信号
    connect(m_scanDirectoriesListWidget, &QListWidget::itemSelectionChanged, [this]() {
        m_removeDirectoryButton->setEnabled(!m_scanDirectoriesListWidget->selectedItems().isEmpty());
    });
    connect(m_addDirectoryButton, &QPushButton::clicked, this, &SettingsDialog::onAddDirectoryClicked);
    connect(m_removeDirectoryButton, &QPushButton::clicked, this, &SettingsDialog::onRemoveDirectoryClicked);
    connect(m_autoScanCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_scanIntervalSpinBox->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_scanIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_recursiveScanCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_defaultViewModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_defaultSortModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_thumbnailSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_generateThumbnailsCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_rememberReadingPositionCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    
    // 缓存页面信号
    connect(m_memoryCacheSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_enableDiskCacheCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_diskCacheSizeSpinBox->setEnabled(enabled);
        m_cacheDirectoryLineEdit->setEnabled(enabled);
        m_browseCacheDirectoryButton->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_diskCacheSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_browseCacheDirectoryButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);
    connect(m_cacheCleanupIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_cacheMaxAgeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_clearCacheButton, &QPushButton::clicked, [this]() {
        int ret = QMessageBox::question(this, "确认清空缓存", 
            "确定要清空所有缓存文件吗？这将删除所有已缓存的图片和数据。",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            CacheManager::instance()->clearAllCache();
            updateCacheUsage();
        }
    });
    
    // 网络页面信号
    connect(m_useProxyCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_proxyTypeComboBox->setEnabled(enabled);
        m_proxyHostLineEdit->setEnabled(enabled);
        m_proxyPortSpinBox->setEnabled(enabled);
        m_proxyUsernameLineEdit->setEnabled(enabled);
        m_proxyPasswordLineEdit->setEnabled(enabled);
        m_testConnectionButton->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_proxyTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_proxyHostLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(m_proxyPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_proxyUsernameLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(m_proxyPasswordLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(m_testConnectionButton, &QPushButton::clicked, this, &SettingsDialog::onTestConnectionClicked);
    connect(m_connectionTimeoutSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_downloadTimeoutSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_maxConcurrentDownloadsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_retryCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_enableSpeedLimitCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_speedLimitSpinBox->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_speedLimitSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    
    // 外观页面信号
    connect(m_styleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_darkModeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onSettingChanged();
    });
    connect(m_fontFamilyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        updateFontPreview();
        onSettingChanged();
    });
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this]() {
        updateFontPreview();
        onSettingChanged();
    });
    connect(m_selectFontButton, &QPushButton::clicked, this, &SettingsDialog::onFontButtonClicked);
    connect(m_customColorsCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_primaryColorButton->setEnabled(enabled);
        m_secondaryColorButton->setEnabled(enabled);
        m_accentColorButton->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_primaryColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    connect(m_secondaryColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    connect(m_accentColorButton, &QPushButton::clicked, this, &SettingsDialog::onColorButtonClicked);
    
    // 高级页面信号
    connect(m_enableLoggingCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_logLevelComboBox->setEnabled(enabled);
        m_logDirectoryLineEdit->setEnabled(enabled);
        m_browseLogDirectoryButton->setEnabled(enabled);
        m_maxLogFilesSpinBox->setEnabled(enabled);
        m_maxLogFileSizeSpinBox->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_logLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_browseLogDirectoryButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);
    connect(m_maxLogFilesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_maxLogFileSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_enableDebugModeCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_enablePerformanceMonitoringCheckBox, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(m_maxUndoStepsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_autoSaveSettingsCheckBox, &QCheckBox::toggled, [this](bool enabled) {
        m_autoSaveIntervalSpinBox->setEnabled(enabled);
        onSettingChanged();
    });
    connect(m_autoSaveIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(m_exportSettingsButton, &QPushButton::clicked, this, &SettingsDialog::onExportSettingsClicked);
    connect(m_importSettingsButton, &QPushButton::clicked, this, &SettingsDialog::onImportSettingsClicked);
    connect(m_resetAllSettingsButton, &QPushButton::clicked, [this]() {
        int ret = QMessageBox::question(this, "确认重置", 
            "确定要将所有设置重置为默认值吗？这将丢失所有自定义设置。",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            resetToDefaults();
        }
    });
}

void SettingsDialog::addSettingsPage(const QString &title, QWidget *page)
{
    m_tabWidget->addTab(page, title);
}

void SettingsDialog::selectPage(const QString &title)
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (m_tabWidget->tabText(i) == title) {
            m_tabWidget->setCurrentIndex(i);
            break;
        }
    }
}

void SettingsDialog::applySettings()
{
    saveSettings();
    m_hasUnsavedChanges = false;
    m_applyButton->setEnabled(false);
    emit settingsApplied();
}

void SettingsDialog::resetToDefaults()
{
    // 通用设置默认值
    m_startWithSystemCheckBox->setChecked(false);
    m_minimizeToTrayCheckBox->setChecked(false);
    m_rememberWindowStateCheckBox->setChecked(true);
    m_checkUpdatesCheckBox->setChecked(true);
    m_languageComboBox->setCurrentIndex(0);
    m_themeComboBox->setCurrentIndex(0);
    
    // 阅读设置默认值
    m_defaultZoomModeComboBox->setCurrentIndex(0);
    m_defaultZoomSpinBox->setValue(100);
    m_smoothScrollingCheckBox->setChecked(true);
    m_invertScrollCheckBox->setChecked(false);
    m_scrollSpeedSpinBox->setValue(3);
    m_fullscreenHideUICheckBox->setChecked(true);
    m_doubleClickFullscreenCheckBox->setChecked(true);
    m_pageTurnModeComboBox->setCurrentIndex(0);
    m_preloadPagesCheckBox->setChecked(true);
    m_preloadCountSpinBox->setValue(2);
    m_backgroundColorComboBox->setCurrentIndex(0);
    
    // 图书馆设置默认值
    m_scanDirectoriesListWidget->clear();
    m_autoScanCheckBox->setChecked(true);
    m_scanIntervalSpinBox->setValue(5);
    m_recursiveScanCheckBox->setChecked(true);
    m_defaultViewModeComboBox->setCurrentIndex(0);
    m_defaultSortModeComboBox->setCurrentIndex(0);
    m_thumbnailSizeSpinBox->setValue(128);
    m_generateThumbnailsCheckBox->setChecked(true);
    m_rememberReadingPositionCheckBox->setChecked(true);
    
    // 缓存设置默认值
    m_memoryCacheSizeSpinBox->setValue(100);
    m_enableDiskCacheCheckBox->setChecked(true);
    m_diskCacheSizeSpinBox->setValue(500);
    m_cacheDirectoryLineEdit->setText(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_cacheCleanupIntervalSpinBox->setValue(30);
    m_cacheMaxAgeSpinBox->setValue(7);
    
    // 网络设置默认值
    m_useProxyCheckBox->setChecked(false);
    m_proxyTypeComboBox->setCurrentIndex(0);
    m_proxyHostLineEdit->clear();
    m_proxyPortSpinBox->setValue(8080);
    m_proxyUsernameLineEdit->clear();
    m_proxyPasswordLineEdit->clear();
    m_connectionTimeoutSpinBox->setValue(30);
    m_downloadTimeoutSpinBox->setValue(60);
    m_maxConcurrentDownloadsSpinBox->setValue(3);
    m_retryCountSpinBox->setValue(3);
    m_enableSpeedLimitCheckBox->setChecked(false);
    m_speedLimitSpinBox->setValue(1024);
    
    // 外观设置默认值
    m_styleComboBox->setCurrentIndex(0);
    m_darkModeCheckBox->setChecked(false);
    m_opacitySlider->setValue(100);
    m_fontFamilyComboBox->setCurrentText(QApplication::font().family());
    m_fontSizeSpinBox->setValue(QApplication::font().pointSize());
    m_customColorsCheckBox->setChecked(false);
    m_primaryColor = Qt::blue;
    m_secondaryColor = Qt::lightGray;
    m_accentColor = Qt::darkBlue;
    
    // 高级设置默认值
    m_enableLoggingCheckBox->setChecked(false);
    m_logLevelComboBox->setCurrentIndex(1);
    m_logDirectoryLineEdit->setText(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs");
    m_maxLogFilesSpinBox->setValue(10);
    m_maxLogFileSizeSpinBox->setValue(10);
    m_enableDebugModeCheckBox->setChecked(false);
    m_enablePerformanceMonitoringCheckBox->setChecked(false);
    m_maxUndoStepsSpinBox->setValue(20);
    m_autoSaveSettingsCheckBox->setChecked(true);
    m_autoSaveIntervalSpinBox->setValue(5);
    
    updateColorButtons();
    updateFontPreview();
    
    m_hasUnsavedChanges = true;
    m_applyButton->setEnabled(true);
    
    emit settingsReset();
}

void SettingsDialog::loadSettings()
{
    // 通用设置
    m_startWithSystemCheckBox->setChecked(m_configManager->getValue("general/startWithSystem", false).toBool());
    m_minimizeToTrayCheckBox->setChecked(m_configManager->getValue("general/minimizeToTray", false).toBool());
    m_rememberWindowStateCheckBox->setChecked(m_configManager->getValue("general/rememberWindowState", true).toBool());
    m_checkUpdatesCheckBox->setChecked(m_configManager->getValue("general/checkUpdates", true).toBool());
    
    QString language = m_configManager->getValue("general/language", "zh_CN").toString();
    int langIndex = m_languageComboBox->findData(language);
    if (langIndex >= 0) m_languageComboBox->setCurrentIndex(langIndex);
    
    QString theme = m_configManager->getValue("general/theme", "system").toString();
    int themeIndex = m_themeComboBox->findData(theme);
    if (themeIndex >= 0) m_themeComboBox->setCurrentIndex(themeIndex);
    
    // 阅读设置
    m_defaultZoomModeComboBox->setCurrentIndex(m_configManager->getValue("reading/defaultZoomMode", 0).toInt());
    m_defaultZoomSpinBox->setValue(m_configManager->getValue("reading/defaultZoomLevel", 100).toInt());
    m_smoothScrollingCheckBox->setChecked(m_configManager->getValue("reading/smoothScrolling", true).toBool());
    m_invertScrollCheckBox->setChecked(m_configManager->getValue("reading/invertScroll", false).toBool());
    m_scrollSpeedSpinBox->setValue(m_configManager->getValue("reading/scrollSpeed", 3).toInt());
    m_fullscreenHideUICheckBox->setChecked(m_configManager->getValue("reading/fullscreenHideUI", true).toBool());
    m_doubleClickFullscreenCheckBox->setChecked(m_configManager->getValue("reading/doubleClickFullscreen", true).toBool());
    m_pageTurnModeComboBox->setCurrentIndex(m_configManager->getValue("reading/pageTurnMode", 0).toInt());
    m_preloadPagesCheckBox->setChecked(m_configManager->getValue("reading/preloadPages", true).toBool());
    m_preloadCountSpinBox->setValue(m_configManager->getValue("reading/preloadCount", 2).toInt());
    
    QString bgColor = m_configManager->getValue("reading/backgroundColor", "white").toString();
    int bgIndex = m_backgroundColorComboBox->findData(bgColor);
    if (bgIndex >= 0) m_backgroundColorComboBox->setCurrentIndex(bgIndex);
    
    m_customBackgroundColor = QColor(m_configManager->getValue("reading/customBackgroundColor", "#FFFFFF").toString());
    
    // 图书馆设置
    QStringList scanDirs = m_configManager->getValue("library/scanDirectories", QStringList()).toStringList();
    m_scanDirectoriesListWidget->addItems(scanDirs);
    
    m_autoScanCheckBox->setChecked(m_configManager->getValue("library/autoScan", true).toBool());
    m_scanIntervalSpinBox->setValue(m_configManager->getValue("library/scanInterval", 5).toInt());
    m_recursiveScanCheckBox->setChecked(m_configManager->getValue("library/recursiveScan", true).toBool());
    m_defaultViewModeComboBox->setCurrentIndex(m_configManager->getValue("library/defaultViewMode", 0).toInt());
    m_defaultSortModeComboBox->setCurrentIndex(m_configManager->getValue("library/defaultSortMode", 0).toInt());
    m_thumbnailSizeSpinBox->setValue(m_configManager->getValue("library/thumbnailSize", 128).toInt());
    m_generateThumbnailsCheckBox->setChecked(m_configManager->getValue("library/generateThumbnails", true).toBool());
    m_rememberReadingPositionCheckBox->setChecked(m_configManager->getValue("library/rememberReadingPosition", true).toBool());
    
    // 缓存设置
    m_memoryCacheSizeSpinBox->setValue(m_configManager->getValue("cache/memoryCacheSize", 100).toInt());
    m_enableDiskCacheCheckBox->setChecked(m_configManager->getValue("cache/enableDiskCache", true).toBool());
    m_diskCacheSizeSpinBox->setValue(m_configManager->getValue("cache/diskCacheSize", 500).toInt());
    m_cacheDirectoryLineEdit->setText(m_configManager->getValue("cache/cacheDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString());
    m_cacheCleanupIntervalSpinBox->setValue(m_configManager->getValue("cache/cleanupInterval", 30).toInt());
    m_cacheMaxAgeSpinBox->setValue(m_configManager->getValue("cache/maxAge", 7).toInt());
    
    updateCacheUsage();
    
    // 网络设置
    m_useProxyCheckBox->setChecked(m_configManager->getValue("network/useProxy", false).toBool());
    m_proxyTypeComboBox->setCurrentIndex(m_configManager->getValue("network/proxyType", 0).toInt());
    m_proxyHostLineEdit->setText(m_configManager->getValue("network/proxyHost", "").toString());
    m_proxyPortSpinBox->setValue(m_configManager->getValue("network/proxyPort", 8080).toInt());
    m_proxyUsernameLineEdit->setText(m_configManager->getValue("network/proxyUsername", "").toString());
    m_proxyPasswordLineEdit->setText(m_configManager->getValue("network/proxyPassword", "").toString());
    m_connectionTimeoutSpinBox->setValue(m_configManager->getValue("network/connectionTimeout", 30).toInt());
    m_downloadTimeoutSpinBox->setValue(m_configManager->getValue("network/downloadTimeout", 60).toInt());
    m_maxConcurrentDownloadsSpinBox->setValue(m_configManager->getValue("network/maxConcurrentDownloads", 3).toInt());
    m_retryCountSpinBox->setValue(m_configManager->getValue("network/retryCount", 3).toInt());
    m_enableSpeedLimitCheckBox->setChecked(m_configManager->getValue("network/enableSpeedLimit", false).toBool());
    m_speedLimitSpinBox->setValue(m_configManager->getValue("network/speedLimit", 1024).toInt());
    
    // 外观设置
    QString style = m_configManager->getValue("appearance/style", "").toString();
    if (!style.isEmpty()) {
        int styleIndex = m_styleComboBox->findText(style);
        if (styleIndex >= 0) m_styleComboBox->setCurrentIndex(styleIndex);
    }
    
    m_darkModeCheckBox->setChecked(m_configManager->getValue("appearance/darkMode", false).toBool());
    m_opacitySlider->setValue(m_configManager->getValue("appearance/opacity", 100).toInt());
    
    QString fontFamily = m_configManager->getValue("appearance/fontFamily", QApplication::font().family()).toString();
    int fontIndex = m_fontFamilyComboBox->findText(fontFamily);
    if (fontIndex >= 0) m_fontFamilyComboBox->setCurrentIndex(fontIndex);
    
    m_fontSizeSpinBox->setValue(m_configManager->getValue("appearance/fontSize", QApplication::font().pointSize()).toInt());
    
    m_customColorsCheckBox->setChecked(m_configManager->getValue("appearance/customColors", false).toBool());
    m_primaryColor = QColor(m_configManager->getValue("appearance/primaryColor", "#0066CC").toString());
    m_secondaryColor = QColor(m_configManager->getValue("appearance/secondaryColor", "#CCCCCC").toString());
    m_accentColor = QColor(m_configManager->getValue("appearance/accentColor", "#004499").toString());
    
    updateColorButtons();
    updateFontPreview();
    
    // 高级设置
    m_enableLoggingCheckBox->setChecked(m_configManager->getValue("advanced/enableLogging", false).toBool());
    m_logLevelComboBox->setCurrentIndex(m_configManager->getValue("advanced/logLevel", 1).toInt());
    m_logDirectoryLineEdit->setText(m_configManager->getValue("advanced/logDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs").toString());
    m_maxLogFilesSpinBox->setValue(m_configManager->getValue("advanced/maxLogFiles", 10).toInt());
    m_maxLogFileSizeSpinBox->setValue(m_configManager->getValue("advanced/maxLogFileSize", 10).toInt());
    m_enableDebugModeCheckBox->setChecked(m_configManager->getValue("advanced/enableDebugMode", false).toBool());
    m_enablePerformanceMonitoringCheckBox->setChecked(m_configManager->getValue("advanced/enablePerformanceMonitoring", false).toBool());
    m_maxUndoStepsSpinBox->setValue(m_configManager->getValue("advanced/maxUndoSteps", 20).toInt());
    m_autoSaveSettingsCheckBox->setChecked(m_configManager->getValue("advanced/autoSaveSettings", true).toBool());
    m_autoSaveIntervalSpinBox->setValue(m_configManager->getValue("advanced/autoSaveInterval", 5).toInt());
    
    m_hasUnsavedChanges = false;
    m_applyButton->setEnabled(false);
}

void SettingsDialog::saveSettings()
{
    // 通用设置
    m_configManager->setValue("general/startWithSystem", m_startWithSystemCheckBox->isChecked());
    m_configManager->setValue("general/minimizeToTray", m_minimizeToTrayCheckBox->isChecked());
    m_configManager->setValue("general/rememberWindowState", m_rememberWindowStateCheckBox->isChecked());
    m_configManager->setValue("general/checkUpdates", m_checkUpdatesCheckBox->isChecked());
    m_configManager->setValue("general/language", m_languageComboBox->currentData());
    m_configManager->setValue("general/theme", m_themeComboBox->currentData());
    
    // 阅读设置
    m_configManager->setValue("reading/defaultZoomMode", m_defaultZoomModeComboBox->currentIndex());
    m_configManager->setValue("reading/defaultZoomLevel", m_defaultZoomSpinBox->value());
    m_configManager->setValue("reading/smoothScrolling", m_smoothScrollingCheckBox->isChecked());
    m_configManager->setValue("reading/invertScroll", m_invertScrollCheckBox->isChecked());
    m_configManager->setValue("reading/scrollSpeed", m_scrollSpeedSpinBox->value());
    m_configManager->setValue("reading/fullscreenHideUI", m_fullscreenHideUICheckBox->isChecked());
    m_configManager->setValue("reading/doubleClickFullscreen", m_doubleClickFullscreenCheckBox->isChecked());
    m_configManager->setValue("reading/pageTurnMode", m_pageTurnModeComboBox->currentIndex());
    m_configManager->setValue("reading/preloadPages", m_preloadPagesCheckBox->isChecked());
    m_configManager->setValue("reading/preloadCount", m_preloadCountSpinBox->value());
    m_configManager->setValue("reading/backgroundColor", m_backgroundColorComboBox->currentData());
    m_configManager->setValue("reading/customBackgroundColor", m_customBackgroundColor.name());
    
    // 图书馆设置
    QStringList scanDirs;
    for (int i = 0; i < m_scanDirectoriesListWidget->count(); ++i) {
        scanDirs << m_scanDirectoriesListWidget->item(i)->text();
    }
    m_configManager->setValue("library/scanDirectories", scanDirs);
    m_configManager->setValue("library/autoScan", m_autoScanCheckBox->isChecked());
    m_configManager->setValue("library/scanInterval", m_scanIntervalSpinBox->value());
    m_configManager->setValue("library/recursiveScan", m_recursiveScanCheckBox->isChecked());
    m_configManager->setValue("library/defaultViewMode", m_defaultViewModeComboBox->currentIndex());
    m_configManager->setValue("library/defaultSortMode", m_defaultSortModeComboBox->currentIndex());
    m_configManager->setValue("library/thumbnailSize", m_thumbnailSizeSpinBox->value());
    m_configManager->setValue("library/generateThumbnails", m_generateThumbnailsCheckBox->isChecked());
    m_configManager->setValue("library/rememberReadingPosition", m_rememberReadingPositionCheckBox->isChecked());
    
    // 缓存设置
    m_configManager->setValue("cache/memoryCacheSize", m_memoryCacheSizeSpinBox->value());
    m_configManager->setValue("cache/enableDiskCache", m_enableDiskCacheCheckBox->isChecked());
    m_configManager->setValue("cache/diskCacheSize", m_diskCacheSizeSpinBox->value());
    m_configManager->setValue("cache/cacheDirectory", m_cacheDirectoryLineEdit->text());
    m_configManager->setValue("cache/cleanupInterval", m_cacheCleanupIntervalSpinBox->value());
    m_configManager->setValue("cache/maxAge", m_cacheMaxAgeSpinBox->value());
    
    // 应用缓存设置
    CacheManager *cacheManager = CacheManager::instance();
    cacheManager->setMaxMemoryCacheSize(m_memoryCacheSizeSpinBox->value());
    cacheManager->setMaxDiskCacheSize(m_diskCacheSizeSpinBox->value());
    cacheManager->setCacheDirectory(m_cacheDirectoryLineEdit->text());
    cacheManager->setAutoCleanupInterval(m_cacheCleanupIntervalSpinBox->value());
    
    // 网络设置
    m_configManager->setValue("network/useProxy", m_useProxyCheckBox->isChecked());
    m_configManager->setValue("network/proxyType", m_proxyTypeComboBox->currentData());
    m_configManager->setValue("network/proxyHost", m_proxyHostLineEdit->text());
    m_configManager->setValue("network/proxyPort", m_proxyPortSpinBox->value());
    m_configManager->setValue("network/proxyUsername", m_proxyUsernameLineEdit->text());
    m_configManager->setValue("network/proxyPassword", m_proxyPasswordLineEdit->text());
    m_configManager->setValue("network/connectionTimeout", m_connectionTimeoutSpinBox->value());
    m_configManager->setValue("network/downloadTimeout", m_downloadTimeoutSpinBox->value());
    m_configManager->setValue("network/maxConcurrentDownloads", m_maxConcurrentDownloadsSpinBox->value());
    m_configManager->setValue("network/retryCount", m_retryCountSpinBox->value());
    m_configManager->setValue("network/enableSpeedLimit", m_enableSpeedLimitCheckBox->isChecked());
    m_configManager->setValue("network/speedLimit", m_speedLimitSpinBox->value());
    
    // 应用网络设置
    DownloadManager *downloadManager = DownloadManager::instance();
    downloadManager->setMaxConcurrentDownloads(m_maxConcurrentDownloadsSpinBox->value());
    downloadManager->setRetryCount(m_retryCountSpinBox->value());
    downloadManager->setTimeout(m_downloadTimeoutSpinBox->value());
    if (m_enableSpeedLimitCheckBox->isChecked()) {
        downloadManager->setSpeedLimit(m_speedLimitSpinBox->value() * 1024); // 转换为bytes/s
    } else {
        downloadManager->setSpeedLimit(0);
    }
    
    // 外观设置
    m_configManager->setValue("appearance/style", m_styleComboBox->currentText());
    m_configManager->setValue("appearance/darkMode", m_darkModeCheckBox->isChecked());
    m_configManager->setValue("appearance/opacity", m_opacitySlider->value());
    m_configManager->setValue("appearance/fontFamily", m_fontFamilyComboBox->currentText());
    m_configManager->setValue("appearance/fontSize", m_fontSizeSpinBox->value());
    m_configManager->setValue("appearance/customColors", m_customColorsCheckBox->isChecked());
    m_configManager->setValue("appearance/primaryColor", m_primaryColor.name());
    m_configManager->setValue("appearance/secondaryColor", m_secondaryColor.name());
    m_configManager->setValue("appearance/accentColor", m_accentColor.name());
    
    // 高级设置
    m_configManager->setValue("advanced/enableLogging", m_enableLoggingCheckBox->isChecked());
    m_configManager->setValue("advanced/logLevel", m_logLevelComboBox->currentIndex());
    m_configManager->setValue("advanced/logDirectory", m_logDirectoryLineEdit->text());
    m_configManager->setValue("advanced/maxLogFiles", m_maxLogFilesSpinBox->value());
    m_configManager->setValue("advanced/maxLogFileSize", m_maxLogFileSizeSpinBox->value());
    m_configManager->setValue("advanced/enableDebugMode", m_enableDebugModeCheckBox->isChecked());
    m_configManager->setValue("advanced/enablePerformanceMonitoring", m_enablePerformanceMonitoringCheckBox->isChecked());
    m_configManager->setValue("advanced/maxUndoSteps", m_maxUndoStepsSpinBox->value());
    m_configManager->setValue("advanced/autoSaveSettings", m_autoSaveSettingsCheckBox->isChecked());
    m_configManager->setValue("advanced/autoSaveInterval", m_autoSaveIntervalSpinBox->value());
    
    emit settingsChanged();
}

// 槽函数实现
void SettingsDialog::onApplyClicked()
{
    applySettings();
}

void SettingsDialog::onResetClicked()
{
    resetToDefaults();
}

void SettingsDialog::onOkClicked()
{
    if (m_hasUnsavedChanges) {
        applySettings();
    }
    accept();
}

void SettingsDialog::onCancelClicked()
{
    if (m_hasUnsavedChanges) {
        int ret = QMessageBox::question(this, "未保存的更改",
            "您有未保存的设置更改。是否要保存这些更改？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        
        if (ret == QMessageBox::Save) {
            applySettings();
            accept();
        } else if (ret == QMessageBox::Discard) {
            reject();
        }
        // Cancel: 不做任何事，保持对话框打开
    } else {
        reject();
    }
}

void SettingsDialog::onPageChanged(int index)
{
    Q_UNUSED(index)
    // 可以在这里添加页面切换时的特殊处理
    if (index == 3) { // 缓存页面
        updateCacheUsage();
    }
}

void SettingsDialog::onSettingChanged()
{
    if (!m_hasUnsavedChanges) {
        m_hasUnsavedChanges = true;
        m_applyButton->setEnabled(true);
        setWindowTitle("设置 *");
    }
}

void SettingsDialog::onBrowseClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    QString dir;
    
    if (button == m_browseCacheDirectoryButton) {
        dir = QFileDialog::getExistingDirectory(this, "选择缓存目录", 
            m_cacheDirectoryLineEdit->text());
        if (!dir.isEmpty()) {
            m_cacheDirectoryLineEdit->setText(dir);
            onSettingChanged();
        }
    } else if (button == m_browseLogDirectoryButton) {
        dir = QFileDialog::getExistingDirectory(this, "选择日志目录", 
            m_logDirectoryLineEdit->text());
        if (!dir.isEmpty()) {
            m_logDirectoryLineEdit->setText(dir);
            onSettingChanged();
        }
    }
}

void SettingsDialog::onColorButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    QColor currentColor;
    QString title;
    
    if (button == m_customBackgroundColorButton) {
        currentColor = m_customBackgroundColor;
        title = "选择背景颜色";
    } else if (button == m_primaryColorButton) {
        currentColor = m_primaryColor;
        title = "选择主色调";
    } else if (button == m_secondaryColorButton) {
        currentColor = m_secondaryColor;
        title = "选择辅助色";
    } else if (button == m_accentColorButton) {
        currentColor = m_accentColor;
        title = "选择强调色";
    } else {
        return;
    }
    
    QColor newColor = QColorDialog::getColor(currentColor, this, title);
    if (newColor.isValid()) {
        if (button == m_customBackgroundColorButton) {
            m_customBackgroundColor = newColor;
        } else if (button == m_primaryColorButton) {
            m_primaryColor = newColor;
        } else if (button == m_secondaryColorButton) {
            m_secondaryColor = newColor;
        } else if (button == m_accentColorButton) {
            m_accentColor = newColor;
        }
        
        updateColorButtons();
        onSettingChanged();
    }
}

void SettingsDialog::onFontButtonClicked()
{
    bool ok;
    QFont currentFont(m_fontFamilyComboBox->currentText(), m_fontSizeSpinBox->value());
    QFont newFont = QFontDialog::getFont(&ok, currentFont, this, "选择字体");
    
    if (ok) {
        m_selectedFont = newFont;
        m_fontFamilyComboBox->setCurrentText(newFont.family());
        m_fontSizeSpinBox->setValue(newFont.pointSize());
        updateFontPreview();
        onSettingChanged();
    }
}

void SettingsDialog::onAddDirectoryClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择扫描目录",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    
    if (!dir.isEmpty()) {
        // 检查是否已存在
        bool exists = false;
        for (int i = 0; i < m_scanDirectoriesListWidget->count(); ++i) {
            if (m_scanDirectoriesListWidget->item(i)->text() == dir) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            m_scanDirectoriesListWidget->addItem(dir);
            onSettingChanged();
        } else {
            QMessageBox::information(this, "目录已存在", "该目录已在扫描列表中。");
        }
    }
}

void SettingsDialog::onRemoveDirectoryClicked()
{
    QListWidgetItem *item = m_scanDirectoriesListWidget->currentItem();
    if (item) {
        delete item;
        onSettingChanged();
    }
}

void SettingsDialog::onTestConnectionClicked()
{
    // 简单的连接测试
    m_testConnectionButton->setEnabled(false);
    m_testConnectionButton->setText("测试中...");
    
    // 这里可以实现实际的网络连接测试
    // 为了简化，我们模拟一个测试过程
    QTimer::singleShot(2000, this, [this]() {
        m_testConnectionButton->setEnabled(true);
        m_testConnectionButton->setText("测试连接");
        
        if (m_proxyHostLineEdit->text().isEmpty()) {
            QMessageBox::warning(this, "连接测试", "请先输入代理服务器地址。");
        } else {
            QMessageBox::information(this, "连接测试", "代理服务器连接测试完成。\n注意：这是一个模拟测试。");
        }
    });
}

void SettingsDialog::onExportSettingsClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出设置",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ComicReader_Settings.json",
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_configManager->exportSettings(fileName)) {
            QMessageBox::information(this, "导出成功", "设置已成功导出到文件。");
        } else {
            QMessageBox::warning(this, "导出失败", "无法导出设置到指定文件。");
        }
    }
}

void SettingsDialog::onImportSettingsClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入设置",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        int ret = QMessageBox::question(this, "确认导入",
            "导入设置将覆盖当前所有设置。是否继续？",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            if (m_configManager->importSettings(fileName)) {
                loadSettings();
                QMessageBox::information(this, "导入成功", "设置已成功从文件导入。");
            } else {
                QMessageBox::warning(this, "导入失败", "无法从指定文件导入设置。");
            }
        }
    }
}

// 辅助方法实现
void SettingsDialog::updateColorButtons()
{
    // 更新颜色按钮的显示
    if (m_customBackgroundColorButton) {
        QString style = QString("QPushButton { background-color: %1; }")
            .arg(m_customBackgroundColor.name());
        m_customBackgroundColorButton->setStyleSheet(style);
    }
    
    if (m_primaryColorButton) {
        QString style = QString("QPushButton { background-color: %1; }")
            .arg(m_primaryColor.name());
        m_primaryColorButton->setStyleSheet(style);
    }
    
    if (m_secondaryColorButton) {
        QString style = QString("QPushButton { background-color: %1; }")
            .arg(m_secondaryColor.name());
        m_secondaryColorButton->setStyleSheet(style);
    }
    
    if (m_accentColorButton) {
        QString style = QString("QPushButton { background-color: %1; }")
            .arg(m_accentColor.name());
        m_accentColorButton->setStyleSheet(style);
    }
}

void SettingsDialog::updateFontLabels()
{
    updateFontPreview();
}

void SettingsDialog::updateFontPreview()
{
    if (m_fontPreviewLabel) {
        QFont previewFont(m_fontFamilyComboBox->currentText(), m_fontSizeSpinBox->value());
        m_fontPreviewLabel->setFont(previewFont);
    }
}

void SettingsDialog::updateCacheUsage()
{
    if (m_cacheUsageLabel) {
        CacheManager *cacheManager = CacheManager::instance();
        int memoryUsage = cacheManager->getMemoryCacheSize();
        qint64 diskUsage = cacheManager->getDiskCacheSize();
        
        QString usageText = QString("内存缓存: %1 MB, 磁盘缓存: %2 MB")
            .arg(memoryUsage)
            .arg(diskUsage);
        
        m_cacheUsageLabel->setText(usageText);
    }
}

void SettingsDialog::validateSettings()
{
    // 这里可以添加设置验证逻辑
    // 例如检查路径是否有效、数值是否在合理范围内等
}

bool SettingsDialog::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}