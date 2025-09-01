#include "../../include/ui/SettingsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFormLayout>
#include <QGridLayout>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

void SettingsWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    // 创建标签页组件
    tabWidget = new QTabWidget(this);
    
    createGeneralTab();
    createReaderTab();
    createDownloadTab();
    createNetworkTab();
    createAboutTab();
    
    mainLayout->addWidget(tabWidget);
    
    // 创建按钮布局
    buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    saveButton = new QPushButton("保存", this);
    resetButton = new QPushButton("重置", this);
    cancelButton = new QPushButton("取消", this);
    
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(saveButton, &QPushButton::clicked, this, &SettingsWidget::saveSettings);
    connect(resetButton, &QPushButton::clicked, this, &SettingsWidget::resetSettings);
}

void SettingsWidget::createGeneralTab()
{
    generalTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(generalTab);
    
    // 启动设置组
    QGroupBox *startupGroup = new QGroupBox("启动设置");
    QVBoxLayout *startupLayout = new QVBoxLayout(startupGroup);
    
    autoStartCheckBox = new QCheckBox("开机自启动");
    minimizeToTrayCheckBox = new QCheckBox("启动时最小化到系统托盘");
    checkUpdatesCheckBox = new QCheckBox("自动检查更新");
    
    startupLayout->addWidget(autoStartCheckBox);
    startupLayout->addWidget(minimizeToTrayCheckBox);
    startupLayout->addWidget(checkUpdatesCheckBox);
    
    // 界面设置组
    QGroupBox *interfaceGroup = new QGroupBox("界面设置");
    QFormLayout *interfaceLayout = new QFormLayout(interfaceGroup);
    
    languageComboBox = new QComboBox();
    languageComboBox->addItems(QStringList() << "简体中文" << "English" << "日本語");
    
    themeComboBox = new QComboBox();
    themeComboBox->addItems(QStringList() << "默认主题" << "深色主题" << "浅色主题");
    
    interfaceLayout->addRow("语言:", languageComboBox);
    interfaceLayout->addRow("主题:", themeComboBox);
    
    layout->addWidget(startupGroup);
    layout->addWidget(interfaceGroup);
    layout->addStretch();
    
    tabWidget->addTab(generalTab, "常规");
}

void SettingsWidget::createReaderTab()
{
    readerTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(readerTab);
    
    // 显示设置组
    QGroupBox *displayGroup = new QGroupBox("显示设置");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);
    
    fullscreenCheckBox = new QCheckBox("启动时全屏显示");
    
    QHBoxLayout *pageLayoutLayout = new QHBoxLayout();
    pageLayoutLayout->addWidget(new QLabel("页面布局:"));
    pageLayoutComboBox = new QComboBox();
    pageLayoutComboBox->addItems(QStringList() << "单页" << "双页" << "连续滚动");
    pageLayoutLayout->addWidget(pageLayoutComboBox);
    pageLayoutLayout->addStretch();
    
    QHBoxLayout *scalingLayout = new QHBoxLayout();
    scalingLayout->addWidget(new QLabel("缩放模式:"));
    scalingModeComboBox = new QComboBox();
    scalingModeComboBox->addItems(QStringList() << "适应窗口" << "适应宽度" << "适应高度" << "原始大小");
    scalingLayout->addWidget(scalingModeComboBox);
    scalingLayout->addStretch();
    
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    zoomLayout->addWidget(new QLabel("默认缩放:"));
    defaultZoomSlider = new QSlider(Qt::Horizontal);
    defaultZoomSlider->setRange(25, 400);
    defaultZoomSlider->setValue(100);
    defaultZoomLabel = new QLabel("100%");
    zoomLayout->addWidget(defaultZoomSlider);
    zoomLayout->addWidget(defaultZoomLabel);
    
    displayLayout->addWidget(fullscreenCheckBox);
    displayLayout->addLayout(pageLayoutLayout);
    displayLayout->addLayout(scalingLayout);
    displayLayout->addLayout(zoomLayout);
    
    // 滚动设置组
    QGroupBox *scrollGroup = new QGroupBox("滚动设置");
    QFormLayout *scrollLayout = new QFormLayout(scrollGroup);
    
    smoothScrollCheckBox = new QCheckBox("平滑滚动");
    scrollSpeedSpinBox = new QSpinBox();
    scrollSpeedSpinBox->setRange(1, 10);
    scrollSpeedSpinBox->setValue(5);
    
    scrollLayout->addRow(smoothScrollCheckBox);
    scrollLayout->addRow("滚动速度:", scrollSpeedSpinBox);
    
    layout->addWidget(displayGroup);
    layout->addWidget(scrollGroup);
    layout->addStretch();
    
    tabWidget->addTab(readerTab, "阅读器");
    
    // 连接缩放滑块信号
    connect(defaultZoomSlider, &QSlider::valueChanged, [this](int value) {
        defaultZoomLabel->setText(QString("%1%").arg(value));
    });
}

void SettingsWidget::createDownloadTab()
{
    downloadTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(downloadTab);
    
    // 下载路径设置组
    QGroupBox *pathGroup = new QGroupBox("下载路径");
    QVBoxLayout *pathLayout = new QVBoxLayout(pathGroup);
    
    QHBoxLayout *downloadPathLayout = new QHBoxLayout();
    downloadPathEdit = new QLineEdit();
    browseDownloadButton = new QPushButton("浏览...");
    downloadPathLayout->addWidget(downloadPathEdit);
    downloadPathLayout->addWidget(browseDownloadButton);
    
    pathLayout->addLayout(downloadPathLayout);
    
    // 下载设置组
    QGroupBox *downloadGroup = new QGroupBox("下载设置");
    QFormLayout *downloadLayout = new QFormLayout(downloadGroup);
    
    maxDownloadsSpinBox = new QSpinBox();
    maxDownloadsSpinBox->setRange(1, 10);
    maxDownloadsSpinBox->setValue(3);
    
    maxSpeedSpinBox = new QSpinBox();
    maxSpeedSpinBox->setRange(0, 10000);
    maxSpeedSpinBox->setValue(0);
    maxSpeedSpinBox->setSuffix(" KB/s");
    maxSpeedSpinBox->setSpecialValueText("无限制");
    
    autoRetryCheckBox = new QCheckBox("自动重试失败的下载");
    
    retryCountSpinBox = new QSpinBox();
    retryCountSpinBox->setRange(1, 10);
    retryCountSpinBox->setValue(3);
    
    downloadLayout->addRow("最大并发下载:", maxDownloadsSpinBox);
    downloadLayout->addRow("限制下载速度:", maxSpeedSpinBox);
    downloadLayout->addRow(autoRetryCheckBox);
    downloadLayout->addRow("重试次数:", retryCountSpinBox);
    
    layout->addWidget(pathGroup);
    layout->addWidget(downloadGroup);
    layout->addStretch();
    
    tabWidget->addTab(downloadTab, "下载");
    
    connect(browseDownloadButton, &QPushButton::clicked, this, &SettingsWidget::browseDownloadPath);
}

void SettingsWidget::createNetworkTab()
{
    networkTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(networkTab);
    
    // 缓存设置组
    QGroupBox *cacheGroup = new QGroupBox("缓存设置");
    QVBoxLayout *cacheLayout = new QVBoxLayout(cacheGroup);
    
    QHBoxLayout *cachePathLayout = new QHBoxLayout();
    cachePathEdit = new QLineEdit();
    browseCacheButton = new QPushButton("浏览...");
    cachePathLayout->addWidget(new QLabel("缓存路径:"));
    cachePathLayout->addWidget(cachePathEdit);
    cachePathLayout->addWidget(browseCacheButton);
    
    QHBoxLayout *cacheSizeLayout = new QHBoxLayout();
    cacheSizeSpinBox = new QSpinBox();
    cacheSizeSpinBox->setRange(10, 1000);
    cacheSizeSpinBox->setValue(100);
    cacheSizeSpinBox->setSuffix(" MB");
    cacheSizeLayout->addWidget(new QLabel("缓存大小:"));
    cacheSizeLayout->addWidget(cacheSizeSpinBox);
    cacheSizeLayout->addStretch();
    
    cacheLayout->addLayout(cachePathLayout);
    cacheLayout->addLayout(cacheSizeLayout);
    
    // 网络设置组
    QGroupBox *networkGroup = new QGroupBox("网络设置");
    QFormLayout *networkLayout = new QFormLayout(networkGroup);
    
    timeoutSpinBox = new QSpinBox();
    timeoutSpinBox->setRange(5, 120);
    timeoutSpinBox->setValue(30);
    timeoutSpinBox->setSuffix(" 秒");
    
    userAgentEdit = new QLineEdit();
    userAgentEdit->setPlaceholderText("默认 User-Agent");
    
    networkLayout->addRow("连接超时:", timeoutSpinBox);
    networkLayout->addRow("User-Agent:", userAgentEdit);
    
    // 代理设置组
    QGroupBox *proxyGroup = new QGroupBox("代理设置");
    QFormLayout *proxyLayout = new QFormLayout(proxyGroup);
    
    useProxyCheckBox = new QCheckBox("使用代理服务器");
    proxyHostEdit = new QLineEdit();
    proxyPortSpinBox = new QSpinBox();
    proxyPortSpinBox->setRange(1, 65535);
    proxyPortSpinBox->setValue(8080);
    
    proxyLayout->addRow(useProxyCheckBox);
    proxyLayout->addRow("代理服务器:", proxyHostEdit);
    proxyLayout->addRow("端口:", proxyPortSpinBox);
    
    layout->addWidget(cacheGroup);
    layout->addWidget(networkGroup);
    layout->addWidget(proxyGroup);
    layout->addStretch();
    
    tabWidget->addTab(networkTab, "网络");
    
    connect(browseCacheButton, &QPushButton::clicked, this, &SettingsWidget::browseCachePath);
}

void SettingsWidget::createAboutTab()
{
    aboutTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(aboutTab);
    
    aboutTextEdit = new QTextEdit();
    aboutTextEdit->setReadOnly(true);
    aboutTextEdit->setHtml(
        "<h2>ComicReader 1.0</h2>"
        "<p><b>一个功能强大的漫画阅读器</b></p>"
        "<p>支持本地和在线漫画阅读、下载管理等功能。</p>"
        "<hr>"
        "<h3>主要功能：</h3>"
        "<ul>"
        "<li>支持多种漫画格式 (.cbz, .cbr, .zip, .rar)</li>"
        "<li>在线漫画浏览和下载</li>"
        "<li>智能缓存管理</li>"
        "<li>多种阅读模式</li>"
        "<li>下载管理器</li>"
        "<li>自定义设置</li>"
        "</ul>"
        "<hr>"
        "<h3>技术信息：</h3>"
        "<p>基于 Qt 框架开发</p>"
        "<p>版权所有 © 2025 ComicReader Team</p>"
    );
    
    layout->addWidget(aboutTextEdit);
    
    tabWidget->addTab(aboutTab, "关于");
}

void SettingsWidget::loadSettings()
{
    // TODO: 从配置文件加载设置
    // 这里设置默认值
    
    QString defaultDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    downloadPathEdit->setText(defaultDownloadPath + "/ComicReader");
    
    QString defaultCachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    cachePathEdit->setText(defaultCachePath);
    
    userAgentEdit->setText("ComicReader/1.0");
}

void SettingsWidget::saveSettings()
{
    // TODO: 保存设置到配置文件
    applySettings();
    QMessageBox::information(this, "提示", "设置已保存。");
}

void SettingsWidget::resetSettings()
{
    int ret = QMessageBox::question(this, "确认", "确定要重置所有设置吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        loadSettings();
        QMessageBox::information(this, "提示", "设置已重置。");
    }
}

void SettingsWidget::browseDownloadPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择下载文件夹", downloadPathEdit->text());
    if (!dir.isEmpty()) {
        downloadPathEdit->setText(dir);
    }
}

void SettingsWidget::browseCachePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择缓存文件夹", cachePathEdit->text());
    if (!dir.isEmpty()) {
        cachePathEdit->setText(dir);
    }
}

void SettingsWidget::onSettingChanged()
{
    // TODO: 标记设置已更改
}

void SettingsWidget::applySettings()
{
    // TODO: 应用设置更改
}
