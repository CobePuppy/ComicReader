#include "../../include/ui/DownloadManagerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>

DownloadManagerWidget::DownloadManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void DownloadManagerWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    createControlPanel();
    createDownloadList();
    createStatusPanel();
    
    // 初始状态
    onDownloadItemSelectionChanged();
}

void DownloadManagerWidget::createControlPanel()
{
    controlGroup = new QGroupBox("下载控制", this);
    controlLayout = new QHBoxLayout(controlGroup);
    
    urlEdit = new QLineEdit(this);
    urlEdit->setPlaceholderText("输入下载URL...");
    
    addUrlButton = new QPushButton("添加", this);
    startButton = new QPushButton("开始", this);
    pauseButton = new QPushButton("暂停", this);
    stopButton = new QPushButton("停止", this);
    removeButton = new QPushButton("删除", this);
    clearButton = new QPushButton("清除已完成", this);
    openFolderButton = new QPushButton("打开文件夹", this);
    
    controlLayout->addWidget(urlEdit, 1);
    controlLayout->addWidget(addUrlButton);
    controlLayout->addSeparator();
    controlLayout->addWidget(startButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(stopButton);
    controlLayout->addWidget(removeButton);
    controlLayout->addSeparator();
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(openFolderButton);
    
    mainLayout->addWidget(controlGroup);
    
    // 连接信号
    connect(addUrlButton, &QPushButton::clicked, this, &DownloadManagerWidget::onAddUrlButtonClicked);
    connect(startButton, &QPushButton::clicked, this, &DownloadManagerWidget::startDownload);
    connect(pauseButton, &QPushButton::clicked, this, &DownloadManagerWidget::pauseDownload);
    connect(stopButton, &QPushButton::clicked, this, &DownloadManagerWidget::stopDownload);
    connect(removeButton, &QPushButton::clicked, this, &DownloadManagerWidget::removeDownload);
    connect(clearButton, &QPushButton::clicked, this, &DownloadManagerWidget::clearCompleted);
    connect(openFolderButton, &QPushButton::clicked, this, &DownloadManagerWidget::openDownloadFolder);
}

void DownloadManagerWidget::createDownloadList()
{
    downloadList = new QTreeWidget(this);
    downloadList->setHeaderLabels(QStringList() << "标题" << "URL" << "状态" << "进度" << "大小" << "速度");
    downloadList->setSelectionMode(QAbstractItemView::SingleSelection);
    downloadList->setRootIsDecorated(false);
    downloadList->setAlternatingRowColors(true);
    
    // 设置列宽
    downloadList->header()->resizeSection(0, 200); // 标题
    downloadList->header()->resizeSection(1, 300); // URL
    downloadList->header()->resizeSection(2, 80);  // 状态
    downloadList->header()->resizeSection(3, 100); // 进度
    downloadList->header()->resizeSection(4, 80);  // 大小
    downloadList->header()->resizeSection(5, 80);  // 速度
    
    mainLayout->addWidget(downloadList);
    
    connect(downloadList, &QTreeWidget::itemSelectionChanged, 
            this, &DownloadManagerWidget::onDownloadItemSelectionChanged);
}

void DownloadManagerWidget::createStatusPanel()
{
    statusGroup = new QGroupBox("下载统计", this);
    statusLayout = new QHBoxLayout(statusGroup);
    
    totalDownloadsLabel = new QLabel("总计: 0", this);
    activeDownloadsLabel = new QLabel("活动: 0", this);
    completedDownloadsLabel = new QLabel("完成: 0", this);
    downloadSpeedLabel = new QLabel("速度: 0 KB/s", this);
    
    overallProgressBar = new QProgressBar(this);
    overallProgressBar->setMinimumWidth(200);
    
    statusLayout->addWidget(totalDownloadsLabel);
    statusLayout->addWidget(activeDownloadsLabel);
    statusLayout->addWidget(completedDownloadsLabel);
    statusLayout->addWidget(downloadSpeedLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(new QLabel("总进度:", this));
    statusLayout->addWidget(overallProgressBar);
    
    mainLayout->addWidget(statusGroup);
}

void DownloadManagerWidget::addDownload(const QString &url, const QString &title)
{
    if (url.isEmpty()) {
        return;
    }
    
    // 创建下载项
    DownloadItem *item = new DownloadItem(url, title);
    downloadItems.append(item);
    
    // 添加到列表
    QTreeWidgetItem *listItem = new QTreeWidgetItem(downloadList);
    listItem->setText(0, item->title);
    listItem->setText(1, item->url);
    listItem->setText(2, "等待中");
    listItem->setText(3, "0%");
    listItem->setText(4, "-");
    listItem->setText(5, "-");
    
    updateDownloadStats();
}

void DownloadManagerWidget::startDownload()
{
    QTreeWidgetItem *selectedItem = downloadList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "提示", "请选择要开始的下载项。");
        return;
    }
    
    // TODO: 实现下载开始逻辑
    selectedItem->setText(2, "下载中");
    QMessageBox::information(this, "提示", "下载已开始（功能待实现）。");
}

void DownloadManagerWidget::pauseDownload()
{
    QTreeWidgetItem *selectedItem = downloadList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "提示", "请选择要暂停的下载项。");
        return;
    }
    
    // TODO: 实现下载暂停逻辑
    selectedItem->setText(2, "已暂停");
    QMessageBox::information(this, "提示", "下载已暂停（功能待实现）。");
}

void DownloadManagerWidget::stopDownload()
{
    QTreeWidgetItem *selectedItem = downloadList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "提示", "请选择要停止的下载项。");
        return;
    }
    
    // TODO: 实现下载停止逻辑
    selectedItem->setText(2, "已停止");
    QMessageBox::information(this, "提示", "下载已停止（功能待实现）。");
}

void DownloadManagerWidget::removeDownload()
{
    QTreeWidgetItem *selectedItem = downloadList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "提示", "请选择要删除的下载项。");
        return;
    }
    
    int ret = QMessageBox::question(this, "确认", "确定要删除选中的下载项吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        int index = downloadList->indexOfTopLevelItem(selectedItem);
        if (index >= 0 && index < downloadItems.size()) {
            delete downloadItems.takeAt(index);
        }
        delete selectedItem;
        updateDownloadStats();
    }
}

void DownloadManagerWidget::clearCompleted()
{
    // TODO: 实现清除已完成下载项的逻辑
    QMessageBox::information(this, "提示", "清除已完成项功能待实现。");
}

void DownloadManagerWidget::openDownloadFolder()
{
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDesktopServices::openUrl(QUrl::fromLocalFile(downloadPath));
}

void DownloadManagerWidget::onDownloadItemSelectionChanged()
{
    bool hasSelection = downloadList->currentItem() != nullptr;
    
    startButton->setEnabled(hasSelection);
    pauseButton->setEnabled(hasSelection);
    stopButton->setEnabled(hasSelection);
    removeButton->setEnabled(hasSelection);
}

void DownloadManagerWidget::onAddUrlButtonClicked()
{
    QString url = urlEdit->text().trimmed();
    if (!url.isEmpty()) {
        addDownload(url);
        urlEdit->clear();
    } else {
        QMessageBox::information(this, "提示", "请输入有效的URL。");
    }
}

void DownloadManagerWidget::updateDownloadStats()
{
    int total = downloadItems.size();
    int active = 0;
    int completed = 0;
    
    // TODO: 计算活动和完成的下载数量
    for (const DownloadItem *item : downloadItems) {
        if (item->status == DownloadItem::Downloading) {
            active++;
        } else if (item->status == DownloadItem::Completed) {
            completed++;
        }
    }
    
    totalDownloadsLabel->setText(QString("总计: %1").arg(total));
    activeDownloadsLabel->setText(QString("活动: %1").arg(active));
    completedDownloadsLabel->setText(QString("完成: %1").arg(completed));
    
    // TODO: 计算总体进度
    overallProgressBar->setValue(total > 0 ? (completed * 100 / total) : 0);
}
