#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
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

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

public slots:
    void saveSettings();
    void resetSettings();
    void browseDownloadPath();
    void browseCachePath();

private slots:
    void onSettingChanged();

private:
    void setupUI();
    void createGeneralTab();
    void createReaderTab();
    void createDownloadTab();
    void createNetworkTab();
    void createAboutTab();
    void loadSettings();
    void applySettings();

    // UI 组件
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;
    QHBoxLayout *buttonLayout;
    QPushButton *saveButton;
    QPushButton *resetButton;
    QPushButton *cancelButton;
    
    // 常规设置
    QWidget *generalTab;
    QCheckBox *autoStartCheckBox;
    QCheckBox *minimizeToTrayCheckBox;
    QCheckBox *checkUpdatesCheckBox;
    QComboBox *languageComboBox;
    QComboBox *themeComboBox;
    
    // 阅读器设置
    QWidget *readerTab;
    QCheckBox *fullscreenCheckBox;
    QComboBox *pageLayoutComboBox;
    QComboBox *scalingModeComboBox;
    QSlider *defaultZoomSlider;
    QLabel *defaultZoomLabel;
    QCheckBox *smoothScrollCheckBox;
    QSpinBox *scrollSpeedSpinBox;
    
    // 下载设置
    QWidget *downloadTab;
    QLineEdit *downloadPathEdit;
    QPushButton *browseDownloadButton;
    QSpinBox *maxDownloadsSpinBox;
    QSpinBox *maxSpeedSpinBox;
    QCheckBox *autoRetryCheckBox;
    QSpinBox *retryCountSpinBox;
    
    // 网络设置
    QWidget *networkTab;
    QLineEdit *cachePathEdit;
    QPushButton *browseCacheButton;
    QSpinBox *cacheSizeSpinBox;
    QSpinBox *timeoutSpinBox;
    QLineEdit *userAgentEdit;
    QCheckBox *useProxyCheckBox;
    QLineEdit *proxyHostEdit;
    QSpinBox *proxyPortSpinBox;
    
    // 关于页面
    QWidget *aboutTab;
    QTextEdit *aboutTextEdit;
};

#endif // SETTINGSWIDGET_H
