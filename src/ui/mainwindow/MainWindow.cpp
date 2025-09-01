#include "../../include/ui/MainWindow.h"
#include "../../include/ui/ReaderWidget.h"
#include "../../include/ui/BrowserWidget.h"
#include "../../include/ui/DownloadManagerWidget.h"
#include "../../include/ui/SettingsWidget.h"

#include <QApplication>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QStackedWidget>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralStack(nullptr)
    , mainSplitter(nullptr)
    , readerWidget(nullptr)
    , browserWidget(nullptr)
    , downloadManagerWidget(nullptr)
    , settingsWidget(nullptr)
{
    setWindowTitle("ComicReader");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    openFileAct = new QAction(tr("打开文件(&O)"), this);
    openFileAct->setShortcuts(QKeySequence::Open);
    openFileAct->setStatusTip(tr("打开本地漫画文件"));
    connect(openFileAct, &QAction::triggered, this, &MainWindow::openFile);

    openUrlAct = new QAction(tr("打开URL(&U)"), this);
    openUrlAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    openUrlAct->setStatusTip(tr("打开在线漫画URL"));
    connect(openUrlAct, &QAction::triggered, this, &MainWindow::openUrl);

    exitAct = new QAction(tr("退出(&E)"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("退出应用程序"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    downloadManagerAct = new QAction(tr("下载管理器(&D)"), this);
    downloadManagerAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    downloadManagerAct->setStatusTip(tr("显示下载管理器"));
    connect(downloadManagerAct, &QAction::triggered, this, &MainWindow::showDownloadManager);

    settingsAct = new QAction(tr("设置(&S)"), this);
    settingsAct->setShortcut(QKeySequence::Preferences);
    settingsAct->setStatusTip(tr("打开设置对话框"));
    connect(settingsAct, &QAction::triggered, this, &MainWindow::showSettings);

    aboutAct = new QAction(tr("关于(&A)"), this);
    aboutAct->setStatusTip(tr("显示关于信息"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(openUrlAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = menuBar()->addMenu(tr("视图(&V)"));

    toolsMenu = menuBar()->addMenu(tr("工具(&T)"));
    toolsMenu->addAction(downloadManagerAct);
    toolsMenu->addSeparator();
    toolsMenu->addAction(settingsAct);

    helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
    mainToolBar = addToolBar(tr("主工具栏"));
    mainToolBar->addAction(openFileAct);
    mainToolBar->addAction(openUrlAct);
    mainToolBar->addSeparator();
    mainToolBar->addAction(downloadManagerAct);
    mainToolBar->addAction(settingsAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("就绪"));
}

void MainWindow::createCentralWidget()
{
    centralStack = new QStackedWidget;
    
    // 创建各个功能组件
    readerWidget = new ReaderWidget;
    browserWidget = new BrowserWidget;
    downloadManagerWidget = new DownloadManagerWidget;
    settingsWidget = new SettingsWidget;
    
    // 添加到堆栈组件
    centralStack->addWidget(readerWidget);
    centralStack->addWidget(browserWidget);
    centralStack->addWidget(downloadManagerWidget);
    centralStack->addWidget(settingsWidget);
    
    setCentralWidget(centralStack);
    centralStack->setCurrentWidget(readerWidget);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("打开漫画文件"), "",
        tr("漫画文件 (*.cbz *.cbr *.zip *.rar);;所有文件 (*.*)"));
    
    if (!fileName.isEmpty()) {
        // TODO: 实现文件打开逻辑
        statusBar()->showMessage(tr("打开文件: %1").arg(fileName), 2000);
    }
}

void MainWindow::openUrl()
{
    bool ok;
    QString url = QInputDialog::getText(this, tr("打开URL"),
        tr("请输入漫画URL:"), QLineEdit::Normal, "", &ok);
    
    if (ok && !url.isEmpty()) {
        // TODO: 实现URL打开逻辑
        statusBar()->showMessage(tr("打开URL: %1").arg(url), 2000);
    }
}

void MainWindow::showDownloadManager()
{
    centralStack->setCurrentWidget(downloadManagerWidget);
    statusBar()->showMessage(tr("下载管理器"), 2000);
}

void MainWindow::showSettings()
{
    centralStack->setCurrentWidget(settingsWidget);
    statusBar()->showMessage(tr("设置"), 2000);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("关于 ComicReader"),
        tr("<h2>ComicReader 1.0</h2>"
           "<p>一个功能强大的漫画阅读器</p>"
           "<p>支持本地和在线漫画阅读、下载管理等功能。</p>"));
}
