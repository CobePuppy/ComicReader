#ifndef BOOKMARKWIDGET_H
#define BOOKMARKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QProgressBar>
#include <QDateEdit>
#include <QGroupBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QTimer>
#include "core/bookmark/BookmarkManager.h"

class BookmarkManager;

class BookmarkItemWidget;
class ProgressItemWidget;

class BookmarkWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarkWidget(QWidget *parent = nullptr);
    
    void setCurrentComic(const QString &comicPath);
    void refreshBookmarks();
    void refreshProgress();

signals:
    void bookmarkSelected(const QString &comicPath, int pageNumber);
    void comicSelected(const QString &comicPath);
    void removeBookmarkRequested(const QString &bookmarkId);
    void editBookmarkRequested(const QString &bookmarkId);

public slots:
    void onBookmarkAdded(const QString &bookmarkId);
    void onBookmarkRemoved(const QString &bookmarkId);
    void onBookmarkUpdated(const QString &bookmarkId);
    void onProgressUpdated(const QString &comicPath);

private slots:
    void onSearchTextChanged();
    void onFilterChanged();
    void onSortChanged();
    void onBookmarkItemClicked(QTreeWidgetItem *item, int column);
    void onBookmarkItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onBookmarkContextMenu(const QPoint &pos);
    void onProgressItemClicked(QListWidgetItem *item);
    void onProgressItemDoubleClicked(QListWidgetItem *item);
    void onProgressContextMenu(const QPoint &pos);
    void onTagFilterChanged();
    void onDateRangeChanged();
    void onExportBookmarks();
    void onImportBookmarks();
    void onExportProgress();
    void onImportProgress();
    void onCleanupBookmarks();
    void onAddTag();
    void onRemoveTag();
    void onEditBookmark();
    void onDeleteBookmark();
    void onJumpToPage();
    void onShowInLibrary();
    void onRefreshTimer();

private:
    void setupUI();
    void setupBookmarksTab();
    void setupProgressTab();
    void setupSearchAndFilters();
    void setupToolbar();
    void connectSignals();
    
    void updateBookmarksList();
    void updateProgressList();
    void updateStatistics();
    void applyFilters();
    void applySorting();
    
    BookmarkItemWidget* createBookmarkItem(const BookmarkInfo &bookmark);
    ProgressItemWidget* createProgressItem(const ReadingProgress &progress);
    
    void showBookmarkDetails(const QString &bookmarkId);
    void showProgressDetails(const QString &comicPath);
    
    // UI组件
    QTabWidget *m_tabWidget;
    
    // 书签页面
    QWidget *m_bookmarksTab;
    QVBoxLayout *m_bookmarksLayout;
    QTreeWidget *m_bookmarksTree;
    QLineEdit *m_searchEdit;
    QComboBox *m_filterCombo;
    QComboBox *m_sortCombo;
    QComboBox *m_tagFilter;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QCheckBox *m_dateRangeEnabled;
    QLabel *m_bookmarkCountLabel;
    
    // 阅读进度页面
    QWidget *m_progressTab;
    QVBoxLayout *m_progressLayout;
    QListWidget *m_progressList;
    QComboBox *m_progressFilter;
    QComboBox *m_progressSort;
    QLabel *m_progressCountLabel;
    QProgressBar *m_totalProgressBar;
    QLabel *m_statisticsLabel;
    
    // 工具栏
    QHBoxLayout *m_toolbarLayout;
    QPushButton *m_addBookmarkBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_importBtn;
    QPushButton *m_cleanupBtn;
    QPushButton *m_refreshBtn;
    
    // 详情面板
    QSplitter *m_splitter;
    QWidget *m_detailsPanel;
    QVBoxLayout *m_detailsLayout;
    QLabel *m_detailsTitle;
    QLabel *m_detailsComic;
    QLabel *m_detailsPage;
    QLabel *m_detailsDate;
    QTextEdit *m_detailsDescription;
    QListWidget *m_detailsTags;
    QPushButton *m_editDetailsBtn;
    QPushButton *m_jumpToPageBtn;
    QPushButton *m_showInLibraryBtn;
    
    // 上下文菜单
    QMenu *m_bookmarkContextMenu;
    QMenu *m_progressContextMenu;
    
    // 数据和状态
    BookmarkManager *m_bookmarkManager;
    QString m_currentComicPath;
    QString m_selectedBookmarkId;
    QString m_selectedComicPath;
    
    // 定时器
    QTimer *m_refreshTimer;
    
    // 搜索和过滤状态
    QString m_currentSearchText;
    int m_currentFilter;
    int m_currentSort;
    QString m_currentTag;
    bool m_dateRangeFilter;
    QDateTime m_startDate;
    QDateTime m_endDate;
};

// 自定义书签项目组件
class BookmarkItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarkItemWidget(const BookmarkInfo &bookmark, QWidget *parent = nullptr);
    
    void updateBookmark(const BookmarkInfo &bookmark);
    QString getBookmarkId() const { return m_bookmarkId; }

signals:
    void clicked();
    void doubleClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    
    QString m_bookmarkId;
    QLabel *m_titleLabel;
    QLabel *m_comicLabel;
    QLabel *m_pageLabel;
    QLabel *m_dateLabel;
    QLabel *m_thumbnailLabel;
    QLabel *m_tagsLabel;
    
    BookmarkInfo m_bookmark;
};

// 自定义进度项目组件
class ProgressItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressItemWidget(const ReadingProgress &progress, QWidget *parent = nullptr);
    
    void updateProgress(const ReadingProgress &progress);
    QString getComicPath() const { return m_comicPath; }

signals:
    void clicked();
    void doubleClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    void updateProgressBar();
    QString formatTime(qint64 seconds) const;
    
    QString m_comicPath;
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QLabel *m_timeLabel;
    QProgressBar *m_progressBar;
    QLabel *m_pagesLabel;
    
    ReadingProgress m_progress;
};

#endif // BOOKMARKWIDGET_H