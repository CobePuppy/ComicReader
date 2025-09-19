#include "ui/MainWindow.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_contentLabel(nullptr)
    , m_fileMenu(nullptr)
    , m_viewMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_mainToolBar(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_openFileAct(nullptr)
    , m_exitAct(nullptr)
    , m_fullscreenAct(nullptr)
    , m_aboutAct(nullptr)
    , m_isFullscreen(false)
{
    setWindowTitle("ComicReader v1.0.0");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    setupUI();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
}

MainWindow::~MainWindow()
{
    // Qt会自动管理子对象的内存
}

void MainWindow::setupUI()
{
    // 创建中央组件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(m_centralWidget);
    
    // 创建内容标签
    m_contentLabel = new QLabel("欢迎使用 ComicReader！\n\n请使用 文件 -> 打开 来选择漫画文件", this);
    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_contentLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 18px;"
        "  color: #666;"
        "  background-color: #f5f5f5;"
        "  border: 2px dashed #ccc;"
        "  border-radius: 10px;"
        "  padding: 50px;"
        "}"
    );
    
    layout->addWidget(m_contentLabel);
}

void MainWindow::createActions()
{
    // 打开文件动作
    m_openFileAct = new QAction("打开文件(&O)...", this);
    m_openFileAct->setShortcut(QKeySequence::Open);
    m_openFileAct->setStatusTip("打开漫画文件");
    connect(m_openFileAct, &QAction::triggered, this, &MainWindow::openFile);
    
    // 退出动作
    m_exitAct = new QAction("退出(&X)", this);
    m_exitAct->setShortcut(QKeySequence::Quit);
    m_exitAct->setStatusTip("退出应用程序");
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);
    
    // 全屏动作
    m_fullscreenAct = new QAction("全屏(&F)", this);
    m_fullscreenAct->setShortcut(QKeySequence::FullScreen);
    m_fullscreenAct->setStatusTip("切换全屏模式");
    m_fullscreenAct->setCheckable(true);
    connect(m_fullscreenAct, &QAction::triggered, this, &MainWindow::toggleFullscreen);
    
    // 关于动作
    m_aboutAct = new QAction("关于(&A)", this);
    m_aboutAct->setStatusTip("显示关于信息");
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus()
{
    // 文件菜单
    m_fileMenu = menuBar()->addMenu("文件(&F)");
    m_fileMenu->addAction(m_openFileAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);
    
    // 视图菜单
    m_viewMenu = menuBar()->addMenu("视图(&V)");
    m_viewMenu->addAction(m_fullscreenAct);
    
    // 帮助菜单
    m_helpMenu = menuBar()->addMenu("帮助(&H)");
    m_helpMenu->addAction(m_aboutAct);
}

void MainWindow::createToolBars()
{
    // 主工具栏
    m_mainToolBar = addToolBar("主工具栏");
    m_mainToolBar->addAction(m_openFileAct);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_fullscreenAct);
}

void MainWindow::createStatusBar()
{
    // 状态标签
    m_statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(m_statusLabel);
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
    
    statusBar()->showMessage("欢迎使用ComicReader", 2000);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开漫画文件",
        QString(),
        "漫画文件 (*.cbz *.cbr *.zip *.rar);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        m_currentFile = fileName;
        m_contentLabel->setText(QString("已选择文件：\n%1\n\n(漫画解析功能开发中...)").arg(fileName));
        m_statusLabel->setText(QString("已打开: %1").arg(QFileInfo(fileName).fileName()));
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于 ComicReader",
        "<h2>ComicReader v1.0.0</h2>"
        "<p>一个功能强大的现代化漫画阅读器</p>"
        "<p><b>主要特性：</b></p>"
        "<ul>"
        "<li>支持 CBZ、CBR、ZIP、RAR 格式</li>"
        "<li>智能书签管理</li>"
        "<li>高性能缓存系统</li>"
        "<li>完整的错误处理</li>"
        "</ul>"
        "<p><b>技术栈：</b> Qt 6.9.1 + C++17</p>"
        "<p>© 2024 ComicReader Team</p>"
    );
}

void MainWindow::toggleFullscreen()
{
    if (m_isFullscreen) {
        showNormal();
        m_isFullscreen = false;
        m_fullscreenAct->setText("全屏(&F)");
    } else {
        showFullScreen();
        m_isFullscreen = true;
        m_fullscreenAct->setText("退出全屏(&F)");
    }
}
