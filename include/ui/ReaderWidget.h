#ifndef READERWIDGET_H
#define READERWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

class ReaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReaderWidget(QWidget *parent = nullptr);

public slots:
    void openComic(const QString &filePath);
    void nextPage();
    void previousPage();
    void goToPage(int page);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWidth();
    void fitToHeight();

private slots:
    void onPageChanged();
    void onZoomChanged(int value);

private:
    void setupUI();
    void updatePageDisplay();
    void updateZoomDisplay();

    // UI 组件
    QVBoxLayout *mainLayout;
    QToolBar *toolBar;
    QScrollArea *scrollArea;
    QLabel *imageLabel;
    
    // 控制组件
    QPushButton *prevButton;
    QPushButton *nextButton;
    QSpinBox *pageSpinBox;
    QLabel *pageLabel;
    QSlider *zoomSlider;
    QLabel *zoomLabel;
    QPushButton *fitWidthButton;
    QPushButton *fitHeightButton;
    QPushButton *resetZoomButton;
    
    // 数据
    QStringList pages;
    int currentPage;
    double zoomFactor;
    QPixmap currentPixmap;
};

#endif // READERWIDGET_H
