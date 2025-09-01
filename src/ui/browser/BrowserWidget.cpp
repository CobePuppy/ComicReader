#include "../../include/ui/BrowserWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

BrowserWidget::BrowserWidget(QWidget *parent)
    : QWidget(parent)
    , homePage("https://www.google.com")
{
    setupUI();
}

void BrowserWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    // 创建导航栏
    navigationLayout = new QHBoxLayout();
    
    backButton = new QPushButton("←", this);
    forwardButton = new QPushButton("→", this);
    refreshButton = new QPushButton("⟳", this);
    homeButton = new QPushButton("🏠", this);
    addressBar = new QLineEdit(this);
    goButton = new QPushButton("转到", this);
    
    // 设置按钮属性
    backButton->setToolTip("后退");
    forwardButton->setToolTip("前进");
    refreshButton->setToolTip("刷新");
    homeButton->setToolTip("主页");
    addressBar->setPlaceholderText("输入URL或搜索...");
    
    // 添加到导航布局
    navigationLayout->addWidget(backButton);
    navigationLayout->addWidget(forwardButton);
    navigationLayout->addWidget(refreshButton);
    navigationLayout->addWidget(homeButton);
    navigationLayout->addWidget(addressBar, 1); // 地址栏占据剩余空间
    navigationLayout->addWidget(goButton);
    
    // 创建进度条
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    progressBar->setMaximumHeight(3);
    
    // 创建网页视图
    webView = new QTextBrowser(this);
    webView->setOpenExternalLinks(true);
    
    // 布局
    mainLayout->addLayout(navigationLayout);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(webView);
    
    connect(backButton, &QPushButton::clicked, this, &BrowserWidget::goBack);
    connect(forwardButton, &QPushButton::clicked, this, &BrowserWidget::goForward);
    connect(refreshButton, &QPushButton::clicked, this, &BrowserWidget::refresh);
    connect(homeButton, &QPushButton::clicked, this, &BrowserWidget::goHome);
    connect(goButton, &QPushButton::clicked, this, &BrowserWidget::onAddressBarReturnPressed);
    connect(addressBar, &QLineEdit::returnPressed, this, &BrowserWidget::onAddressBarReturnPressed);
    
    // 加载主页
    goHome();
}

void BrowserWidget::navigateToUrl(const QString &url)
{
    QString processedUrl = url;
    
    // 如果URL不包含协议，添加http://
    if (!processedUrl.startsWith("http://") && !processedUrl.startsWith("https://")) {
        processedUrl = "http://" + processedUrl;
    }
    
    // 对于 QTextBrowser，我们显示一个简单的消息
    webView->setHtml(QString("<h2>浏览器功能</h2><p>正在加载: %1</p><p>注意：当前使用简化的浏览器视图。</p>").arg(processedUrl));
}

void BrowserWidget::goBack()
{
    // 对于 QTextBrowser，禁用后退功能
}

void BrowserWidget::goForward()
{
    // 对于 QTextBrowser，禁用前进功能
}

void BrowserWidget::refresh()
{
    // 对于 QTextBrowser，重新设置当前内容
    QString currentUrl = addressBar->text();
    if (!currentUrl.isEmpty()) {
        navigateToUrl(currentUrl);
    }
}

void BrowserWidget::goHome()
{
    navigateToUrl(homePage);
}

void BrowserWidget::onUrlChanged(const QUrl &url)
{
    addressBar->setText(url.toString());
    updateNavigationButtons();
}

void BrowserWidget::onLoadProgress(int progress)
{
    if (progress < 100) {
        progressBar->setVisible(true);
        progressBar->setValue(progress);
    } else {
        progressBar->setVisible(false);
    }
}

void BrowserWidget::onLoadFinished(bool success)
{
    progressBar->setVisible(false);
    updateNavigationButtons();
    Q_UNUSED(success)
}

void BrowserWidget::onAddressBarReturnPressed()
{
    QString url = addressBar->text().trimmed();
    if (!url.isEmpty()) {
        navigateToUrl(url);
    }
}

void BrowserWidget::updateNavigationButtons()
{
    // 对于 QTextBrowser，禁用导航按钮
    backButton->setEnabled(false);
    forwardButton->setEnabled(false);
}
