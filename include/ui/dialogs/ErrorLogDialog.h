#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QCheckBox>
#include <QDateEdit>
#include <QProgressBar>
#include <QTimer>

class ErrorHandler;
struct ErrorInfo;

class ErrorLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorLogDialog(QWidget *parent = nullptr);
    
    void refreshErrors();
    void showError(const QString &errorId);

private slots:
    void onFilterChanged();
    void onSearchTextChanged();
    void onErrorSelectionChanged();
    void onClearErrors();
    void onExportErrors();
    void onRefreshErrors();
    void onAutoRefreshToggled(bool enabled);
    void onErrorReported(const ErrorInfo &error);
    void onErrorsCleared();
    void onStatisticsChanged();
    void onAutoRefreshTimer();

private:
    void setupUI();
    void setupFilters();
    void setupErrorTable();
    void setupDetailsPanel();
    void setupToolbar();
    void connectSignals();
    
    void updateErrorTable();
    void updateErrorDetails(const ErrorInfo &error);
    void updateStatistics();
    void applyFilters();
    
    QIcon getSeverityIcon(int severity) const;
    QString getSeverityText(int severity) const;
    QString getCategoryText(int category) const;
    QColor getSeverityColor(int severity) const;
    
    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_toolbarLayout;
    QSplitter *m_splitter;
    
    // 过滤器
    QWidget *m_filterWidget;
    QHBoxLayout *m_filterLayout;
    QComboBox *m_severityFilter;
    QComboBox *m_categoryFilter;
    QLineEdit *m_searchEdit;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QCheckBox *m_dateRangeEnabled;
    QPushButton *m_clearFiltersBtn;
    
    // 错误表格
    QTableWidget *m_errorTable;
    
    // 详情面板
    QWidget *m_detailsPanel;
    QVBoxLayout *m_detailsLayout;
    QLabel *m_detailsTitle;
    QTextEdit *m_detailsText;
    
    // 工具栏
    QPushButton *m_refreshBtn;
    QPushButton *m_clearBtn;
    QPushButton *m_exportBtn;
    QCheckBox *m_autoRefreshCheck;
    QLabel *m_statisticsLabel;
    QProgressBar *m_loadingProgress;
    
    // 状态
    ErrorHandler *m_errorHandler;
    QTimer *m_autoRefreshTimer;
    QString m_selectedErrorId;
    
    // 过滤状态
    int m_currentSeverityFilter;
    int m_currentCategoryFilter;
    QString m_currentSearchText;
    bool m_dateRangeFilter;
    QDateTime m_startDate;
    QDateTime m_endDate;
};

#endif // ERRORLOGDIALOG_H