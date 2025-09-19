#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QAction>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void showAbout();
    void toggleFullscreen();

private:
    void setupUI();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    // 中央组件
    QWidget *m_centralWidget;
    QLabel *m_contentLabel;
    
    // 菜单
    QMenu *m_fileMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;
    
    // 工具栏
    QToolBar *m_mainToolBar;
    
    // 状态栏
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    
    // 动作
    QAction *m_openFileAct;
    QAction *m_exitAct;
    QAction *m_fullscreenAct;
    QAction *m_aboutAct;
    
    // 状态
    QString m_currentFile;
    bool m_isFullscreen;
};

#endif // MAINWINDOW_H