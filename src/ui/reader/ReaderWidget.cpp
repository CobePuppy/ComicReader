#include "../../include/ui/ReaderWidget.h"
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

ReaderWidget::ReaderWidget(QWidget *parent)
    : QWidget(parent)
    , currentPage(0)
    , zoomFactor(1.0)
{
    setupUI();
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
    // TODO: 实现漫画文件解析逻辑
    // 这里应该根据文件类型(.cbz, .cbr等)解析文件内容
    Q_UNUSED(filePath)
    
    // 模拟数据
    pages.clear();
    pages << "page1.jpg" << "page2.jpg" << "page3.jpg";
    
    if (!pages.isEmpty()) {
        pageSpinBox->setMaximum(pages.size());
        pageLabel->setText(QString("/ %1").arg(pages.size()));
        currentPage = 0;
        updatePageDisplay();
    }
}

void ReaderWidget::nextPage()
{
    if (currentPage < pages.size() - 1) {
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
    if (page >= 1 && page <= pages.size()) {
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
    // TODO: 实现适应宽度逻辑
}

void ReaderWidget::fitToHeight()
{
    // TODO: 实现适应高度逻辑
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
    if (currentPage >= 0 && currentPage < pages.size()) {
        // TODO: 加载并显示当前页面图像
        imageLabel->setText(QString("当前页面: %1").arg(pages[currentPage]));
        onPageChanged();
    }
}

void ReaderWidget::updateZoomDisplay()
{
    if (!currentPixmap.isNull()) {
        QPixmap scaledPixmap = currentPixmap.scaled(
            currentPixmap.size() * zoomFactor,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        imageLabel->setPixmap(scaledPixmap);
    }
}
