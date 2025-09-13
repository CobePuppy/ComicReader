#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QMenu>
#include <QTimer>
#include <QFileSystemWatcher>

class ComicLibraryModel;
class ComicItemDelegate;

struct ComicInfo {
    QString filePath;
    QString fileName;
    QString title;
    QString series;
    QString author;
    QString genre;
    QDateTime dateAdded;
    QDateTime lastRead;
    int totalPages;
    int currentPage;
    qint64 fileSize;
    QPixmap thumbnail;
    bool isFavorite;
    QStringList tags;
    QString description;
    double rating;
    
    ComicInfo() : totalPages(0), currentPage(0), fileSize(0), isFavorite(false), rating(0.0) {}
};

class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    enum ViewMode {
        ListView,
        GridView,
        DetailsView
    };
    
    enum SortBy {
        SortByName,
        SortByDateAdded,
        SortByLastRead,
        SortBySize,
        SortByRating
    };

    explicit LibraryWidget(QWidget *parent = nullptr);
    ~LibraryWidget();

    // 视图管理
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return m_viewMode; }
    
    // 排序和过滤
    void setSortBy(SortBy sortBy, Qt::SortOrder order = Qt::AscendingOrder);
    void setFilter(const QString &filter);
    void setGenreFilter(const QString &genre);
    void setFavoriteFilter(bool favoritesOnly);
    
    // 漫画管理
    void addComicToLibrary(const QString &filePath);
    void addComicsFromDirectory(const QString &dirPath, bool recursive = true);
    void removeComicFromLibrary(const QString &filePath);
    void updateComicInfo(const QString &filePath, const ComicInfo &info);
    
    // 获取信息
    QStringList getAllComicPaths() const;
    ComicInfo getComicInfo(const QString &filePath) const;
    QStringList getFavoriteComics() const;
    QStringList getRecentComics(int count = 10) const;
    
    // 搜索功能
    QStringList searchComics(const QString &query) const;
    QStringList getComicsByGenre(const QString &genre) const;
    QStringList getComicsByAuthor(const QString &author) const;
    
    // 配置
    void setThumbnailSize(const QSize &size);
    void setAutoScanDirectories(const QStringList &directories);
    void refreshLibrary();

signals:
    void comicSelected(const QString &filePath);
    void comicDoubleClicked(const QString &filePath);
    void comicContextMenuRequested(const QString &filePath, const QPoint &pos);
    void libraryUpdated();
    void scanProgress(int current, int total);
    void scanCompleted();

public slots:
    void onComicOpened(const QString &filePath, int currentPage);
    void onFavoriteToggled(const QString &filePath, bool favorite);
    void onRatingChanged(const QString &filePath, double rating);

private slots:
    void onSearchTextChanged();
    void onFilterChanged();
    void onViewModeChanged();
    void onSortOrderChanged();
    void onItemSelectionChanged();
    void onItemDoubleClicked();
    void onContextMenuRequested(const QPoint &pos);
    void onAddComicClicked();
    void onAddFolderClicked();
    void onRemoveComicClicked();
    void onRefreshClicked();
    void onScanTimer();
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);
    void onThumbnailGenerated(const QString &filePath, const QPixmap &thumbnail);

private:
    void setupUI();
    void setupToolbar();
    void setupViewArea();
    void setupStatusBar();
    void connectSignals();
    
    void updateViewWidget();
    void updateStatusBar();
    void loadLibraryData();
    void saveLibraryData();
    void generateThumbnail(const QString &filePath);
    void scanDirectory(const QString &dirPath, bool recursive);
    void applyFilters();
    void createContextMenu();
    
    QString getGenreFromFile(const QString &filePath) const;
    QString getAuthorFromFile(const QString &filePath) const;
    QStringList extractMetadata(const QString &filePath) const;
    bool isComicFile(const QString &filePath) const;
    
    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_toolbarLayout;
    QSplitter *m_splitter;
    
    // 工具栏
    QLineEdit *m_searchEdit;
    QComboBox *m_genreFilter;
    QComboBox *m_sortCombo;
    QComboBox *m_viewModeCombo;
    QPushButton *m_addComicBtn;
    QPushButton *m_addFolderBtn;
    QPushButton *m_removeBtn;
    QPushButton *m_refreshBtn;
    QPushButton *m_favoritesBtn;
    
    // 视图区域
    QWidget *m_viewContainer;
    QListWidget *m_listWidget;
    QTreeWidget *m_treeWidget;
    QWidget *m_gridWidget;
    
    // 状态栏
    QHBoxLayout *m_statusLayout;
    QLabel *m_statusLabel;
    QLabel *m_countLabel;
    QProgressBar *m_scanProgress;
    
    // 上下文菜单
    QMenu *m_contextMenu;
    QAction *m_openAction;
    QAction *m_favoriteAction;
    QAction *m_removeAction;
    QAction *m_propertiesAction;
    
    // 数据和状态
    QHash<QString, ComicInfo> m_comicDatabase;
    QStringList m_filteredComics;
    ViewMode m_viewMode;
    SortBy m_sortBy;
    Qt::SortOrder m_sortOrder;
    QString m_currentFilter;
    QString m_currentGenreFilter;
    bool m_favoritesOnly;
    QSize m_thumbnailSize;
    
    // 扫描和监控
    QStringList m_scanDirectories;
    QFileSystemWatcher *m_fileWatcher;
    QTimer *m_scanTimer;
    bool m_isScanning;
    int m_scanProgress;
    int m_scanTotal;
    
    // 缓存和性能
    QTimer *m_searchTimer;
    QString m_pendingSearch;
    QHash<QString, QPixmap> m_thumbnailCache;
    int m_maxThumbnailCacheSize;
};

#endif // LIBRARYWIDGET_H
