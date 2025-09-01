#ifndef DOWNLOADMANAGERWIDGET_H
#define DOWNLOADMANAGERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>

class DownloadItem;

class DownloadManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadManagerWidget(QWidget *parent = nullptr);

public slots:
    void addDownload(const QString &url, const QString &title = QString());
    void startDownload();
    void pauseDownload();
    void stopDownload();
    void removeDownload();
    void clearCompleted();
    void openDownloadFolder();

private slots:
    void onDownloadItemSelectionChanged();
    void onAddUrlButtonClicked();
    void updateDownloadStats();

private:
    void setupUI();
    void createControlPanel();
    void createDownloadList();
    void createStatusPanel();

    // UI 组件
    QVBoxLayout *mainLayout;
    
    // 控制面板
    QGroupBox *controlGroup;
    QHBoxLayout *controlLayout;
    QLineEdit *urlEdit;
    QPushButton *addUrlButton;
    QPushButton *startButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QPushButton *removeButton;
    QPushButton *clearButton;
    QPushButton *openFolderButton;
    
    // 下载列表
    QTreeWidget *downloadList;
    
    // 状态面板
    QGroupBox *statusGroup;
    QHBoxLayout *statusLayout;
    QLabel *totalDownloadsLabel;
    QLabel *activeDownloadsLabel;
    QLabel *completedDownloadsLabel;
    QLabel *downloadSpeedLabel;
    QProgressBar *overallProgressBar;
    
    // 数据
    QList<DownloadItem*> downloadItems;
};

// 下载项数据结构
class DownloadItem
{
public:
    enum Status {
        Pending,
        Downloading,
        Paused,
        Completed,
        Error
    };
    
    QString url;
    QString title;
    QString fileName;
    QString savePath;
    Status status;
    qint64 totalSize;
    qint64 downloadedSize;
    double progress;
    QString errorMessage;
    
    DownloadItem(const QString &url, const QString &title = QString())
        : url(url), title(title), status(Pending), totalSize(0), downloadedSize(0), progress(0.0)
    {
        if (title.isEmpty()) {
            this->title = url.split('/').last();
        }
    }
};

#endif // DOWNLOADMANAGERWIDGET_H
