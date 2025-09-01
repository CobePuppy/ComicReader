#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QUrl>

class BrowserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrowserWidget(QWidget *parent = nullptr);

public slots:
    void navigateToUrl(const QString &url);
    void goBack();
    void goForward();
    void refresh();
    void goHome();

private slots:
    void onUrlChanged(const QUrl &url);
    void onLoadProgress(int progress);
    void onLoadFinished(bool success);
    void onAddressBarReturnPressed();

private:
    void setupUI();
    void updateNavigationButtons();

    // UI 组件
    QVBoxLayout *mainLayout;
    QHBoxLayout *navigationLayout;
    
    // 导航组件
    QPushButton *backButton;
    QPushButton *forwardButton;
    QPushButton *refreshButton;
    QPushButton *homeButton;
    QLineEdit *addressBar;
    QPushButton *goButton;
    
    // 浏览器组件
    QTextBrowser *webView;
    QProgressBar *progressBar;
    
    // 默认主页
    QString homePage;
};

#endif // BROWSERWIDGET_H
