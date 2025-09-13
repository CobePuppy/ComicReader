#include "../../include/ui/ReaderWidget.h"
#include "../../include/core/ComicParser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QPixmap>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>

ReaderWidget::ReaderWidget(QWidget *parent)
    : QWidget(parent)
    , comicParser(new ComicParser(this))
    , currentPage(0)
    , zoomFactor(1.0)
{
    setupUI();
    
    // 连接解析器信号
    connect(comicParser, &ComicParser::parseCompleted, this, [this](bool success) {
        if (success) {
            const auto& info = comicParser->getComicInfo();
            pageSpinBox->setMaximum(info.pageCount);
            pageLabel->setText(QString("/ %1").arg(info.pageCount));
            if (info.pageCount > 0) {
                currentPage = 0;
                updatePageDisplay();
            }
        } else {
            QMessageBox::warning(this, "错误", "无法解析漫画文件");
        }
    });
    
    connect(comicParser, &ComicParser::error, this, [this](const QString& error) {
        QMessageBox::critical(this, "解析错误", error);
    });
}

void ReaderWidget::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    // 创建工具栏
    toolBar = new QToolBar(this);
    
    prevButton = new QPushButton("上一页", this);
    nextButton = new QPushButton("下一页", this);
    pageSpinBox = new QSpinBox(this);
    pageLabel = new QLabel("/ 0", this);
    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomLabel = new QLabel("100%", this);
    fitWidthButton = new QPushButton("适应宽度", this);
    fitHeightButton = new QPushButton("适应高度", this);
    resetZoomButton = new QPushButton("重置缩放", this);
    
    // 设置组件属性
    pageSpinBox->setMinimum(1);
    zoomSlider->setRange(10, 500);
    zoomSlider->setValue(100);
    zoomSlider->setToolTip("缩放比例");
    
    // 添加到工具栏
    toolBar->addWidget(prevButton);
    toolBar->addWidget(nextButton);
    toolBar->addSeparator();
    toolBar->addWidget(new QLabel("页码:"));
    toolBar->addWidget(pageSpinBox);
    toolBar->addWidget(pageLabel);
    toolBar->addSeparator();
    toolBar->addWidget(new QLabel("缩放:"));
    toolBar->addWidget(zoomSlider);
    toolBar->addWidget(zoomLabel);
    toolBar->addSeparator();
    toolBar->addWidget(fitWidthButton);
    toolBar->addWidget(fitHeightButton);
    toolBar->addWidget(resetZoomButton);
    
    // 创建滚动区域
    scrollArea = new QScrollArea(this);
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background-color: white;");
    imageLabel->setText("请打开漫画文件");
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    
    // 布局
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(scrollArea);
    
    // 连接信号
    connect(prevButton, &QPushButton::clicked, this, &ReaderWidget::previousPage);
    connect(nextButton, &QPushButton::clicked, this, &ReaderWidget::nextPage);
    connect(pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ReaderWidget::goToPage);
    connect(zoomSlider, &QSlider::valueChanged, this, &ReaderWidget::onZoomChanged);
    connect(fitWidthButton, &QPushButton::clicked, this, &ReaderWidget::fitToWidth);
    connect(fitHeightButton, &QPushButton::clicked, this, &ReaderWidget::fitToHeight);
    connect(resetZoomButton, &QPushButton::clicked, this, &ReaderWidget::resetZoom);
}

void ReaderWidget::openComic(const QString &filePath)
{
    // 显示进度对话框
    QProgressDialog *progressDialog = new QProgressDialog("正在解析漫画文件...", "取消", 0, 100, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->show();
    
    connect(comicParser, &ComicParser::parseProgress, progressDialog, [progressDialog](int current, int total) {
        if (total > 0) {
            progressDialog->setValue((current * 100) / total);
        }
    });
    
    connect(comicParser, &ComicParser::parseCompleted, progressDialog, [progressDialog](bool success) {
        progressDialog->setValue(100);
        progressDialog->close();
        progressDialog->deleteLater();
    });
    
    // 解析漫画文件
    comicParser->parseComic(filePath);
}

void ReaderWidget::nextPage()
{
    const auto& info = comicParser->getComicInfo();
    if (currentPage < info.pageCount - 1) {
        currentPage++;
        updatePageDisplay();
    }
}

void ReaderWidget::previousPage()
{
    if (currentPage > 0) {
        currentPage--;
        updatePageDisplay();
    }
}

void ReaderWidget::goToPage(int page)
{
    const auto& info = comicParser->getComicInfo();
    if (page >= 1 && page <= info.pageCount) {
        currentPage = page - 1;
        updatePageDisplay();
    }
}

void ReaderWidget::zoomIn()
{
    int value = zoomSlider->value();
    zoomSlider->setValue(qMin(500, value + 25));
}

void ReaderWidget::zoomOut()
{
    int value = zoomSlider->value();
    zoomSlider->setValue(qMax(10, value - 25));
}

void ReaderWidget::resetZoom()
{
    zoomSlider->setValue(100);
}

void ReaderWidget::fitToWidth()
{
    if (!originalPixmap.isNull()) {
        int targetWidth = scrollArea->viewport()->width() - 20; // 留一些边距
        if (targetWidth > 0) {
            double ratio = double(targetWidth) / originalPixmap.width();
            int newValue = qRound(ratio * 100);
            zoomSlider->setValue(qBound(10, newValue, 500));
        }
    }
}

void ReaderWidget::fitToHeight()
{
    if (!originalPixmap.isNull()) {
        int targetHeight = scrollArea->viewport()->height() - 20; // 留一些边距
        if (targetHeight > 0) {
            double ratio = double(targetHeight) / originalPixmap.height();
            int newValue = qRound(ratio * 100);
            zoomSlider->setValue(qBound(10, newValue, 500));
        }
    }
}

void ReaderWidget::onPageChanged()
{
    pageSpinBox->setValue(currentPage + 1);
}

void ReaderWidget::onZoomChanged(int value)
{
    zoomFactor = value / 100.0;
    zoomLabel->setText(QString("%1%").arg(value));
    updateZoomDisplay();
}

void ReaderWidget::updatePageDisplay()
{
    const auto& info = comicParser->getComicInfo();
    if (currentPage >= 0 && currentPage < info.pageCount) {
        // 显示加载提示
        imageLabel->setText("加载中...");
        QApplication::processEvents();
        
        // 加载当前页面图像
        QPixmap pixmap = comicParser->getPagePixmap(currentPage);
        if (!pixmap.isNull()) {
            originalPixmap = pixmap;
            currentPixmap = pixmap;
            updateZoomDisplay();
        } else {
            imageLabel->setText(QString("无法加载页面 %1").arg(currentPage + 1));
        }
        
        onPageChanged();
    }
}

void ReaderWidget::updateZoomDisplay()
{
    if (!originalPixmap.isNull()) {
        if (qAbs(zoomFactor - 1.0) < 0.01) {
            // 原始大小，直接使用原图
            currentPixmap = originalPixmap;
        } else {
            // 缩放图像
            QSize newSize = originalPixmap.size() * zoomFactor;
            currentPixmap = originalPixmap.scaled(
                newSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
        }
        
        imageLabel->setPixmap(currentPixmap);
        imageLabel->resize(currentPixmap.size());
    }
}
