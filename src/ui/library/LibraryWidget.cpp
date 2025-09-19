#include "../../../include/ui/library/LibraryWidget.h"
#include "../../../include/core/ComicParser.h"
#include "../../../include/core/ConfigManager.h"
#include "../../../include/core/cache/CacheManager.h"
#include <QApplication>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QConcurrentRun>
#include <QFutureWatcher>
#include <QDebug>

LibraryWidget::LibraryWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolbarLayout(nullptr)
    , m_splitter(nullptr)
    , m_searchEdit(nullptr)
    , m_genreFilter(nullptr)
    , m_sortCombo(nullptr)
    , m_viewModeCombo(nullptr)
    , m_addComicBtn(nullptr)
    , m_addFolderBtn(nullptr)
    , m_removeBtn(nullptr)
    , m_refreshBtn(nullptr)
    , m_favoritesBtn(nullptr)
    , m_viewContainer(nullptr)
    , m_listWidget(nullptr)
    , m_treeWidget(nullptr)
    , m_gridWidget(nullptr)
    , m_statusLayout(nullptr)
    , m_statusLabel(nullptr)
    , m_countLabel(nullptr)
    , m_scanProgress(nullptr)
    , m_contextMenu(nullptr)
    , m_openAction(nullptr)
    , m_favoriteAction(nullptr)
    , m_removeAction(nullptr)
    , m_propertiesAction(nullptr)
    , m_viewMode(ListView)
    , m_sortBy(SortByName)
    , m_sortOrder(Qt::AscendingOrder)
    , m_favoritesOnly(false)
    , m_thumbnailSize(128, 192)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_scanTimer(new QTimer(this))
    , m_isScanning(false)
    , m_scanCurrentItem(0)
    , m_scanTotal(0)
    , m_searchTimer(new QTimer(this))
    , m_maxThumbnailCacheSize(100)
{
    setupUI();
    connectSignals();
    loadLibraryData();
    
    // 配置搜索定时器
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300); // 300ms延迟搜索
    
    // 配置扫描定时器
    m_scanTimer->setSingleShot(true);
    m_scanTimer->setInterval(1000); // 1秒延迟扫描
    
    // 启用拖放
    setAcceptDrops(true);
}

LibraryWidget::~LibraryWidget()
{
    saveLibraryData();
}

void LibraryWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    
    setupToolbar();
    setupViewArea();
    setupStatusBar();
    
    updateViewWidget();
}

void LibraryWidget::setupToolbar()
{
    m_toolbarLayout = new QHBoxLayout();
    
    // 搜索框
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索漫画...");
    m_searchEdit->setMaximumWidth(200);
    
    // 过滤器
    m_genreFilter = new QComboBox();
    m_genreFilter->addItem("所有类型");
    m_genreFilter->setMaximumWidth(120);
    
    // 排序选项
    m_sortCombo = new QComboBox();
    m_sortCombo->addItem("按名称", SortByName);
    m_sortCombo->addItem("按添加时间", SortByDateAdded);
    m_sortCombo->addItem("按最近阅读", SortByLastRead);
    m_sortCombo->addItem("按文件大小", SortBySize);
    m_sortCombo->addItem("按评分", SortByRating);
    m_sortCombo->setMaximumWidth(120);
    
    // 视图模式
    m_viewModeCombo = new QComboBox();
    m_viewModeCombo->addItem("列表视图", ListView);
    m_viewModeCombo->addItem("网格视图", GridView);
    m_viewModeCombo->addItem("详细视图", DetailsView);
    m_viewModeCombo->setMaximumWidth(100);
    
    // 按钮
    m_addComicBtn = new QPushButton("添加漫画");
    m_addFolderBtn = new QPushButton("添加文件夹");
    m_removeBtn = new QPushButton("移除");
    m_refreshBtn = new QPushButton("刷新");
    m_favoritesBtn = new QPushButton("收藏");
    m_favoritesBtn->setCheckable(true);
    
    // 设置图标（当图标文件存在时）
    // m_addComicBtn->setIcon(QIcon(":/icons/toolbar/open"));
    // m_addFolderBtn->setIcon(QIcon(":/icons/library/folder"));
    // m_removeBtn->setIcon(QIcon(":/icons/toolbar/close"));
    // m_refreshBtn->setIcon(QIcon(":/icons/toolbar/refresh"));
    // m_favoritesBtn->setIcon(QIcon(":/icons/library/favorite"));
    
    // 布局
    m_toolbarLayout->addWidget(new QLabel("搜索:"));
    m_toolbarLayout->addWidget(m_searchEdit);
    m_toolbarLayout->addWidget(new QLabel("类型:"));
    m_toolbarLayout->addWidget(m_genreFilter);
    m_toolbarLayout->addWidget(new QLabel("排序:"));
    m_toolbarLayout->addWidget(m_sortCombo);
    m_toolbarLayout->addWidget(new QLabel("视图:"));
    m_toolbarLayout->addWidget(m_viewModeCombo);
    m_toolbarLayout->addStretch();
    m_toolbarLayout->addWidget(m_favoritesBtn);
    m_toolbarLayout->addWidget(m_addComicBtn);
    m_toolbarLayout->addWidget(m_addFolderBtn);
    m_toolbarLayout->addWidget(m_removeBtn);
    m_toolbarLayout->addWidget(m_refreshBtn);
    
    m_mainLayout->addLayout(m_toolbarLayout);
}

void LibraryWidget::setupViewArea()
{
    m_splitter = new QSplitter(Qt::Horizontal);
    
    // 创建视图容器
    m_viewContainer = new QWidget();
    QVBoxLayout *containerLayout = new QVBoxLayout(m_viewContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    
    // 列表视图
    m_listWidget = new QListWidget();
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    containerLayout->addWidget(m_listWidget);
    
    // 树视图（详细视图）
    m_treeWidget = new QTreeWidget();
    m_treeWidget->setHeaderLabels(QStringList() << "名称" << "类型" << "作者" << "大小" << "添加时间" << "最近阅读");
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeWidget->hide();
    containerLayout->addWidget(m_treeWidget);
    
    // 网格视图（将在后续实现）
    m_gridWidget = new QWidget();
    m_gridWidget->hide();
    containerLayout->addWidget(m_gridWidget);
    
    m_splitter->addWidget(m_viewContainer);
    m_mainLayout->addWidget(m_splitter);
}

void LibraryWidget::setupStatusBar()
{
    m_statusLayout = new QHBoxLayout();
    
    m_statusLabel = new QLabel("就绪");
    m_countLabel = new QLabel("共 0 本漫画");
    m_scanProgress = new QProgressBar();
    m_scanProgress->setVisible(false);
    m_scanProgress->setMaximumWidth(200);
    
    m_statusLayout->addWidget(m_statusLabel);
    m_statusLayout->addStretch();
    m_statusLayout->addWidget(m_scanProgress);
    m_statusLayout->addWidget(m_countLabel);
    
    m_mainLayout->addLayout(m_statusLayout);
}

void LibraryWidget::connectSignals()
{
    // 搜索和过滤
    connect(m_searchEdit, &QLineEdit::textChanged, this, &LibraryWidget::onSearchTextChanged);
    connect(m_genreFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &LibraryWidget::onFilterChanged);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &LibraryWidget::onSortOrderChanged);
    connect(m_viewModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &LibraryWidget::onViewModeChanged);
    
    // 按钮
    connect(m_addComicBtn, &QPushButton::clicked, this, &LibraryWidget::onAddComicClicked);
    connect(m_addFolderBtn, &QPushButton::clicked, this, &LibraryWidget::onAddFolderClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &LibraryWidget::onRemoveComicClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &LibraryWidget::onRefreshClicked);
    connect(m_favoritesBtn, &QPushButton::toggled, [this](bool checked) {
        m_favoritesOnly = checked;
        applyFilters();
    });
    
    // 视图控件
    connect(m_listWidget, &QListWidget::itemSelectionChanged, 
            this, &LibraryWidget::onItemSelectionChanged);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, 
            this, &LibraryWidget::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested, 
            this, &LibraryWidget::onContextMenuRequested);
    
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, 
            this, &LibraryWidget::onItemSelectionChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, 
            this, &LibraryWidget::onItemDoubleClicked);
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, 
            this, &LibraryWidget::onContextMenuRequested);
    
    // 定时器
    connect(m_searchTimer, &QTimer::timeout, this, &LibraryWidget::onSearchTextChanged);
    connect(m_scanTimer, &QTimer::timeout, this, &LibraryWidget::onScanTimer);
    
    // 文件系统监控
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, 
            this, &LibraryWidget::onDirectoryChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, 
            this, &LibraryWidget::onFileChanged);
}

void LibraryWidget::setViewMode(ViewMode mode)
{
    if (m_viewMode == mode) return;
    
    m_viewMode = mode;
    m_viewModeCombo->setCurrentIndex(static_cast<int>(mode));
    updateViewWidget();
}

void LibraryWidget::updateViewWidget()
{
    // 隐藏所有视图
    m_listWidget->hide();
    m_treeWidget->hide();
    m_gridWidget->hide();
    
    // 显示当前视图
    switch (m_viewMode) {
        case ListView:
            m_listWidget->show();
            break;
        case GridView:
            m_gridWidget->show();
            break;
        case DetailsView:
            m_treeWidget->show();
            break;
    }
    
    // 刷新显示内容
    applyFilters();
}

void LibraryWidget::setSortBy(SortBy sortBy, Qt::SortOrder order)
{
    m_sortBy = sortBy;
    m_sortOrder = order;
    m_sortCombo->setCurrentIndex(static_cast<int>(sortBy));
    applyFilters();
}

void LibraryWidget::setFilter(const QString &filter)
{
    m_currentFilter = filter;
    applyFilters();
}

void LibraryWidget::setGenreFilter(const QString &genre)
{
    m_currentGenreFilter = genre;
    
    // 更新组合框选择
    int index = m_genreFilter->findText(genre);
    if (index >= 0) {
        m_genreFilter->setCurrentIndex(index);
    }
    
    applyFilters();
}

void LibraryWidget::setFavoriteFilter(bool favoritesOnly)
{
    m_favoritesOnly = favoritesOnly;
    m_favoritesBtn->setChecked(favoritesOnly);
    applyFilters();
}

void LibraryWidget::addComicToLibrary(const QString &filePath)
{
    if (!isComicFile(filePath) || m_comicDatabase.contains(filePath)) {
        return;
    }
    
    ComicInfo info;
    info.filePath = filePath;
    info.fileName = QFileInfo(filePath).fileName();
    info.title = QFileInfo(filePath).baseName();
    info.dateAdded = QDateTime::currentDateTime();
    info.fileSize = QFileInfo(filePath).size();
    
    // 从文件提取元数据
    QStringList metadata = extractMetadata(filePath);
    if (metadata.size() >= 2) {
        info.author = metadata[0];
        info.genre = metadata[1];
    }
    
    // 获取页面数
    ComicParser parser;
    if (parser.parseComic(filePath)) {
        info.totalPages = parser.getPageCount();
    }
    
    m_comicDatabase.insert(filePath, info);
    
    // 生成缩略图（异步）
    generateThumbnail(filePath);
    
    // 更新显示
    applyFilters();
    updateStatusBar();
    
    emit libraryUpdated();
}

void LibraryWidget::addComicsFromDirectory(const QString &dirPath, bool recursive)
{
    if (!QDir(dirPath).exists()) {
        return;
    }
    
    m_isScanning = true;
    m_scanCurrentItem = 0;
    m_scanTotal = 0;
    
    // 显示进度条
    m_scanProgress->setVisible(true);
    m_statusLabel->setText("正在扫描目录...");
    
    // 开始扫描
    scanDirectory(dirPath, recursive);
    
    emit scanCompleted();
}

void LibraryWidget::removeComicFromLibrary(const QString &filePath)
{
    if (m_comicDatabase.remove(filePath)) {
        // 从缓存中移除缩略图
        m_thumbnailCache.remove(filePath);
        
        // 更新显示
        applyFilters();
        updateStatusBar();
        
        emit libraryUpdated();
    }
}

void LibraryWidget::updateComicInfo(const QString &filePath, const ComicInfo &info)
{
    if (m_comicDatabase.contains(filePath)) {
        m_comicDatabase[filePath] = info;
        applyFilters();
        emit libraryUpdated();
    }
}

QStringList LibraryWidget::getAllComicPaths() const
{
    return m_comicDatabase.keys();
}

ComicInfo LibraryWidget::getComicInfo(const QString &filePath) const
{
    return m_comicDatabase.value(filePath);
}

QStringList LibraryWidget::getFavoriteComics() const
{
    QStringList favorites;
    for (auto it = m_comicDatabase.begin(); it != m_comicDatabase.end(); ++it) {
        if (it.value().isFavorite) {
            favorites.append(it.key());
        }
    }
    return favorites;
}

QStringList LibraryWidget::getRecentComics(int count) const
{
    QList<QPair<QDateTime, QString>> recent;
    
    for (auto it = m_comicDatabase.begin(); it != m_comicDatabase.end(); ++it) {
        if (it.value().lastRead.isValid()) {
            recent.append(qMakePair(it.value().lastRead, it.key()));
        }
    }
    
    // 按最近阅读时间排序
    std::sort(recent.begin(), recent.end(), [](const QPair<QDateTime, QString> &a, const QPair<QDateTime, QString> &b) {
        return a.first > b.first;
    });
    
    QStringList result;
    for (int i = 0; i < qMin(count, recent.size()); ++i) {
        result.append(recent[i].second);
    }
    
    return result;
}

QStringList LibraryWidget::searchComics(const QString &query) const
{
    if (query.isEmpty()) {
        return m_comicDatabase.keys();
    }
    
    QStringList results;
    QString lowerQuery = query.toLower();
    
    for (auto it = m_comicDatabase.begin(); it != m_comicDatabase.end(); ++it) {
        const ComicInfo &info = it.value();
        
        // 搜索标题、文件名、作者、类型和标签
        if (info.title.toLower().contains(lowerQuery) ||
            info.fileName.toLower().contains(lowerQuery) ||
            info.author.toLower().contains(lowerQuery) ||
            info.genre.toLower().contains(lowerQuery) ||
            info.tags.join(" ").toLower().contains(lowerQuery)) {
            results.append(it.key());
        }
    }
    
    return results;
}

QStringList LibraryWidget::getComicsByGenre(const QString &genre) const
{
    QStringList results;
    
    for (auto it = m_comicDatabase.begin(); it != m_comicDatabase.end(); ++it) {
        if (it.value().genre == genre) {
            results.append(it.key());
        }
    }
    
    return results;
}

QStringList LibraryWidget::getComicsByAuthor(const QString &author) const
{
    QStringList results;
    
    for (auto it = m_comicDatabase.begin(); it != m_comicDatabase.end(); ++it) {
        if (it.value().author == author) {
            results.append(it.key());
        }
    }
    
    return results;
}

void LibraryWidget::setThumbnailSize(const QSize &size)
{
    m_thumbnailSize = size;
    
    // 重新生成缩略图
    m_thumbnailCache.clear();
    for (const QString &filePath : m_comicDatabase.keys()) {
        generateThumbnail(filePath);
    }
    
    applyFilters();
}

void LibraryWidget::setAutoScanDirectories(const QStringList &directories)
{
    // 移除旧的监控目录
    if (!m_fileWatcher->directories().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->directories());
    }
    
    m_scanDirectories = directories;
    
    // 添加新的监控目录
    for (const QString &dir : directories) {
        if (QDir(dir).exists()) {
            m_fileWatcher->addPath(dir);
        }
    }
}

void LibraryWidget::refreshLibrary()
{
    // 重新扫描所有监控目录
    for (const QString &dir : m_scanDirectories) {
        addComicsFromDirectory(dir, true);
    }
}

// 槽函数实现
void LibraryWidget::onComicOpened(const QString &filePath, int currentPage)
{
    if (m_comicDatabase.contains(filePath)) {
        ComicInfo &info = m_comicDatabase[filePath];
        info.lastRead = QDateTime::currentDateTime();
        info.currentPage = currentPage;
        
        // 保存数据
        saveLibraryData();
        
        emit libraryUpdated();
    }
}

void LibraryWidget::onFavoriteToggled(const QString &filePath, bool favorite)
{
    if (m_comicDatabase.contains(filePath)) {
        m_comicDatabase[filePath].isFavorite = favorite;
        
        // 如果当前只显示收藏，需要更新过滤
        if (m_favoritesOnly) {
            applyFilters();
        }
        
        saveLibraryData();
        emit libraryUpdated();
    }
}

void LibraryWidget::onRatingChanged(const QString &filePath, double rating)
{
    if (m_comicDatabase.contains(filePath)) {
        m_comicDatabase[filePath].rating = rating;
        saveLibraryData();
        emit libraryUpdated();
    }
}

void LibraryWidget::onSearchTextChanged()
{
    QString text = m_searchEdit->text();
    if (text != m_pendingSearch) {
        m_pendingSearch = text;
        m_searchTimer->start(); // 延迟搜索
    }
}

void LibraryWidget::onFilterChanged()
{
    QString genre = m_genreFilter->currentText();
    if (genre == "所有类型") {
        m_currentGenreFilter.clear();
    } else {
        m_currentGenreFilter = genre;
    }
    
    applyFilters();
}

void LibraryWidget::onViewModeChanged()
{
    ViewMode mode = static_cast<ViewMode>(m_viewModeCombo->currentData().toInt());
    setViewMode(mode);
}

void LibraryWidget::onSortOrderChanged()
{
    SortBy sortBy = static_cast<SortBy>(m_sortCombo->currentData().toInt());
    setSortBy(sortBy, m_sortOrder);
}

void LibraryWidget::onItemSelectionChanged()
{
    QStringList selectedPaths;
    
    if (m_viewMode == ListView) {
        for (QListWidgetItem *item : m_listWidget->selectedItems()) {
            selectedPaths.append(item->data(Qt::UserRole).toString());
        }
    } else if (m_viewMode == DetailsView) {
        for (QTreeWidgetItem *item : m_treeWidget->selectedItems()) {
            selectedPaths.append(item->data(0, Qt::UserRole).toString());
        }
    }
    
    if (selectedPaths.size() == 1) {
        emit comicSelected(selectedPaths.first());
    }
    
    // 更新移除按钮状态
    m_removeBtn->setEnabled(!selectedPaths.isEmpty());
}

void LibraryWidget::onItemDoubleClicked()
{
    QString filePath;
    
    if (m_viewMode == ListView) {
        QListWidgetItem *item = m_listWidget->currentItem();
        if (item) {
            filePath = item->data(Qt::UserRole).toString();
        }
    } else if (m_viewMode == DetailsView) {
        QTreeWidgetItem *item = m_treeWidget->currentItem();
        if (item) {
            filePath = item->data(0, Qt::UserRole).toString();
        }
    }
    
    if (!filePath.isEmpty()) {
        emit comicDoubleClicked(filePath);
    }
}

void LibraryWidget::onContextMenuRequested(const QPoint &pos)
{
    QString filePath;
    
    if (m_viewMode == ListView) {
        QListWidgetItem *item = m_listWidget->itemAt(pos);
        if (item) {
            filePath = item->data(Qt::UserRole).toString();
        }
    } else if (m_viewMode == DetailsView) {
        QTreeWidgetItem *item = m_treeWidget->itemAt(pos);
        if (item) {
            filePath = item->data(0, Qt::UserRole).toString();
        }
    }
    
    if (!filePath.isEmpty()) {
        createContextMenu();
        
        // 更新收藏状态
        bool isFavorite = m_comicDatabase.value(filePath).isFavorite;
        m_favoriteAction->setText(isFavorite ? "取消收藏" : "添加收藏");
        
        QPoint globalPos = mapToGlobal(pos);
        if (m_viewMode == ListView) {
            globalPos = m_listWidget->mapToGlobal(pos);
        } else if (m_viewMode == DetailsView) {
            globalPos = m_treeWidget->mapToGlobal(pos);
        }
        
        emit comicContextMenuRequested(filePath, globalPos);
        m_contextMenu->exec(globalPos);
    }
}

void LibraryWidget::onAddComicClicked()
{
    QStringList filters;
    filters << "漫画文件 (*.cbz *.cbr *.zip *.rar)"
            << "CBZ文件 (*.cbz)"
            << "CBR文件 (*.cbr)" 
            << "ZIP文件 (*.zip)"
            << "RAR文件 (*.rar)"
            << "所有文件 (*.*)";
    
    QStringList files = QFileDialog::getOpenFileNames(
        this, 
        "选择漫画文件", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        filters.join(";;")
    );
    
    for (const QString &file : files) {
        addComicToLibrary(file);
    }
}

void LibraryWidget::onAddFolderClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择漫画文件夹",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    );
    
    if (!dir.isEmpty()) {
        addComicsFromDirectory(dir, true);
    }
}

void LibraryWidget::onRemoveComicClicked()
{
    QStringList selectedPaths;
    
    if (m_viewMode == ListView) {
        for (QListWidgetItem *item : m_listWidget->selectedItems()) {
            selectedPaths.append(item->data(Qt::UserRole).toString());
        }
    } else if (m_viewMode == DetailsView) {
        for (QTreeWidgetItem *item : m_treeWidget->selectedItems()) {
            selectedPaths.append(item->data(0, Qt::UserRole).toString());
        }
    }
    
    if (selectedPaths.isEmpty()) {
        return;
    }
    
    int ret = QMessageBox::question(
        this,
        "确认移除",
        QString("确定要从图书馆中移除 %1 本漫画吗？\n\n注意：这不会删除文件，只是从图书馆列表中移除。")
            .arg(selectedPaths.size()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        for (const QString &path : selectedPaths) {
            removeComicFromLibrary(path);
        }
    }
}

void LibraryWidget::onRefreshClicked()
{
    refreshLibrary();
}

void LibraryWidget::onScanTimer()
{
    // 延迟扫描，避免频繁的文件系统操作
    if (!m_scanDirectories.isEmpty()) {
        for (const QString &dir : m_scanDirectories) {
            scanDirectory(dir, true);
        }
    }
}

void LibraryWidget::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path)
    // 目录变化时延迟扫描
    m_scanTimer->start();
}

void LibraryWidget::onFileChanged(const QString &path)
{
    // 文件变化时检查是否需要更新信息
    if (m_comicDatabase.contains(path)) {
        QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            // 文件被删除
            removeComicFromLibrary(path);
        } else {
            // 文件被修改，更新信息
            ComicInfo &info = m_comicDatabase[path];
            info.fileSize = fileInfo.size();
            
            // 重新生成缩略图
            generateThumbnail(path);
            
            applyFilters();
        }
    }
}

void LibraryWidget::onThumbnailGenerated(const QString &filePath, const QPixmap &thumbnail)
{
    if (m_comicDatabase.contains(filePath)) {
        // 更新缓存
        m_thumbnailCache.insert(filePath, thumbnail);
        
        // 限制缓存大小
        if (m_thumbnailCache.size() > m_maxThumbnailCacheSize) {
            // 移除最旧的缓存项（简单实现）
            auto it = m_thumbnailCache.begin();
            m_thumbnailCache.erase(it);
        }
        
        // 更新显示
        applyFilters();
    }
}

// 私有方法实现
void LibraryWidget::updateStatusBar()
{
    int totalComics = m_comicDatabase.size();
    int filteredComics = m_filteredComics.size();
    
    if (m_isScanning) {
        m_statusLabel->setText(QString("正在扫描... (%1/%2)").arg(m_scanCurrentItem).arg(m_scanTotal));
        m_scanProgress->setValue(m_scanCurrentItem);
        m_scanProgress->setMaximum(m_scanTotal);
    } else {
        m_statusLabel->setText("就绪");
        m_scanProgress->setVisible(false);
    }
    
    if (filteredComics == totalComics) {
        m_countLabel->setText(QString("共 %1 本漫画").arg(totalComics));
    } else {
        m_countLabel->setText(QString("显示 %1 / 共 %2 本漫画").arg(filteredComics).arg(totalComics));
    }
}

void LibraryWidget::loadLibraryData()
{
    ConfigManager *config = ConfigManager::instance();
    QString dataPath = config->getDataPath() + "/library.json";
    
    QFile file(dataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray comics = root["comics"].toArray();
    
    m_comicDatabase.clear();
    
    for (const QJsonValue &value : comics) {
        QJsonObject comicObj = value.toObject();
        
        ComicInfo info;
        info.filePath = comicObj["filePath"].toString();
        info.fileName = comicObj["fileName"].toString();
        info.title = comicObj["title"].toString();
        info.series = comicObj["series"].toString();
        info.author = comicObj["author"].toString();
        info.genre = comicObj["genre"].toString();
        info.dateAdded = QDateTime::fromString(comicObj["dateAdded"].toString(), Qt::ISODate);
        info.lastRead = QDateTime::fromString(comicObj["lastRead"].toString(), Qt::ISODate);
        info.totalPages = comicObj["totalPages"].toInt();
        info.currentPage = comicObj["currentPage"].toInt();
        info.fileSize = comicObj["fileSize"].toVariant().toLongLong();
        info.isFavorite = comicObj["isFavorite"].toBool();
        info.rating = comicObj["rating"].toDouble();
        info.description = comicObj["description"].toString();
        
        QJsonArray tagsArray = comicObj["tags"].toArray();
        for (const QJsonValue &tagValue : tagsArray) {
            info.tags.append(tagValue.toString());
        }
        
        // 只添加存在的文件
        if (QFile::exists(info.filePath)) {
            m_comicDatabase.insert(info.filePath, info);
        }
    }
    
    // 加载设置
    QJsonObject settings = root["settings"].toObject();
    m_viewMode = static_cast<ViewMode>(settings["viewMode"].toInt());
    m_sortBy = static_cast<SortBy>(settings["sortBy"].toInt());
    m_sortOrder = static_cast<Qt::SortOrder>(settings["sortOrder"].toInt());
    m_thumbnailSize = QSize(settings["thumbnailWidth"].toInt(128), settings["thumbnailHeight"].toInt(192));
    
    QJsonArray dirsArray = settings["scanDirectories"].toArray();
    m_scanDirectories.clear();
    for (const QJsonValue &dirValue : dirsArray) {
        m_scanDirectories.append(dirValue.toString());
    }
    
    // 更新UI
    m_viewModeCombo->setCurrentIndex(static_cast<int>(m_viewMode));
    m_sortCombo->setCurrentIndex(static_cast<int>(m_sortBy));
    setAutoScanDirectories(m_scanDirectories);
    updateViewWidget();
    applyFilters();
}

void LibraryWidget::saveLibraryData()
{
    ConfigManager *config = ConfigManager::instance();
    QString dataPath = config->getDataPath() + "/library.json";
    
    // 确保数据目录存在
    QDir().mkpath(QFileInfo(dataPath).absolutePath());
    
    QJsonObject root;
    QJsonArray comics;
    
    for (auto it = m_comicDatabase.begin(); it != m_comicDatabase.end(); ++it) {
        const ComicInfo &info = it.value();
        
        QJsonObject comicObj;
        comicObj["filePath"] = info.filePath;
        comicObj["fileName"] = info.fileName;
        comicObj["title"] = info.title;
        comicObj["series"] = info.series;
        comicObj["author"] = info.author;
        comicObj["genre"] = info.genre;
        comicObj["dateAdded"] = info.dateAdded.toString(Qt::ISODate);
        comicObj["lastRead"] = info.lastRead.toString(Qt::ISODate);
        comicObj["totalPages"] = info.totalPages;
        comicObj["currentPage"] = info.currentPage;
        comicObj["fileSize"] = QJsonValue::fromVariant(info.fileSize);
        comicObj["isFavorite"] = info.isFavorite;
        comicObj["rating"] = info.rating;
        comicObj["description"] = info.description;
        
        QJsonArray tagsArray;
        for (const QString &tag : info.tags) {
            tagsArray.append(tag);
        }
        comicObj["tags"] = tagsArray;
        
        comics.append(comicObj);
    }
    
    root["comics"] = comics;
    
    // 保存设置
    QJsonObject settings;
    settings["viewMode"] = static_cast<int>(m_viewMode);
    settings["sortBy"] = static_cast<int>(m_sortBy);
    settings["sortOrder"] = static_cast<int>(m_sortOrder);
    settings["thumbnailWidth"] = m_thumbnailSize.width();
    settings["thumbnailHeight"] = m_thumbnailSize.height();
    
    QJsonArray dirsArray;
    for (const QString &dir : m_scanDirectories) {
        dirsArray.append(dir);
    }
    settings["scanDirectories"] = dirsArray;
    
    root["settings"] = settings;
    
    QFile file(dataPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
    }
}

void LibraryWidget::generateThumbnail(const QString &filePath)
{
    // 如果已经有缓存，跳过
    if (m_thumbnailCache.contains(filePath)) {
        return;
    }
    
    // 异步生成缩略图
    ComicParser parser;
    if (parser.parseComic(filePath)) {
        QPixmap thumbnail = parser.getPagePixmap(0); // 获取第一页
        if (!thumbnail.isNull()) {
            // 缩放到合适大小
            thumbnail = thumbnail.scaled(m_thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            onThumbnailGenerated(filePath, thumbnail);
        }
    }
}

void LibraryWidget::scanDirectory(const QString &dirPath, bool recursive)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }
    
    QStringList filters;
    filters << "*.cbz" << "*.cbr" << "*.zip" << "*.rar";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    m_scanTotal += files.size();
    
    for (const QFileInfo &fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        if (!m_comicDatabase.contains(filePath)) {
            addComicToLibrary(filePath);
        }
        m_scanCurrentItem++;
        emit scanProgress(m_scanCurrentItem, m_scanTotal);
    }
    
    if (recursive) {
        QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &dirInfo : dirs) {
            scanDirectory(dirInfo.absoluteFilePath(), true);
        }
    }
    
    m_isScanning = false;
    updateStatusBar();
}

void LibraryWidget::applyFilters()
{
    m_filteredComics.clear();
    
    // 应用搜索过滤
    QStringList searchResults;
    if (!m_pendingSearch.isEmpty()) {
        searchResults = searchComics(m_pendingSearch);
    } else {
        searchResults = m_comicDatabase.keys();
    }
    
    // 应用类型过滤
    if (!m_currentGenreFilter.isEmpty()) {
        QStringList genreResults = getComicsByGenre(m_currentGenreFilter);
        QStringList intersection;
        
        for (const QString &path : searchResults) {
            if (genreResults.contains(path)) {
                intersection.append(path);
            }
        }
        searchResults = intersection;
    }
    
    // 应用收藏过滤
    if (m_favoritesOnly) {
        QStringList favoriteResults = getFavoriteComics();
        QStringList intersection;
        
        for (const QString &path : searchResults) {
            if (favoriteResults.contains(path)) {
                intersection.append(path);
            }
        }
        searchResults = intersection;
    }
    
    // 排序
    std::sort(searchResults.begin(), searchResults.end(), [this](const QString &a, const QString &b) {
        const ComicInfo &infoA = m_comicDatabase.value(a);
        const ComicInfo &infoB = m_comicDatabase.value(b);
        
        bool result = false;
        switch (m_sortBy) {
            case SortByName:
                result = infoA.title < infoB.title;
                break;
            case SortByDateAdded:
                result = infoA.dateAdded < infoB.dateAdded;
                break;
            case SortByLastRead:
                result = infoA.lastRead < infoB.lastRead;
                break;
            case SortBySize:
                result = infoA.fileSize < infoB.fileSize;
                break;
            case SortByRating:
                result = infoA.rating < infoB.rating;
                break;
        }
        
        return m_sortOrder == Qt::AscendingOrder ? result : !result;
    });
    
    m_filteredComics = searchResults;
    
    // 更新视图
    updateView();
    updateStatusBar();
    
    // 更新类型过滤器
    updateGenreFilter();
}

void LibraryWidget::updateView()
{
    // 清空当前视图
    m_listWidget->clear();
    m_treeWidget->clear();
    
    for (const QString &filePath : m_filteredComics) {
        const ComicInfo &info = m_comicDatabase.value(filePath);
        
        if (m_viewMode == ListView) {
            QListWidgetItem *item = new QListWidgetItem();
            item->setText(info.title);
            item->setData(Qt::UserRole, filePath);
            
            // 设置图标
            if (m_thumbnailCache.contains(filePath)) {
                item->setIcon(QIcon(m_thumbnailCache.value(filePath)));
            } else {
                // 使用默认图标
                // item->setIcon(QIcon(":/icons/library/comic"));
            }
            
            // 设置工具提示
            QString tooltip = QString("标题: %1\n作者: %2\n类型: %3\n页数: %4\n大小: %5")
                .arg(info.title)
                .arg(info.author.isEmpty() ? "未知" : info.author)
                .arg(info.genre.isEmpty() ? "未知" : info.genre)
                .arg(info.totalPages)
                .arg(formatFileSize(info.fileSize));
            item->setToolTip(tooltip);
            
            m_listWidget->addItem(item);
        } 
        else if (m_viewMode == DetailsView) {
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, info.title);
            item->setText(1, info.genre);
            item->setText(2, info.author);
            item->setText(3, formatFileSize(info.fileSize));
            item->setText(4, info.dateAdded.toString("yyyy-MM-dd"));
            item->setText(5, info.lastRead.isValid() ? info.lastRead.toString("yyyy-MM-dd") : "从未");
            item->setData(0, Qt::UserRole, filePath);
            
            // 设置收藏状态
            if (info.isFavorite) {
                item->setBackground(0, QColor(255, 255, 200));
            }
            
            m_treeWidget->addTopLevelItem(item);
        }
    }
    
    // 调整列宽
    if (m_viewMode == DetailsView) {
        for (int i = 1; i < m_treeWidget->columnCount(); ++i) {
            m_treeWidget->resizeColumnToContents(i);
        }
    }
}

void LibraryWidget::updateGenreFilter()
{
    // 收集所有类型
    QSet<QString> genres;
    for (const ComicInfo &info : m_comicDatabase) {
        if (!info.genre.isEmpty()) {
            genres.insert(info.genre);
        }
    }
    
    // 更新组合框
    QString currentGenre = m_genreFilter->currentText();
    m_genreFilter->clear();
    m_genreFilter->addItem("所有类型");
    
    QStringList sortedGenres = genres.values();
    sortedGenres.sort();
    m_genreFilter->addItems(sortedGenres);
    
    // 恢复选择
    int index = m_genreFilter->findText(currentGenre);
    if (index >= 0) {
        m_genreFilter->setCurrentIndex(index);
    }
}

void LibraryWidget::createContextMenu()
{
    if (m_contextMenu) {
        return;
    }
    
    m_contextMenu = new QMenu(this);
    
    m_openAction = m_contextMenu->addAction("打开");
    connect(m_openAction, &QAction::triggered, [this]() {
        QString filePath = getSelectedFilePath();
        if (!filePath.isEmpty()) {
            emit comicDoubleClicked(filePath);
        }
    });
    
    m_contextMenu->addSeparator();
    
    m_favoriteAction = m_contextMenu->addAction("添加收藏");
    connect(m_favoriteAction, &QAction::triggered, [this]() {
        QString filePath = getSelectedFilePath();
        if (!filePath.isEmpty()) {
            bool isFavorite = m_comicDatabase.value(filePath).isFavorite;
            onFavoriteToggled(filePath, !isFavorite);
        }
    });
    
    m_contextMenu->addSeparator();
    
    m_removeAction = m_contextMenu->addAction("从图书馆移除");
    connect(m_removeAction, &QAction::triggered, [this]() {
        QString filePath = getSelectedFilePath();
        if (!filePath.isEmpty()) {
            removeComicFromLibrary(filePath);
        }
    });
    
    m_contextMenu->addSeparator();
    
    m_propertiesAction = m_contextMenu->addAction("属性");
    connect(m_propertiesAction, &QAction::triggered, [this]() {
        QString filePath = getSelectedFilePath();
        if (!filePath.isEmpty()) {
            showComicProperties(filePath);
        }
    });
}

QString LibraryWidget::getSelectedFilePath() const
{
    if (m_viewMode == ListView) {
        QListWidgetItem *item = m_listWidget->currentItem();
        return item ? item->data(Qt::UserRole).toString() : QString();
    } else if (m_viewMode == DetailsView) {
        QTreeWidgetItem *item = m_treeWidget->currentItem();
        return item ? item->data(0, Qt::UserRole).toString() : QString();
    }
    return QString();
}

void LibraryWidget::showComicProperties(const QString &filePath)
{
    // 这里可以实现一个属性对话框
    // 暂时使用消息框显示基本信息
    
    const ComicInfo &info = m_comicDatabase.value(filePath);
    
    QString properties = QString(
        "标题: %1\n"
        "文件名: %2\n"
        "作者: %3\n"
        "类型: %4\n"
        "页数: %5\n"
        "文件大小: %6\n"
        "添加时间: %7\n"
        "最近阅读: %8\n"
        "当前页: %9/%10\n"
        "是否收藏: %11\n"
        "评分: %12"
    ).arg(info.title)
     .arg(info.fileName)
     .arg(info.author.isEmpty() ? "未知" : info.author)
     .arg(info.genre.isEmpty() ? "未知" : info.genre)
     .arg(info.totalPages)
     .arg(formatFileSize(info.fileSize))
     .arg(info.dateAdded.toString("yyyy-MM-dd hh:mm:ss"))
     .arg(info.lastRead.isValid() ? info.lastRead.toString("yyyy-MM-dd hh:mm:ss") : "从未")
     .arg(info.currentPage)
     .arg(info.totalPages)
     .arg(info.isFavorite ? "是" : "否")
     .arg(info.rating, 0, 'f', 1);
    
    QMessageBox::information(this, "漫画属性", properties);
}

QString LibraryWidget::formatFileSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes < KB) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < MB) {
        return QString("%1 KB").arg(bytes / KB);
    } else if (bytes < GB) {
        return QString("%1 MB").arg(bytes / MB);
    } else {
        return QString("%1 GB").arg(bytes / GB);
    }
}

QString LibraryWidget::getGenreFromFile(const QString &filePath) const
{
    // 简单的类型推测，基于文件路径
    QString path = filePath.toLower();
    
    if (path.contains("manga") || path.contains("漫画")) {
        return "漫画";
    } else if (path.contains("comic")) {
        return "漫画";
    } else if (path.contains("novel") || path.contains("小说")) {
        return "小说";
    } else if (path.contains("action") || path.contains("动作")) {
        return "动作";
    } else if (path.contains("romance") || path.contains("爱情")) {
        return "爱情";
    } else if (path.contains("horror") || path.contains("恐怖")) {
        return "恐怖";
    } else if (path.contains("sci-fi") || path.contains("科幻")) {
        return "科幻";
    }
    
    return "其他";
}

QString LibraryWidget::getAuthorFromFile(const QString &filePath) const
{
    // 简单的作者提取，从文件名或路径中提取
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.baseName();
    
    // 查找常见的作者标识符
    QRegularExpression authorRegex(R"(\[([^\]]+)\]|\(([^\)]+)\)|by[\s]+([^\s]+))");
    QRegularExpressionMatch match = authorRegex.match(fileName);
    
    if (match.hasMatch()) {
        for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
            QString captured = match.captured(i);
            if (!captured.isEmpty()) {
                return captured;
            }
        }
    }
    
    return QString();
}

QStringList LibraryWidget::extractMetadata(const QString &filePath) const
{
    QStringList metadata;
    
    // 提取作者
    QString author = getAuthorFromFile(filePath);
    metadata.append(author);
    
    // 提取类型
    QString genre = getGenreFromFile(filePath);
    metadata.append(genre);
    
    return metadata;
}

bool LibraryWidget::isComicFile(const QString &filePath) const
{
    QStringList supportedFormats;
    supportedFormats << "cbz" << "cbr" << "zip" << "rar";
    
    QString suffix = QFileInfo(filePath).suffix().toLower();
    return supportedFormats.contains(suffix);
}
