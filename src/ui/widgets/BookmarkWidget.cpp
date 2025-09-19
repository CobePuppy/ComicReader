#include "BookmarkWidget.h"
#include "BookmarkManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyleOption>

BookmarkWidget::BookmarkWidget(QWidget *parent)
    : QWidget(parent)
    , m_bookmarkManager(BookmarkManager::instance())
    , m_currentFilter(0)
    , m_currentSort(0)
    , m_dateRangeFilter(false)
    , m_selectedBookmarkId("")
    , m_selectedComicPath("")
{
    setupUI();
    connectSignals();
    
    // 设置刷新定时器
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(false);
    m_refreshTimer->setInterval(30000); // 30秒
    connect(m_refreshTimer, &QTimer::timeout, this, &BookmarkWidget::onRefreshTimer);
    m_refreshTimer->start();
    
    // 初始化数据
    refreshBookmarks();
    refreshProgress();
}

void BookmarkWidget::setupUI()
{
    setObjectName("BookmarkWidget");
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 工具栏
    setupToolbar();
    mainLayout->addLayout(m_toolbarLayout);
    
    // 分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(m_splitter);
    
    // 标签页
    m_tabWidget = new QTabWidget(this);
    m_splitter->addWidget(m_tabWidget);
    
    // 设置书签和进度页面
    setupBookmarksTab();
    setupProgressTab();
    
    // 详情面板
    setupDetailsPanel();
    
    // 设置分割器比例
    m_splitter->setSizes({700, 300});
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);
}

void BookmarkWidget::setupToolbar()
{
    m_toolbarLayout = new QHBoxLayout();
    m_toolbarLayout->setContentsMargins(5, 5, 5, 5);
    
    m_addBookmarkBtn = new QPushButton("添加书签", this);
    m_addBookmarkBtn->setIcon(QIcon(":/icons/bookmark_add.png"));
    m_toolbarLayout->addWidget(m_addBookmarkBtn);
    
    m_toolbarLayout->addSpacing(10);
    
    m_exportBtn = new QPushButton("导出", this);
    m_exportBtn->setIcon(QIcon(":/icons/export.png"));
    m_toolbarLayout->addWidget(m_exportBtn);
    
    m_importBtn = new QPushButton("导入", this);
    m_importBtn->setIcon(QIcon(":/icons/import.png"));
    m_toolbarLayout->addWidget(m_importBtn);
    
    m_toolbarLayout->addSpacing(10);
    
    m_cleanupBtn = new QPushButton("清理", this);
    m_cleanupBtn->setIcon(QIcon(":/icons/cleanup.png"));
    m_toolbarLayout->addWidget(m_cleanupBtn);
    
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setIcon(QIcon(":/icons/refresh.png"));
    m_toolbarLayout->addWidget(m_refreshBtn);
    
    m_toolbarLayout->addStretch();
}

void BookmarkWidget::setupBookmarksTab()
{
    m_bookmarksTab = new QWidget();
    m_bookmarksLayout = new QVBoxLayout(m_bookmarksTab);
    
    // 搜索和过滤器
    setupSearchAndFilters();
    
    // 书签列表
    m_bookmarksTree = new QTreeWidget(this);
    m_bookmarksTree->setHeaderLabels({"标题", "漫画", "页面", "创建时间", "标签"});
    m_bookmarksTree->setRootIsDecorated(false);
    m_bookmarksTree->setAlternatingRowColors(true);
    m_bookmarksTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_bookmarksTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_bookmarksTree->setDragDropMode(QAbstractItemView::DragOnly);
    
    // 调整列宽
    m_bookmarksTree->header()->resizeSection(0, 200);
    m_bookmarksTree->header()->resizeSection(1, 180);
    m_bookmarksTree->header()->resizeSection(2, 80);
    m_bookmarksTree->header()->resizeSection(3, 120);
    m_bookmarksTree->header()->setStretchLastSection(true);
    
    m_bookmarksLayout->addWidget(m_bookmarksTree);
    
    // 统计信息
    m_bookmarkCountLabel = new QLabel("书签数量: 0", this);
    m_bookmarkCountLabel->setStyleSheet("color: #666; font-size: 12px;");
    m_bookmarksLayout->addWidget(m_bookmarkCountLabel);
    
    m_tabWidget->addTab(m_bookmarksTab, "书签管理");
}

void BookmarkWidget::setupProgressTab()
{
    m_progressTab = new QWidget();
    m_progressLayout = new QVBoxLayout(m_progressTab);
    
    // 过滤器和排序
    QHBoxLayout *progressFilterLayout = new QHBoxLayout();
    
    progressFilterLayout->addWidget(new QLabel("筛选:", this));
    m_progressFilter = new QComboBox(this);
    m_progressFilter->addItems({"全部", "进行中", "已完成", "最近阅读"});
    progressFilterLayout->addWidget(m_progressFilter);
    
    progressFilterLayout->addWidget(new QLabel("排序:", this));
    m_progressSort = new QComboBox(this);
    m_progressSort->addItems({"最近阅读", "阅读进度", "总时长", "名称"});
    progressFilterLayout->addWidget(m_progressSort);
    
    progressFilterLayout->addStretch();
    m_progressLayout->addLayout(progressFilterLayout);
    
    // 统计信息
    QHBoxLayout *statsLayout = new QHBoxLayout();
    m_statisticsLabel = new QLabel("统计信息", this);
    m_statisticsLabel->setStyleSheet("font-weight: bold;");
    statsLayout->addWidget(m_statisticsLabel);
    
    m_totalProgressBar = new QProgressBar(this);
    m_totalProgressBar->setTextVisible(true);
    m_totalProgressBar->setFormat("总体进度: %p%");
    statsLayout->addWidget(m_totalProgressBar);
    
    m_progressLayout->addLayout(statsLayout);
    
    // 进度列表
    m_progressList = new QListWidget(this);
    m_progressList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_progressList->setAlternatingRowColors(true);
    m_progressLayout->addWidget(m_progressList);
    
    // 统计标签
    m_progressCountLabel = new QLabel("漫画数量: 0", this);
    m_progressCountLabel->setStyleSheet("color: #666; font-size: 12px;");
    m_progressLayout->addWidget(m_progressCountLabel);
    
    m_tabWidget->addTab(m_progressTab, "阅读进度");
}

void BookmarkWidget::setupSearchAndFilters()
{
    // 搜索框
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("搜索:", this));
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索书签标题、描述或标签...");
    searchLayout->addWidget(m_searchEdit);
    
    m_bookmarksLayout->addLayout(searchLayout);
    
    // 过滤器
    QHBoxLayout *filterLayout = new QHBoxLayout();
    
    filterLayout->addWidget(new QLabel("筛选:", this));
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({"全部书签", "当前漫画", "快速书签", "最近添加"});
    filterLayout->addWidget(m_filterCombo);
    
    filterLayout->addWidget(new QLabel("排序:", this));
    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItems({"创建时间", "页面编号", "标题", "漫画名称"});
    filterLayout->addWidget(m_sortCombo);
    
    filterLayout->addWidget(new QLabel("标签:", this));
    m_tagFilter = new QComboBox(this);
    m_tagFilter->addItem("全部标签");
    filterLayout->addWidget(m_tagFilter);
    
    m_bookmarksLayout->addLayout(filterLayout);
    
    // 日期范围过滤
    QHBoxLayout *dateLayout = new QHBoxLayout();
    
    m_dateRangeEnabled = new QCheckBox("日期范围:", this);
    dateLayout->addWidget(m_dateRangeEnabled);
    
    m_startDateEdit = new QDateEdit(this);
    m_startDateEdit->setDate(QDate::currentDate().addDays(-30));
    m_startDateEdit->setEnabled(false);
    dateLayout->addWidget(m_startDateEdit);
    
    dateLayout->addWidget(new QLabel("到", this));
    
    m_endDateEdit = new QDateEdit(this);
    m_endDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setEnabled(false);
    dateLayout->addWidget(m_endDateEdit);
    
    dateLayout->addStretch();
    m_bookmarksLayout->addLayout(dateLayout);
}

void BookmarkWidget::setupDetailsPanel()
{
    m_detailsPanel = new QWidget(this);
    m_detailsLayout = new QVBoxLayout(m_detailsPanel);
    
    // 标题
    m_detailsTitle = new QLabel("书签详情", this);
    m_detailsTitle->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 10px;");
    m_detailsLayout->addWidget(m_detailsTitle);
    
    // 详情信息
    QGroupBox *infoGroup = new QGroupBox("基本信息", this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    
    m_detailsComic = new QLabel("漫画: 无", this);
    infoLayout->addWidget(m_detailsComic);
    
    m_detailsPage = new QLabel("页面: 无", this);
    infoLayout->addWidget(m_detailsPage);
    
    m_detailsDate = new QLabel("创建时间: 无", this);
    infoLayout->addWidget(m_detailsDate);
    
    m_detailsLayout->addWidget(infoGroup);
    
    // 描述
    QGroupBox *descGroup = new QGroupBox("描述", this);
    QVBoxLayout *descLayout = new QVBoxLayout(descGroup);
    
    m_detailsDescription = new QTextEdit(this);
    m_detailsDescription->setMaximumHeight(100);
    m_detailsDescription->setReadOnly(true);
    descLayout->addWidget(m_detailsDescription);
    
    m_detailsLayout->addWidget(descGroup);
    
    // 标签
    QGroupBox *tagsGroup = new QGroupBox("标签", this);
    QVBoxLayout *tagsLayout = new QVBoxLayout(tagsGroup);
    
    m_detailsTags = new QListWidget(this);
    m_detailsTags->setMaximumHeight(80);
    tagsLayout->addWidget(m_detailsTags);
    
    m_detailsLayout->addWidget(tagsGroup);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_editDetailsBtn = new QPushButton("编辑", this);
    buttonLayout->addWidget(m_editDetailsBtn);
    
    m_jumpToPageBtn = new QPushButton("跳转到页面", this);
    buttonLayout->addWidget(m_jumpToPageBtn);
    
    m_showInLibraryBtn = new QPushButton("在库中显示", this);
    buttonLayout->addWidget(m_showInLibraryBtn);
    
    m_detailsLayout->addLayout(buttonLayout);
    
    m_detailsLayout->addStretch();
    
    m_splitter->addWidget(m_detailsPanel);
}

void BookmarkWidget::connectSignals()
{
    // 书签管理器信号
    connect(m_bookmarkManager, &BookmarkManager::bookmarkAdded,
            this, &BookmarkWidget::onBookmarkAdded);
    connect(m_bookmarkManager, &BookmarkManager::bookmarkRemoved,
            this, &BookmarkWidget::onBookmarkRemoved);
    connect(m_bookmarkManager, &BookmarkManager::bookmarkUpdated,
            this, &BookmarkWidget::onBookmarkUpdated);
    connect(m_bookmarkManager, &BookmarkManager::readingProgressUpdated,
            this, &BookmarkWidget::onProgressUpdated);
    
    // 搜索和过滤
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BookmarkWidget::onSearchTextChanged);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookmarkWidget::onFilterChanged);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookmarkWidget::onSortChanged);
    connect(m_tagFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookmarkWidget::onTagFilterChanged);
    
    // 日期范围
    connect(m_dateRangeEnabled, &QCheckBox::toggled, [this](bool enabled) {
        m_startDateEdit->setEnabled(enabled);
        m_endDateEdit->setEnabled(enabled);
        m_dateRangeFilter = enabled;
        onDateRangeChanged();
    });
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &BookmarkWidget::onDateRangeChanged);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &BookmarkWidget::onDateRangeChanged);
    
    // 书签列表
    connect(m_bookmarksTree, &QTreeWidget::itemClicked,
            this, &BookmarkWidget::onBookmarkItemClicked);
    connect(m_bookmarksTree, &QTreeWidget::itemDoubleClicked,
            this, &BookmarkWidget::onBookmarkItemDoubleClicked);
    connect(m_bookmarksTree, &QTreeWidget::customContextMenuRequested,
            this, &BookmarkWidget::onBookmarkContextMenu);
    
    // 进度列表
    connect(m_progressList, &QListWidget::itemClicked,
            this, &BookmarkWidget::onProgressItemClicked);
    connect(m_progressList, &QListWidget::itemDoubleClicked,
            this, &BookmarkWidget::onProgressItemDoubleClicked);
    connect(m_progressList, &QListWidget::customContextMenuRequested,
            this, &BookmarkWidget::onProgressContextMenu);
    
    // 进度过滤和排序
    connect(m_progressFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookmarkWidget::updateProgressList);
    connect(m_progressSort, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookmarkWidget::updateProgressList);
    
    // 工具栏按钮
    connect(m_exportBtn, &QPushButton::clicked, this, &BookmarkWidget::onExportBookmarks);
    connect(m_importBtn, &QPushButton::clicked, this, &BookmarkWidget::onImportBookmarks);
    connect(m_cleanupBtn, &QPushButton::clicked, this, &BookmarkWidget::onCleanupBookmarks);
    connect(m_refreshBtn, &QPushButton::clicked, [this]() {
        refreshBookmarks();
        refreshProgress();
    });
    
    // 详情面板按钮
    connect(m_editDetailsBtn, &QPushButton::clicked, this, &BookmarkWidget::onEditBookmark);
    connect(m_jumpToPageBtn, &QPushButton::clicked, this, &BookmarkWidget::onJumpToPage);
    connect(m_showInLibraryBtn, &QPushButton::clicked, this, &BookmarkWidget::onShowInLibrary);
    
    // 上下文菜单
    setupContextMenus();
}

void BookmarkWidget::setupContextMenus()
{
    // 书签上下文菜单
    m_bookmarkContextMenu = new QMenu(this);
    
    QAction *jumpAction = m_bookmarkContextMenu->addAction("跳转到页面");
    jumpAction->setIcon(QIcon(":/icons/jump.png"));
    connect(jumpAction, &QAction::triggered, this, &BookmarkWidget::onJumpToPage);
    
    QAction *editAction = m_bookmarkContextMenu->addAction("编辑书签");
    editAction->setIcon(QIcon(":/icons/edit.png"));
    connect(editAction, &QAction::triggered, this, &BookmarkWidget::onEditBookmark);
    
    m_bookmarkContextMenu->addSeparator();
    
    QAction *addTagAction = m_bookmarkContextMenu->addAction("添加标签");
    addTagAction->setIcon(QIcon(":/icons/tag_add.png"));
    connect(addTagAction, &QAction::triggered, this, &BookmarkWidget::onAddTag);
    
    QAction *removeTagAction = m_bookmarkContextMenu->addAction("移除标签");
    removeTagAction->setIcon(QIcon(":/icons/tag_remove.png"));
    connect(removeTagAction, &QAction::triggered, this, &BookmarkWidget::onRemoveTag);
    
    m_bookmarkContextMenu->addSeparator();
    
    QAction *showInLibraryAction = m_bookmarkContextMenu->addAction("在库中显示");
    showInLibraryAction->setIcon(QIcon(":/icons/library.png"));
    connect(showInLibraryAction, &QAction::triggered, this, &BookmarkWidget::onShowInLibrary);
    
    m_bookmarkContextMenu->addSeparator();
    
    QAction *deleteAction = m_bookmarkContextMenu->addAction("删除书签");
    deleteAction->setIcon(QIcon(":/icons/delete.png"));
    connect(deleteAction, &QAction::triggered, this, &BookmarkWidget::onDeleteBookmark);
    
    // 进度上下文菜单
    m_progressContextMenu = new QMenu(this);
    
    QAction *openComicAction = m_progressContextMenu->addAction("打开漫画");
    openComicAction->setIcon(QIcon(":/icons/open.png"));
    connect(openComicAction, &QAction::triggered, [this]() {
        if (!m_selectedComicPath.isEmpty()) {
            emit comicSelected(m_selectedComicPath);
        }
    });
    
    QAction *showProgressInLibraryAction = m_progressContextMenu->addAction("在库中显示");
    showProgressInLibraryAction->setIcon(QIcon(":/icons/library.png"));
    connect(showProgressInLibraryAction, &QAction::triggered, this, &BookmarkWidget::onShowInLibrary);
    
    m_progressContextMenu->addSeparator();
    
    QAction *exportProgressAction = m_progressContextMenu->addAction("导出进度");
    exportProgressAction->setIcon(QIcon(":/icons/export.png"));
    connect(exportProgressAction, &QAction::triggered, this, &BookmarkWidget::onExportProgress);
}

void BookmarkWidget::setCurrentComic(const QString &comicPath)
{
    m_currentComicPath = comicPath;
    
    // 如果当前筛选是"当前漫画"，则刷新列表
    if (m_currentFilter == 1) {
        updateBookmarksList();
    }
}

void BookmarkWidget::refreshBookmarks()
{
    updateBookmarksList();
    updateTagFilter();
    updateStatistics();
}

void BookmarkWidget::refreshProgress()
{
    updateProgressList();
    updateStatistics();
}

void BookmarkWidget::updateTagFilter()
{
    QStringList allTags = m_bookmarkManager->getAllTags();
    
    QString currentTag = m_tagFilter->currentText();
    m_tagFilter->clear();
    m_tagFilter->addItem("全部标签");
    m_tagFilter->addItems(allTags);
    
    // 恢复之前选择的标签
    int index = m_tagFilter->findText(currentTag);
    if (index >= 0) {
        m_tagFilter->setCurrentIndex(index);
    }
}

void BookmarkWidget::updateBookmarksList()
{
    m_bookmarksTree->clear();
    
    QList<BookmarkInfo> bookmarks;
    
    // 根据筛选条件获取书签
    switch (m_currentFilter) {
    case 0: // 全部书签
        bookmarks = m_bookmarkManager->getAllBookmarks();
        break;
    case 1: // 当前漫画
        if (!m_currentComicPath.isEmpty()) {
            bookmarks = m_bookmarkManager->getAllBookmarksForComic(m_currentComicPath);
        }
        break;
    case 2: // 快速书签
        if (!m_currentComicPath.isEmpty()) {
            bookmarks = m_bookmarkManager->getQuickBookmarks(m_currentComicPath);
        } else {
            // 获取所有快速书签
            QList<BookmarkInfo> allBookmarks = m_bookmarkManager->getAllBookmarks();
            for (const BookmarkInfo &bookmark : allBookmarks) {
                if (bookmark.isQuickBookmark) {
                    bookmarks.append(bookmark);
                }
            }
        }
        break;
    case 3: // 最近添加
        bookmarks = m_bookmarkManager->getRecentBookmarks(50);
        break;
    }
    
    // 应用搜索过滤
    if (!m_currentSearchText.isEmpty()) {
        QList<BookmarkInfo> searchResults = m_bookmarkManager->searchBookmarks(m_currentSearchText);
        
        // 取交集
        QList<BookmarkInfo> filteredBookmarks;
        for (const BookmarkInfo &bookmark : bookmarks) {
            for (const BookmarkInfo &searchResult : searchResults) {
                if (bookmark.id == searchResult.id) {
                    filteredBookmarks.append(bookmark);
                    break;
                }
            }
        }
        bookmarks = filteredBookmarks;
    }
    
    // 应用标签过滤
    if (m_currentTag != "全部标签" && !m_currentTag.isEmpty()) {
        QList<BookmarkInfo> tagFiltered;
        for (const BookmarkInfo &bookmark : bookmarks) {
            if (bookmark.tags.contains(m_currentTag)) {
                tagFiltered.append(bookmark);
            }
        }
        bookmarks = tagFiltered;
    }
    
    // 应用日期范围过滤
    if (m_dateRangeFilter) {
        QDateTime startDateTime = QDateTime(m_startDate);
        QDateTime endDateTime = QDateTime(m_endDate).addDays(1); // 包括结束日期的整天
        
        QList<BookmarkInfo> dateFiltered;
        for (const BookmarkInfo &bookmark : bookmarks) {
            if (bookmark.dateCreated >= startDateTime && bookmark.dateCreated < endDateTime) {
                dateFiltered.append(bookmark);
            }
        }
        bookmarks = dateFiltered;
    }
    
    // 排序
    applySorting(bookmarks);
    
    // 添加到树形控件
    for (const BookmarkInfo &bookmark : bookmarks) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_bookmarksTree);
        item->setText(0, bookmark.title);
        item->setText(1, bookmark.comicTitle);
        item->setText(2, QString::number(bookmark.pageNumber + 1));
        item->setText(3, bookmark.dateCreated.toString("yyyy-MM-dd hh:mm"));
        item->setText(4, bookmark.tags.join(", "));
        item->setData(0, Qt::UserRole, bookmark.id);
        
        // 设置图标
        if (bookmark.isQuickBookmark) {
            item->setIcon(0, QIcon(":/icons/quick_bookmark.png"));
        } else {
            item->setIcon(0, QIcon(":/icons/bookmark.png"));
        }
        
        // 设置工具提示
        QString tooltip = QString("标题: %1\n漫画: %2\n页面: %3\n创建时间: %4")
                         .arg(bookmark.title)
                         .arg(bookmark.comicTitle)
                         .arg(bookmark.pageNumber + 1)
                         .arg(bookmark.dateCreated.toString("yyyy-MM-dd hh:mm:ss"));
        if (!bookmark.description.isEmpty()) {
            tooltip += "\n描述: " + bookmark.description;
        }
        if (!bookmark.tags.isEmpty()) {
            tooltip += "\n标签: " + bookmark.tags.join(", ");
        }
        item->setToolTip(0, tooltip);
    }
    
    // 更新统计
    m_bookmarkCountLabel->setText(QString("书签数量: %1").arg(bookmarks.size()));
}

void BookmarkWidget::applySorting(QList<BookmarkInfo> &bookmarks)
{
    switch (m_currentSort) {
    case 0: // 创建时间
        std::sort(bookmarks.begin(), bookmarks.end(),
                  [](const BookmarkInfo &a, const BookmarkInfo &b) {
                      return a.dateCreated > b.dateCreated;
                  });
        break;
    case 1: // 页面编号
        std::sort(bookmarks.begin(), bookmarks.end(),
                  [](const BookmarkInfo &a, const BookmarkInfo &b) {
                      if (a.comicPath == b.comicPath) {
                          return a.pageNumber < b.pageNumber;
                      }
                      return a.comicTitle < b.comicTitle;
                  });
        break;
    case 2: // 标题
        std::sort(bookmarks.begin(), bookmarks.end(),
                  [](const BookmarkInfo &a, const BookmarkInfo &b) {
                      return a.title < b.title;
                  });
        break;
    case 3: // 漫画名称
        std::sort(bookmarks.begin(), bookmarks.end(),
                  [](const BookmarkInfo &a, const BookmarkInfo &b) {
                      if (a.comicTitle == b.comicTitle) {
                          return a.pageNumber < b.pageNumber;
                      }
                      return a.comicTitle < b.comicTitle;
                  });
        break;
    }
}

void BookmarkWidget::updateProgressList()
{
    m_progressList->clear();
    
    QList<ReadingProgress> progressList;
    
    // 根据筛选条件获取进度
    switch (m_progressFilter->currentIndex()) {
    case 0: // 全部
        progressList = m_bookmarkManager->getAllReadingProgress();
        break;
    case 1: // 进行中
        progressList = m_bookmarkManager->getUnfinishedComics();
        break;
    case 2: // 已完成
        progressList = m_bookmarkManager->getCompletedComics();
        break;
    case 3: // 最近阅读
        progressList = m_bookmarkManager->getRecentReadingProgress(20);
        break;
    }
    
    // 排序
    switch (m_progressSort->currentIndex()) {
    case 0: // 最近阅读
        std::sort(progressList.begin(), progressList.end(),
                  [](const ReadingProgress &a, const ReadingProgress &b) {
                      return a.lastReadTime > b.lastReadTime;
                  });
        break;
    case 1: // 阅读进度
        std::sort(progressList.begin(), progressList.end(),
                  [](const ReadingProgress &a, const ReadingProgress &b) {
                      return a.progress > b.progress;
                  });
        break;
    case 2: // 总时长
        std::sort(progressList.begin(), progressList.end(),
                  [](const ReadingProgress &a, const ReadingProgress &b) {
                      return a.readingTime > b.readingTime;
                  });
        break;
    case 3: // 名称
        std::sort(progressList.begin(), progressList.end(),
                  [](const ReadingProgress &a, const ReadingProgress &b) {
                      QFileInfo aInfo(a.comicPath);
                      QFileInfo bInfo(b.comicPath);
                      return aInfo.baseName() < bInfo.baseName();
                  });
        break;
    }
    
    // 添加到列表
    for (const ReadingProgress &progress : progressList) {
        ProgressItemWidget *itemWidget = createProgressItem(progress);
        QListWidgetItem *item = new QListWidgetItem(m_progressList);
        item->setSizeHint(itemWidget->sizeHint());
        item->setData(Qt::UserRole, progress.comicPath);
        m_progressList->setItemWidget(item, itemWidget);
    }
    
    // 更新统计
    m_progressCountLabel->setText(QString("漫画数量: %1").arg(progressList.size()));
}

ProgressItemWidget* BookmarkWidget::createProgressItem(const ReadingProgress &progress)
{
    return new ProgressItemWidget(progress, this);
}

void BookmarkWidget::updateStatistics()
{
    QHash<QString, int> stats = m_bookmarkManager->getReadingStatistics();
    
    QString statsText = QString("总书签: %1  总漫画: %2  已完成: %3  进行中: %4")
                       .arg(stats["totalBookmarks"])
                       .arg(stats["totalComics"])
                       .arg(stats["completedComics"])
                       .arg(stats["unfinishedComics"]);
    
    m_statisticsLabel->setText(statsText);
    
    // 计算总体进度
    int total = stats["totalComics"];
    int completed = stats["completedComics"];
    int progressValue = total > 0 ? (completed * 100 / total) : 0;
    m_totalProgressBar->setValue(progressValue);
}

// 槽函数实现
void BookmarkWidget::onSearchTextChanged()
{
    m_currentSearchText = m_searchEdit->text().trimmed();
    updateBookmarksList();
}

void BookmarkWidget::onFilterChanged()
{
    m_currentFilter = m_filterCombo->currentIndex();
    updateBookmarksList();
}

void BookmarkWidget::onSortChanged()
{
    m_currentSort = m_sortCombo->currentIndex();
    updateBookmarksList();
}

void BookmarkWidget::onTagFilterChanged()
{
    m_currentTag = m_tagFilter->currentText();
    updateBookmarksList();
}

void BookmarkWidget::onDateRangeChanged()
{
    if (m_dateRangeFilter) {
        m_startDate = m_startDateEdit->date();
        m_endDate = m_endDateEdit->date();
        updateBookmarksList();
    }
}

void BookmarkWidget::onBookmarkItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    
    if (item) {
        m_selectedBookmarkId = item->data(0, Qt::UserRole).toString();
        showBookmarkDetails(m_selectedBookmarkId);
    }
}

void BookmarkWidget::onBookmarkItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    
    if (item) {
        QString bookmarkId = item->data(0, Qt::UserRole).toString();
        BookmarkInfo bookmark = m_bookmarkManager->getBookmark(bookmarkId);
        if (bookmark.isValid()) {
            emit bookmarkSelected(bookmark.comicPath, bookmark.pageNumber);
        }
    }
}

void BookmarkWidget::onBookmarkContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_bookmarksTree->itemAt(pos);
    if (item) {
        m_selectedBookmarkId = item->data(0, Qt::UserRole).toString();
        m_bookmarkContextMenu->exec(m_bookmarksTree->mapToGlobal(pos));
    }
}

void BookmarkWidget::onProgressItemClicked(QListWidgetItem *item)
{
    if (item) {
        m_selectedComicPath = item->data(Qt::UserRole).toString();
        showProgressDetails(m_selectedComicPath);
    }
}

void BookmarkWidget::onProgressItemDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        QString comicPath = item->data(Qt::UserRole).toString();
        emit comicSelected(comicPath);
    }
}

void BookmarkWidget::onProgressContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_progressList->itemAt(pos);
    if (item) {
        m_selectedComicPath = item->data(Qt::UserRole).toString();
        m_progressContextMenu->exec(m_progressList->mapToGlobal(pos));
    }
}

void BookmarkWidget::showBookmarkDetails(const QString &bookmarkId)
{
    BookmarkInfo bookmark = m_bookmarkManager->getBookmark(bookmarkId);
    if (!bookmark.isValid()) {
        return;
    }
    
    m_detailsTitle->setText("书签详情");
    m_detailsComic->setText(QString("漫画: %1").arg(bookmark.comicTitle));
    m_detailsPage->setText(QString("页面: %1").arg(bookmark.pageNumber + 1));
    m_detailsDate->setText(QString("创建时间: %1").arg(bookmark.dateCreated.toString("yyyy-MM-dd hh:mm:ss")));
    m_detailsDescription->setPlainText(bookmark.description);
    
    // 更新标签列表
    m_detailsTags->clear();
    for (const QString &tag : bookmark.tags) {
        QListWidgetItem *tagItem = new QListWidgetItem(tag, m_detailsTags);
        tagItem->setIcon(QIcon(":/icons/tag.png"));
    }
    
    // 启用按钮
    m_editDetailsBtn->setEnabled(true);
    m_jumpToPageBtn->setEnabled(true);
    m_showInLibraryBtn->setEnabled(true);
}

void BookmarkWidget::showProgressDetails(const QString &comicPath)
{
    ReadingProgress progress = m_bookmarkManager->getReadingProgress(comicPath);
    if (progress.comicPath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(comicPath);
    m_detailsTitle->setText("阅读进度");
    m_detailsComic->setText(QString("漫画: %1").arg(fileInfo.baseName()));
    m_detailsPage->setText(QString("页面: %1 / %2").arg(progress.currentPage + 1).arg(progress.totalPages));
    m_detailsDate->setText(QString("最后阅读: %1").arg(progress.lastReadTime.toString("yyyy-MM-dd hh:mm:ss")));
    
    QString description = QString("阅读进度: %1%\n累计时长: %2\n状态: %3")
                         .arg(QString::number(progress.progress * 100, 'f', 1))
                         .arg(formatReadingTime(progress.readingTime))
                         .arg(progress.isCompleted ? "已完成" : "进行中");
    m_detailsDescription->setPlainText(description);
    
    m_detailsTags->clear();
    
    // 启用相关按钮
    m_editDetailsBtn->setEnabled(false);
    m_jumpToPageBtn->setEnabled(true);
    m_showInLibraryBtn->setEnabled(true);
}

QString BookmarkWidget::formatReadingTime(qint64 seconds) const
{
    if (seconds < 60) {
        return QString("%1秒").arg(seconds);
    } else if (seconds < 3600) {
        return QString("%1分%2秒").arg(seconds / 60).arg(seconds % 60);
    } else {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        return QString("%1小时%2分").arg(hours).arg(minutes);
    }
}

// 工具栏槽函数
void BookmarkWidget::onExportBookmarks()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出书签", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/bookmarks.json",
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_bookmarkManager->exportBookmarks(fileName)) {
            QMessageBox::information(this, "导出成功", "书签已成功导出到文件。");
        } else {
            QMessageBox::warning(this, "导出失败", "导出书签时发生错误。");
        }
    }
}

void BookmarkWidget::onImportBookmarks()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入书签",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_bookmarkManager->importBookmarks(fileName)) {
            QMessageBox::information(this, "导入成功", "书签已成功导入。");
            refreshBookmarks();
        } else {
            QMessageBox::warning(this, "导入失败", "导入书签时发生错误。");
        }
    }
}

void BookmarkWidget::onExportProgress()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出阅读进度",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/reading_progress.json",
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_bookmarkManager->exportReadingProgress(fileName)) {
            QMessageBox::information(this, "导出成功", "阅读进度已成功导出到文件。");
        } else {
            QMessageBox::warning(this, "导出失败", "导出阅读进度时发生错误。");
        }
    }
}

void BookmarkWidget::onImportProgress()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入阅读进度",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_bookmarkManager->importReadingProgress(fileName)) {
            QMessageBox::information(this, "导入成功", "阅读进度已成功导入。");
            refreshProgress();
        } else {
            QMessageBox::warning(this, "导入失败", "导入阅读进度时发生错误。");
        }
    }
}

void BookmarkWidget::onCleanupBookmarks()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "清理书签",
        "是否要清理无效的书签和过期的阅读记录？",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_bookmarkManager->cleanupOrphanedBookmarks();
        m_bookmarkManager->cleanupOldProgress(30); // 保留30天内的记录
        QMessageBox::information(this, "清理完成", "已清理无效的书签和过期的阅读记录。");
        refreshBookmarks();
        refreshProgress();
    }
}

// 上下文菜单槽函数
void BookmarkWidget::onAddTag()
{
    if (m_selectedBookmarkId.isEmpty()) return;
    
    bool ok;
    QString tag = QInputDialog::getText(this, "添加标签", "输入标签名称:", QLineEdit::Normal, "", &ok);
    
    if (ok && !tag.isEmpty()) {
        if (m_bookmarkManager->addTagToBookmark(m_selectedBookmarkId, tag)) {
            refreshBookmarks();
            showBookmarkDetails(m_selectedBookmarkId);
        }
    }
}

void BookmarkWidget::onRemoveTag()
{
    if (m_selectedBookmarkId.isEmpty()) return;
    
    QStringList tags = m_bookmarkManager->getTagsForBookmark(m_selectedBookmarkId);
    if (tags.isEmpty()) {
        QMessageBox::information(this, "提示", "该书签没有标签。");
        return;
    }
    
    bool ok;
    QString tag = QInputDialog::getItem(this, "移除标签", "选择要移除的标签:", tags, 0, false, &ok);
    
    if (ok && !tag.isEmpty()) {
        if (m_bookmarkManager->removeTagFromBookmark(m_selectedBookmarkId, tag)) {
            refreshBookmarks();
            showBookmarkDetails(m_selectedBookmarkId);
        }
    }
}

void BookmarkWidget::onEditBookmark()
{
    if (m_selectedBookmarkId.isEmpty()) return;
    
    // 这里应该打开书签编辑对话框
    // TODO: 实现书签编辑对话框
    emit editBookmarkRequested(m_selectedBookmarkId);
}

void BookmarkWidget::onDeleteBookmark()
{
    if (m_selectedBookmarkId.isEmpty()) return;
    
    BookmarkInfo bookmark = m_bookmarkManager->getBookmark(m_selectedBookmarkId);
    if (!bookmark.isValid()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "删除书签",
        QString("确定要删除书签 \"%1\" 吗？").arg(bookmark.title),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_bookmarkManager->removeBookmark(m_selectedBookmarkId)) {
            m_selectedBookmarkId.clear();
            refreshBookmarks();
        }
    }
}

void BookmarkWidget::onJumpToPage()
{
    if (!m_selectedBookmarkId.isEmpty()) {
        BookmarkInfo bookmark = m_bookmarkManager->getBookmark(m_selectedBookmarkId);
        if (bookmark.isValid()) {
            emit bookmarkSelected(bookmark.comicPath, bookmark.pageNumber);
        }
    } else if (!m_selectedComicPath.isEmpty()) {
        ReadingProgress progress = m_bookmarkManager->getReadingProgress(m_selectedComicPath);
        if (!progress.comicPath.isEmpty()) {
            emit bookmarkSelected(progress.comicPath, progress.currentPage);
        }
    }
}

void BookmarkWidget::onShowInLibrary()
{
    QString comicPath;
    
    if (!m_selectedBookmarkId.isEmpty()) {
        BookmarkInfo bookmark = m_bookmarkManager->getBookmark(m_selectedBookmarkId);
        if (bookmark.isValid()) {
            comicPath = bookmark.comicPath;
        }
    } else if (!m_selectedComicPath.isEmpty()) {
        comicPath = m_selectedComicPath;
    }
    
    if (!comicPath.isEmpty()) {
        emit comicSelected(comicPath);
    }
}

void BookmarkWidget::onRefreshTimer()
{
    // 定期刷新统计信息
    updateStatistics();
}

// BookmarkManager信号槽
void BookmarkWidget::onBookmarkAdded(const QString &bookmarkId)
{
    Q_UNUSED(bookmarkId)
    refreshBookmarks();
}

void BookmarkWidget::onBookmarkRemoved(const QString &bookmarkId)
{
    Q_UNUSED(bookmarkId)
    refreshBookmarks();
    if (m_selectedBookmarkId == bookmarkId) {
        m_selectedBookmarkId.clear();
        // 清空详情面板
        m_detailsTitle->setText("书签详情");
        m_detailsComic->setText("漫画: 无");
        m_detailsPage->setText("页面: 无");
        m_detailsDate->setText("创建时间: 无");
        m_detailsDescription->clear();
        m_detailsTags->clear();
        m_editDetailsBtn->setEnabled(false);
        m_jumpToPageBtn->setEnabled(false);
        m_showInLibraryBtn->setEnabled(false);
    }
}

void BookmarkWidget::onBookmarkUpdated(const QString &bookmarkId)
{
    refreshBookmarks();
    if (m_selectedBookmarkId == bookmarkId) {
        showBookmarkDetails(bookmarkId);
    }
}

void BookmarkWidget::onProgressUpdated(const QString &comicPath)
{
    Q_UNUSED(comicPath)
    refreshProgress();
}

// 自定义进度项目组件实现
ProgressItemWidget::ProgressItemWidget(const ReadingProgress &progress, QWidget *parent)
    : QWidget(parent), m_progress(progress), m_comicPath(progress.comicPath)
{
    setupUI();
    updateProgress(progress);
}

void ProgressItemWidget::setupUI()
{
    setFixedHeight(80);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 5);
    
    // 左侧信息
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    leftLayout->addWidget(m_titleLabel);
    
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #666;");
    leftLayout->addWidget(m_statusLabel);
    
    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet("color: #999; font-size: 12px;");
    leftLayout->addWidget(m_timeLabel);
    
    mainLayout->addLayout(leftLayout, 1);
    
    // 右侧进度
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    m_pagesLabel = new QLabel(this);
    m_pagesLabel->setAlignment(Qt::AlignRight);
    m_pagesLabel->setStyleSheet("font-size: 12px; color: #666;");
    rightLayout->addWidget(m_pagesLabel);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setTextVisible(true);
    m_progressBar->setMaximumHeight(20);
    rightLayout->addWidget(m_progressBar);
    
    rightLayout->addStretch();
    
    mainLayout->addLayout(rightLayout);
}

void ProgressItemWidget::updateProgress(const ReadingProgress &progress)
{
    m_progress = progress;
    
    QFileInfo fileInfo(progress.comicPath);
    m_titleLabel->setText(fileInfo.baseName());
    
    QString status = progress.isCompleted ? "已完成" : "进行中";
    m_statusLabel->setText(QString("状态: %1").arg(status));
    
    m_timeLabel->setText(QString("最后阅读: %1  累计: %2")
                        .arg(progress.lastReadTime.toString("MM-dd hh:mm"))
                        .arg(formatTime(progress.readingTime)));
    
    m_pagesLabel->setText(QString("%1 / %2 页")
                         .arg(progress.currentPage + 1)
                         .arg(progress.totalPages));
    
    updateProgressBar();
}

void ProgressItemWidget::updateProgressBar()
{
    int progressValue = static_cast<int>(m_progress.progress * 100);
    m_progressBar->setValue(progressValue);
    m_progressBar->setFormat(QString("%1%").arg(progressValue));
    
    // 根据进度设置颜色
    QString style;
    if (m_progress.isCompleted) {
        style = "QProgressBar::chunk { background-color: #4CAF50; }"; // 绿色
    } else if (m_progress.progress > 0.7) {
        style = "QProgressBar::chunk { background-color: #FF9800; }"; // 橙色
    } else {
        style = "QProgressBar::chunk { background-color: #2196F3; }"; // 蓝色
    }
    m_progressBar->setStyleSheet(style);
}

QString ProgressItemWidget::formatTime(qint64 seconds) const
{
    if (seconds < 60) {
        return QString("%1秒").arg(seconds);
    } else if (seconds < 3600) {
        return QString("%1分").arg(seconds / 60);
    } else {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        return QString("%1h%2m").arg(hours).arg(minutes);
    }
}

void ProgressItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void ProgressItemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void ProgressItemWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}