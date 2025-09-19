#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QStackedWidget>
#include <QSplitter>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

class ReaderWidget;
class BrowserWidget;
class DownloadManagerWidget;
class SettingsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void openUrl();
    void showDownloadManager();
    void showSettings();
    void about();

private:
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void createActions();

    // 菜单和工具栏
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;
    QToolBar *mainToolBar;
    
    // 动作
    QAction *openFileAct;
    QAction *openUrlAct;
    QAction *exitAct;
    QAction *downloadManagerAct;
    QAction *settingsAct;
    QAction *aboutAct;
    
    // 中央部件
    QStackedWidget *centralStack;
    QSplitter *mainSplitter;
    
    // 功能组件
    ReaderWidget *readerWidget;
    BrowserWidget *browserWidget;
    DownloadManagerWidget *downloadManagerWidget;
    SettingsWidget *settingsWidget;
};

#endif // MAINWINDOW_H
