#include "ErrorLogDialog.h"
#include "ErrorHandler.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>
#include <algorithm>

ErrorLogDialog::ErrorLogDialog(QWidget *parent)
    : QDialog(parent)
    , m_errorHandler(ErrorHandler::instance())
    , m_currentSeverityFilter(-1) // 全部
    , m_currentCategoryFilter(-1) // 全部
    , m_dateRangeFilter(false)
    , m_selectedErrorId("")
{
    setWindowTitle("错误日志 - ComicReader");
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    resize(1000, 700);
    
    setupUI();
    connectSignals();
    
    // 设置自动刷新定时器
    m_autoRefreshTimer = new QTimer(this);
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &ErrorLogDialog::onAutoRefreshTimer);
    
    // 初始加载
    refreshErrors();
}

void ErrorLogDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 工具栏
    setupToolbar();
    
    // 过滤器
    setupFilters();
    
    // 分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);
    
    // 错误表格
    setupErrorTable();
    
    // 详情面板
    setupDetailsPanel();
    
    // 设置分割器比例
    m_splitter->setSizes({600, 400});
}

void ErrorLogDialog::setupToolbar()
{
    m_toolbarLayout = new QHBoxLayout();
    m_mainLayout->addLayout(m_toolbarLayout);
    
    m_refreshBtn = new QPushButton("刷新", this);
    m_refreshBtn->setIcon(QIcon(":/icons/refresh.png"));
    m_toolbarLayout->addWidget(m_refreshBtn);
    
    m_clearBtn = new QPushButton("清空", this);
    m_clearBtn->setIcon(QIcon(":/icons/clear.png"));
    m_toolbarLayout->addWidget(m_clearBtn);
    
    m_exportBtn = new QPushButton("导出", this);
    m_exportBtn->setIcon(QIcon(":/icons/export.png"));
    m_toolbarLayout->addWidget(m_exportBtn);
    
    m_toolbarLayout->addSpacing(20);
    
    m_autoRefreshCheck = new QCheckBox("自动刷新", this);
    m_autoRefreshCheck->setChecked(true);
    m_toolbarLayout->addWidget(m_autoRefreshCheck);
    
    m_toolbarLayout->addStretch();
    
    m_statisticsLabel = new QLabel("统计信息", this);
    m_statisticsLabel->setStyleSheet("color: #666; font-size: 12px;");
    m_toolbarLayout->addWidget(m_statisticsLabel);
    
    m_loadingProgress = new QProgressBar(this);
    m_loadingProgress->setVisible(false);
    m_loadingProgress->setMaximumWidth(150);
    m_toolbarLayout->addWidget(m_loadingProgress);
}

void ErrorLogDialog::setupFilters()
{
    m_filterWidget = new QWidget(this);
    m_filterLayout = new QHBoxLayout(m_filterWidget);
    m_filterLayout->setContentsMargins(0, 5, 0, 5);
    
    // 严重程度过滤
    m_filterLayout->addWidget(new QLabel("严重程度:", this));
    m_severityFilter = new QComboBox(this);
    m_severityFilter->addItems({"全部", "信息", "警告", "错误", "严重"});
    m_filterLayout->addWidget(m_severityFilter);
    
    // 类别过滤
    m_filterLayout->addWidget(new QLabel("类别:", this));
    m_categoryFilter = new QComboBox(this);
    m_categoryFilter->addItems({"全部", "文件IO", "网络", "解析", "缓存", "配置", "界面", "内存", "权限", "未知"});
    m_filterLayout->addWidget(m_categoryFilter);
    
    // 搜索框
    m_filterLayout->addWidget(new QLabel("搜索:", this));
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索错误标题、消息或代码...");
    m_filterLayout->addWidget(m_searchEdit);
    
    m_filterLayout->addSpacing(10);
    
    // 日期范围过滤
    m_dateRangeEnabled = new QCheckBox("日期范围:", this);
    m_filterLayout->addWidget(m_dateRangeEnabled);
    
    m_startDateEdit = new QDateEdit(this);
    m_startDateEdit->setDate(QDate::currentDate().addDays(-7));
    m_startDateEdit->setEnabled(false);
    m_filterLayout->addWidget(m_startDateEdit);
    
    m_filterLayout->addWidget(new QLabel("到", this));
    
    m_endDateEdit = new QDateEdit(this);
    m_endDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setEnabled(false);
    m_filterLayout->addWidget(m_endDateEdit);
    
    m_clearFiltersBtn = new QPushButton("清除过滤", this);
    m_filterLayout->addWidget(m_clearFiltersBtn);
    
    m_filterLayout->addStretch();
    
    m_mainLayout->addWidget(m_filterWidget);
}

void ErrorLogDialog::setupErrorTable()
{
    m_errorTable = new QTableWidget(this);
    m_errorTable->setColumnCount(6);
    m_errorTable->setHorizontalHeaderLabels({"时间", "严重程度", "类别", "代码", "标题", "消息"});
    
    // 设置表格属性
    m_errorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_errorTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_errorTable->setAlternatingRowColors(true);
    m_errorTable->setSortingEnabled(true);
    m_errorTable->verticalHeader()->setVisible(false);
    
    // 调整列宽
    QHeaderView *header = m_errorTable->horizontalHeader();
    header->resizeSection(0, 140); // 时间
    header->resizeSection(1, 80);  // 严重程度
    header->resizeSection(2, 80);  // 类别
    header->resizeSection(3, 100); // 代码
    header->resizeSection(4, 200); // 标题
    header->setStretchLastSection(true); // 消息列自动拉伸
    
    m_splitter->addWidget(m_errorTable);
}

void ErrorLogDialog::setupDetailsPanel()
{
    m_detailsPanel = new QWidget(this);
    m_detailsLayout = new QVBoxLayout(m_detailsPanel);
    
    m_detailsTitle = new QLabel("错误详情", this);
    m_detailsTitle->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 5px;");
    m_detailsLayout->addWidget(m_detailsTitle);
    
    m_detailsText = new QTextEdit(this);
    m_detailsText->setReadOnly(true);
    m_detailsText->setFont(QFont("Courier", 10)); // 等宽字体便于阅读
    m_detailsLayout->addWidget(m_detailsText);
    
    m_splitter->addWidget(m_detailsPanel);
}

void ErrorLogDialog::connectSignals()
{
    // 错误处理器信号
    connect(m_errorHandler, &ErrorHandler::errorReported,
            this, &ErrorLogDialog::onErrorReported);
    connect(m_errorHandler, &ErrorHandler::errorsCleared,
            this, &ErrorLogDialog::onErrorsCleared);
    connect(m_errorHandler, &ErrorHandler::errorStatisticsChanged,
            this, &ErrorLogDialog::onStatisticsChanged);
    
    // 过滤器信号
    connect(m_severityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ErrorLogDialog::onFilterChanged);
    connect(m_categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ErrorLogDialog::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &ErrorLogDialog::onSearchTextChanged);
    
    // 日期范围
    connect(m_dateRangeEnabled, &QCheckBox::toggled, [this](bool enabled) {
        m_startDateEdit->setEnabled(enabled);
        m_endDateEdit->setEnabled(enabled);
        m_dateRangeFilter = enabled;
        onFilterChanged();
    });
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &ErrorLogDialog::onFilterChanged);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &ErrorLogDialog::onFilterChanged);
    
    // 清除过滤器
    connect(m_clearFiltersBtn, &QPushButton::clicked, [this]() {
        m_severityFilter->setCurrentIndex(0);
        m_categoryFilter->setCurrentIndex(0);
        m_searchEdit->clear();
        m_dateRangeEnabled->setChecked(false);
        onFilterChanged();
    });
    
    // 表格选择
    connect(m_errorTable, &QTableWidget::itemSelectionChanged,
            this, &ErrorLogDialog::onErrorSelectionChanged);
    
    // 工具栏按钮
    connect(m_refreshBtn, &QPushButton::clicked, this, &ErrorLogDialog::onRefreshErrors);
    connect(m_clearBtn, &QPushButton::clicked, this, &ErrorLogDialog::onClearErrors);
    connect(m_exportBtn, &QPushButton::clicked, this, &ErrorLogDialog::onExportErrors);
    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &ErrorLogDialog::onAutoRefreshToggled);
}

void ErrorLogDialog::refreshErrors()
{
    m_loadingProgress->setVisible(true);
    m_loadingProgress->setRange(0, 0); // 不确定进度
    
    QApplication::processEvents();
    
    updateErrorTable();
    updateStatistics();
    
    m_loadingProgress->setVisible(false);
}

void ErrorLogDialog::updateErrorTable()
{
    m_errorTable->clearContents();
    
    QList<ErrorInfo> allErrors = m_errorHandler->getAllErrors();
    
    // 应用过滤器
    QList<ErrorInfo> filteredErrors;
    for (const ErrorInfo &error : allErrors) {
        bool matches = true;
        
        // 严重程度过滤
        if (m_currentSeverityFilter >= 0 && 
            static_cast<int>(error.severity) != m_currentSeverityFilter) {
            matches = false;
        }
        
        // 类别过滤
        if (matches && m_currentCategoryFilter >= 0 && 
            static_cast<int>(error.category) != m_currentCategoryFilter) {
            matches = false;
        }
        
        // 搜索文本过滤
        if (matches && !m_currentSearchText.isEmpty()) {
            QString lowerSearch = m_currentSearchText.toLower();
            bool textMatches = error.title.toLower().contains(lowerSearch) ||
                              error.message.toLower().contains(lowerSearch) ||
                              error.code.toLower().contains(lowerSearch);
            if (!textMatches) {
                matches = false;
            }
        }
        
        // 日期范围过滤
        if (matches && m_dateRangeFilter) {
            QDateTime startDateTime = QDateTime(m_startDate);
            QDateTime endDateTime = QDateTime(m_endDate).addDays(1);
            if (error.timestamp < startDateTime || error.timestamp >= endDateTime) {
                matches = false;
            }
        }
        
        if (matches) {
            filteredErrors.append(error);
        }
    }
    
    // 设置表格行数
    m_errorTable->setRowCount(filteredErrors.size());
    
    // 填充表格数据
    for (int i = 0; i < filteredErrors.size(); ++i) {
        const ErrorInfo &error = filteredErrors[i];
        
        // 时间
        QTableWidgetItem *timeItem = new QTableWidgetItem(error.timestamp.toString("MM-dd hh:mm:ss"));
        timeItem->setData(Qt::UserRole, error.id);
        m_errorTable->setItem(i, 0, timeItem);
        
        // 严重程度
        QTableWidgetItem *severityItem = new QTableWidgetItem(getSeverityText(static_cast<int>(error.severity)));
        severityItem->setIcon(getSeverityIcon(static_cast<int>(error.severity)));
        severityItem->setForeground(QBrush(getSeverityColor(static_cast<int>(error.severity))));
        severityItem->setData(Qt::UserRole, static_cast<int>(error.severity));
        m_errorTable->setItem(i, 1, severityItem);
        
        // 类别
        QTableWidgetItem *categoryItem = new QTableWidgetItem(getCategoryText(static_cast<int>(error.category)));
        categoryItem->setData(Qt::UserRole, static_cast<int>(error.category));
        m_errorTable->setItem(i, 2, categoryItem);
        
        // 代码
        QTableWidgetItem *codeItem = new QTableWidgetItem(error.code);
        m_errorTable->setItem(i, 3, codeItem);
        
        // 标题
        QTableWidgetItem *titleItem = new QTableWidgetItem(error.title);
        titleItem->setToolTip(error.title);
        m_errorTable->setItem(i, 4, titleItem);
        
        // 消息
        QTableWidgetItem *messageItem = new QTableWidgetItem(error.message);
        messageItem->setToolTip(error.message);
        m_errorTable->setItem(i, 5, messageItem);
        
        // 设置行的背景色（根据严重程度）
        QColor bgColor = getSeverityColor(static_cast<int>(error.severity));
        bgColor.setAlpha(30); // 设置透明度
        for (int col = 0; col < 6; ++col) {
            if (m_errorTable->item(i, col)) {
                m_errorTable->item(i, col)->setBackground(QBrush(bgColor));
            }
        }
    }
    
    // 按时间排序（最新的在前面）
    m_errorTable->sortByColumn(0, Qt::DescendingOrder);
}

void ErrorLogDialog::updateErrorDetails(const ErrorInfo &error)
{
    if (!error.id.isEmpty()) {
        m_detailsTitle->setText(QString("错误详情 - %1").arg(error.code));
        
        QString details;
        details += QString("错误ID: %1\n").arg(error.id);
        details += QString("严重程度: %1\n").arg(getSeverityText(static_cast<int>(error.severity)));
        details += QString("类别: %1\n").arg(getCategoryText(static_cast<int>(error.category)));
        details += QString("代码: %1\n").arg(error.code);
        details += QString("标题: %1\n").arg(error.title);
        details += QString("时间: %1\n").arg(error.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
        details += QString("\n消息:\n%1\n").arg(error.message);
        
        if (!error.context.isEmpty()) {
            details += QString("\n上下文:\n%1\n").arg(error.context);
        }
        
        if (!error.file.isEmpty()) {
            details += QString("\n位置:\n文件: %1\n行号: %2\n函数: %3\n")
                      .arg(error.file).arg(error.line).arg(error.function);
        }
        
        if (!error.extra.isEmpty()) {
            details += "\n额外信息:\n";
            for (auto it = error.extra.begin(); it != error.extra.end(); ++it) {
                details += QString("%1: %2\n").arg(it.key(), it.value());
            }
        }
        
        m_detailsText->setPlainText(details);
    } else {
        m_detailsTitle->setText("错误详情");
        m_detailsText->clear();
    }
}

void ErrorLogDialog::updateStatistics()
{
    QHash<ErrorSeverity, int> severityStats = m_errorHandler->getSeverityStatistics();
    
    int totalErrors = m_errorHandler->getErrorCount();
    int criticalCount = severityStats.value(ErrorSeverity::Critical, 0);
    int errorCount = severityStats.value(ErrorSeverity::Error, 0);
    int warningCount = severityStats.value(ErrorSeverity::Warning, 0);
    int infoCount = severityStats.value(ErrorSeverity::Info, 0);
    
    QString stats = QString("总计: %1  严重: %2  错误: %3  警告: %4  信息: %5")
                   .arg(totalErrors).arg(criticalCount).arg(errorCount).arg(warningCount).arg(infoCount);
    
    m_statisticsLabel->setText(stats);
}

void ErrorLogDialog::showError(const QString &errorId)
{
    // 在表格中查找并选择指定的错误
    for (int i = 0; i < m_errorTable->rowCount(); ++i) {
        QTableWidgetItem *item = m_errorTable->item(i, 0);
        if (item && item->data(Qt::UserRole).toString() == errorId) {
            m_errorTable->selectRow(i);
            m_errorTable->scrollToItem(item);
            break;
        }
    }
}

// 槽函数实现
void ErrorLogDialog::onFilterChanged()
{
    m_currentSeverityFilter = m_severityFilter->currentIndex() - 1; // -1表示全部
    m_currentCategoryFilter = m_categoryFilter->currentIndex() - 1; // -1表示全部
    
    if (m_dateRangeFilter) {
        m_startDate = m_startDateEdit->date();
        m_endDate = m_endDateEdit->date();
    }
    
    updateErrorTable();
}

void ErrorLogDialog::onSearchTextChanged()
{
    m_currentSearchText = m_searchEdit->text().trimmed();
    updateErrorTable();
}

void ErrorLogDialog::onErrorSelectionChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_errorTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        QTableWidgetItem *firstItem = selectedItems.first();
        QString errorId = firstItem->data(Qt::UserRole).toString();
        m_selectedErrorId = errorId;
        
        ErrorInfo error = m_errorHandler->getError(errorId);
        updateErrorDetails(error);
    } else {
        m_selectedErrorId.clear();
        updateErrorDetails(ErrorInfo());
    }
}

void ErrorLogDialog::onClearErrors()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "清空错误日志",
        "确定要清空所有错误记录吗？此操作不可撤销。",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_errorHandler->clearErrors();
    }
}

void ErrorLogDialog::onExportErrors()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出错误日志",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/error_log.json",
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        if (m_errorHandler->exportErrors(fileName)) {
            QMessageBox::information(this, "导出成功", "错误日志已成功导出。");
        } else {
            QMessageBox::warning(this, "导出失败", "导出错误日志时发生错误。");
        }
    }
}

void ErrorLogDialog::onRefreshErrors()
{
    refreshErrors();
}

void ErrorLogDialog::onAutoRefreshToggled(bool enabled)
{
    if (enabled) {
        m_autoRefreshTimer->start(5000); // 5秒刷新一次
    } else {
        m_autoRefreshTimer->stop();
    }
}

void ErrorLogDialog::onErrorReported(const ErrorInfo &error)
{
    Q_UNUSED(error)
    if (m_autoRefreshCheck->isChecked()) {
        refreshErrors();
    }
}

void ErrorLogDialog::onErrorsCleared()
{
    refreshErrors();
}

void ErrorLogDialog::onStatisticsChanged()
{
    updateStatistics();
}

void ErrorLogDialog::onAutoRefreshTimer()
{
    if (m_autoRefreshCheck->isChecked()) {
        refreshErrors();
    }
}

// 辅助函数实现
QIcon ErrorLogDialog::getSeverityIcon(int severity) const
{
    switch (static_cast<ErrorSeverity>(severity)) {
    case ErrorSeverity::Info:
        return QIcon(":/icons/info.png");
    case ErrorSeverity::Warning:
        return QIcon(":/icons/warning.png");
    case ErrorSeverity::Error:
        return QIcon(":/icons/error.png");
    case ErrorSeverity::Critical:
        return QIcon(":/icons/critical.png");
    default:
        return QIcon();
    }
}

QString ErrorLogDialog::getSeverityText(int severity) const
{
    switch (static_cast<ErrorSeverity>(severity)) {
    case ErrorSeverity::Info:
        return "信息";
    case ErrorSeverity::Warning:
        return "警告";
    case ErrorSeverity::Error:
        return "错误";
    case ErrorSeverity::Critical:
        return "严重";
    default:
        return "未知";
    }
}

QString ErrorLogDialog::getCategoryText(int category) const
{
    switch (static_cast<ErrorCategory>(category)) {
    case ErrorCategory::FileIO:
        return "文件IO";
    case ErrorCategory::Network:
        return "网络";
    case ErrorCategory::Parsing:
        return "解析";
    case ErrorCategory::Cache:
        return "缓存";
    case ErrorCategory::Configuration:
        return "配置";
    case ErrorCategory::UI:
        return "界面";
    case ErrorCategory::Memory:
        return "内存";
    case ErrorCategory::Permission:
        return "权限";
    default:
        return "未知";
    }
}

QColor ErrorLogDialog::getSeverityColor(int severity) const
{
    switch (static_cast<ErrorSeverity>(severity)) {
    case ErrorSeverity::Info:
        return QColor("#0099FF"); // 蓝色
    case ErrorSeverity::Warning:
        return QColor("#FF9900"); // 橙色
    case ErrorSeverity::Error:
        return QColor("#FF3300"); // 红色
    case ErrorSeverity::Critical:
        return QColor("#CC0000"); // 深红色
    default:
        return QColor("#666666"); // 灰色
    }
}