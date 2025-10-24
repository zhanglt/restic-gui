#include "RestoreWizard.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/PasswordManager.h"
#include "../../utils/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QtConcurrent>
#include <QDir>
#include <QTimer>
#include <functional>

namespace ResticGUI {
namespace UI {

// ============================================================================
// RestoreWizard - 主向导类
// ============================================================================

RestoreWizard::RestoreWizard(QWidget* parent)
    : QWizard(parent)
    , m_repositoryId(-1)
    , m_snapshotId()
    , m_selectedPaths()
    , m_options()
    , m_presetTargetPath()
{
    setWindowTitle(tr("数据恢复向导"));
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::NoBackButtonOnStartPage, true);

    // 设置按钮文本为中文
    setButtonText(QWizard::NextButton, tr("下一步 >"));
    setButtonText(QWizard::BackButton, tr("< 上一步"));
    setButtonText(QWizard::FinishButton, tr("完成"));
    setButtonText(QWizard::CancelButton, tr("取消"));

    // 添加页面
    setPage(Page_SnapshotSelection, new SnapshotSelectionPage(this));
    setPage(Page_FileSelection, new FileSelectionPage(this));
    setPage(Page_RestoreOptions, new RestoreOptionsPage(this));
    setPage(Page_Confirm, new RestoreConfirmPage(this));

    setStartId(Page_SnapshotSelection);

    resize(900, 700);
}

RestoreWizard::~RestoreWizard()
{
}

void RestoreWizard::setPresetTargetPath(const QString& path)
{
    m_presetTargetPath = path;
}

int RestoreWizard::getRepositoryId() const
{
    return field("repositoryId").toInt();
}

QString RestoreWizard::getSnapshotId() const
{
    return field("snapshotId").toString();
}

QStringList RestoreWizard::getSelectedPaths() const
{
    return field("selectedPaths").toStringList();
}

Models::RestoreOptions RestoreWizard::getRestoreOptions() const
{
    Models::RestoreOptions options;
    options.targetPath = field("targetPath").toString();
    options.includePaths = field("selectedPaths").toStringList();

    // 如果有额外的包含路径
    QString additionalPaths = field("additionalIncludePaths").toString();
    if (!additionalPaths.isEmpty()) {
        options.includePaths.append(additionalPaths.split(';', Qt::SkipEmptyParts));
    }

    options.restorePermissions = field("restorePermissions").toBool();
    options.restoreTimestamps = field("restoreTimestamps").toBool();
    options.verify = field("verify").toBool();
    options.overwritePolicy = Models::RestoreOptions::Always;
    options.restoreOwnership = false;
    options.sparse = false;

    return options;
}

void RestoreWizard::accept()
{
    // 向导完成，保存所有配置
    m_repositoryId = getRepositoryId();
    m_snapshotId = getSnapshotId();
    m_selectedPaths = getSelectedPaths();
    m_options = getRestoreOptions();

    QDialog::accept();
}

// ============================================================================
// SnapshotSelectionPage - 步骤1：选择快照
// ============================================================================

SnapshotSelectionPage::SnapshotSelectionPage(QWidget* parent)
    : QWizardPage(parent)
    , m_repositoryComboBox(nullptr)
    , m_refreshButton(nullptr)
    , m_snapshotTable(nullptr)
    , m_infoLabel(nullptr)
    , m_currentRepositoryId(-1)
    , m_isLoading(false)
    , m_snapshotWatcher(nullptr)
{
    setTitle(tr("步骤 1/4: 选择快照"));
    setSubTitle(tr("请选择要恢复的仓库和快照"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    // 提示信息
    m_infoLabel = new QLabel(tr("请先选择一个仓库，然后从列表中选择要恢复的快照"), this);
    m_infoLabel->setWordWrap(true);
    layout->addWidget(m_infoLabel);

    // 仓库选择行
    QHBoxLayout* repoLayout = new QHBoxLayout();
    QLabel* repoLabel = new QLabel(tr("仓库:"), this);
    m_repositoryComboBox = new QComboBox(this);
    m_refreshButton = new QPushButton(tr("刷新"), this);
    repoLayout->addWidget(repoLabel);
    repoLayout->addWidget(m_repositoryComboBox, 1);
    repoLayout->addWidget(m_refreshButton);
    layout->addLayout(repoLayout);

    // 快照表格
    m_snapshotTable = new QTableWidget(this);
    m_snapshotTable->setColumnCount(6);
    m_snapshotTable->setHorizontalHeaderLabels(
        QStringList() << tr("快照ID") << tr("创建时间") << tr("主机名")
                      << tr("路径") << tr("大小") << tr("标签"));
    m_snapshotTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_snapshotTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_snapshotTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_snapshotTable->setAlternatingRowColors(true);
    m_snapshotTable->horizontalHeader()->setStretchLastSection(true);
    m_snapshotTable->setColumnWidth(0, 150);
    m_snapshotTable->setColumnWidth(1, 150);
    m_snapshotTable->setColumnWidth(2, 120);
    m_snapshotTable->setColumnWidth(3, 200);
    m_snapshotTable->setColumnWidth(4, 100);
    layout->addWidget(m_snapshotTable);

    // 注册字段
    registerField("repositoryId", this, "repositoryId");
    registerField("snapshotId", this, "snapshotId");
    registerField("snapshotInfo", this, "snapshotInfo");

    // 初始化异步加载器
    m_snapshotWatcher = new QFutureWatcher<QList<Models::Snapshot>>(this);
    connect(m_snapshotWatcher, &QFutureWatcher<QList<Models::Snapshot>>::finished,
            this, &SnapshotSelectionPage::onSnapshotsLoaded);

    // 连接信号
    connect(m_repositoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SnapshotSelectionPage::onRepositoryChanged);
    connect(m_refreshButton, &QPushButton::clicked, this, &SnapshotSelectionPage::onRefresh);
    connect(m_snapshotTable, &QTableWidget::currentCellChanged,
            this, [this](int currentRow, int, int previousRow, int) {
        onSnapshotSelected(currentRow, previousRow);
    });

    setLayout(layout);
}

SnapshotSelectionPage::~SnapshotSelectionPage()
{
}

void SnapshotSelectionPage::initializePage()
{
    loadRepositories();
}

bool SnapshotSelectionPage::isComplete() const
{
    return m_snapshotTable->currentRow() >= 0;
}

int SnapshotSelectionPage::nextId() const
{
    return RestoreWizard::Page_FileSelection;
}

void SnapshotSelectionPage::loadRepositories()
{
    QSignalBlocker blocker(m_repositoryComboBox);
    m_repositoryComboBox->clear();

    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    if (repositories.isEmpty()) {
        m_repositoryComboBox->addItem(tr("(无仓库)"), -1);
        m_currentRepositoryId = -1;
        return;
    }

    for (const Models::Repository& repo : repositories) {
        m_repositoryComboBox->addItem(repo.name, repo.id);
    }

    Models::Repository defaultRepo = repoMgr->getDefaultRepository();
    if (defaultRepo.id > 0) {
        int index = m_repositoryComboBox->findData(defaultRepo.id);
        if (index >= 0) {
            m_repositoryComboBox->setCurrentIndex(index);
            m_currentRepositoryId = defaultRepo.id;
        }
    } else if (m_repositoryComboBox->count() > 0) {
        m_currentRepositoryId = m_repositoryComboBox->itemData(0).toInt();
    }

    blocker.unblock();
    loadSnapshots();
}

void SnapshotSelectionPage::onRepositoryChanged(int index)
{
    m_currentRepositoryId = m_repositoryComboBox->itemData(index).toInt();
    m_isLoading = false;
    loadSnapshots();
}

void SnapshotSelectionPage::onRefresh()
{
    loadSnapshots();
}

void SnapshotSelectionPage::loadSnapshots()
{
    if (m_currentRepositoryId <= 0) {
        m_snapshotTable->setRowCount(0);
        return;
    }

    if (m_isLoading) {
        return;
    }

    // 获取仓库密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    if (!passMgr->hasPassword(m_currentRepositoryId)) {
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        Models::Repository repo = repoMgr->getRepository(m_currentRepositoryId);

        bool ok;
        QString password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

        if (!ok || password.isEmpty()) {
            m_snapshotTable->setRowCount(0);
            return;
        }

        passMgr->setPassword(m_currentRepositoryId, password);
    }

    if (m_snapshotWatcher->isRunning()) {
        m_snapshotWatcher->cancel();
        m_snapshotWatcher->waitForFinished();
    }

    m_isLoading = true;
    m_snapshotTable->setRowCount(0);
    m_snapshotTable->setEnabled(false);

    int repoId = m_currentRepositoryId;
    QFuture<QList<Models::Snapshot>> future = QtConcurrent::run([repoId]() {
        Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
        return snapshotMgr->listSnapshots(repoId, true);
    });

    m_snapshotWatcher->setFuture(future);
}

void SnapshotSelectionPage::onSnapshotsLoaded()
{
    m_isLoading = false;
    m_snapshotTable->setEnabled(true);

    QList<Models::Snapshot> snapshots = m_snapshotWatcher->result();
    displaySnapshots(snapshots);
}

void SnapshotSelectionPage::displaySnapshots(const QList<Models::Snapshot>& snapshots)
{
    m_snapshotTable->setRowCount(0);

    if (snapshots.isEmpty()) {
        return;
    }

    m_snapshotTable->setRowCount(snapshots.size());

    for (int i = 0; i < snapshots.size(); ++i) {
        const Models::Snapshot& snapshot = snapshots[snapshots.size() - 1 - i];

        QString displayId = snapshot.id.left(8);
        QTableWidgetItem* idItem = new QTableWidgetItem(displayId);
        idItem->setData(Qt::UserRole, snapshot.fullId.isEmpty() ? snapshot.id : snapshot.fullId);
        m_snapshotTable->setItem(i, 0, idItem);

        QTableWidgetItem* timeItem = new QTableWidgetItem(snapshot.time.toString("yyyy-MM-dd HH:mm:ss"));
        m_snapshotTable->setItem(i, 1, timeItem);

        QTableWidgetItem* hostnameItem = new QTableWidgetItem(snapshot.hostname);
        m_snapshotTable->setItem(i, 2, hostnameItem);

        QString pathsText = snapshot.paths.isEmpty() ? tr("(无路径)") : snapshot.paths.first();
        if (snapshot.paths.size() > 1) {
            pathsText += QString(" (+%1)").arg(snapshot.paths.size() - 1);
        }
        QTableWidgetItem* pathsItem = new QTableWidgetItem(pathsText);
        pathsItem->setToolTip(snapshot.paths.join("\n"));
        m_snapshotTable->setItem(i, 3, pathsItem);

        QString sizeText;
        qint64 size = snapshot.size;
        if (size >= 1024LL * 1024 * 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0 * 1024.0 * 1024.0), 'f', 2) + " TB";
        } else if (size >= 1024LL * 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
        } else if (size >= 1024LL * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
        } else if (size >= 1024) {
            sizeText = QString::number(size / 1024.0, 'f', 2) + " KB";
        } else if (size > 0) {
            sizeText = QString::number(size) + " B";
        } else {
            sizeText = "-";
        }
        QTableWidgetItem* sizeItem = new QTableWidgetItem(sizeText);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_snapshotTable->setItem(i, 4, sizeItem);

        QTableWidgetItem* tagsItem = new QTableWidgetItem(snapshot.tags.join(", "));
        m_snapshotTable->setItem(i, 5, tagsItem);
    }

    m_snapshotTable->resizeColumnsToContents();
}

void SnapshotSelectionPage::onSnapshotSelected(int currentRow, int previousRow)
{
    Q_UNUSED(previousRow);

    if (currentRow < 0) {
        setField("repositoryId", -1);
        setField("snapshotId", QString());
        setField("snapshotInfo", QString());
    } else {
        QTableWidgetItem* idItem = m_snapshotTable->item(currentRow, 0);
        QTableWidgetItem* timeItem = m_snapshotTable->item(currentRow, 1);
        QTableWidgetItem* sizeItem = m_snapshotTable->item(currentRow, 4);

        if (idItem) {
            setField("repositoryId", m_currentRepositoryId);
            setField("snapshotId", idItem->data(Qt::UserRole).toString());

            QString info = QString("%1 | 时间: %2 | 大小: %3")
                .arg(idItem->text())
                .arg(timeItem ? timeItem->text() : "-")
                .arg(sizeItem ? sizeItem->text() : "-");
            setField("snapshotInfo", info);
        }
    }

    emit completeChanged();
}

// ============================================================================
// FileSelectionPage - 步骤2：文件选择
// ============================================================================

FileSelectionPage::FileSelectionPage(QWidget* parent)
    : QWizardPage(parent)
    , m_searchEdit(nullptr)
    , m_treeWidget(nullptr)
    , m_statusLabel(nullptr)
    , m_selectionLabel(nullptr)
    , m_selectAllButton(nullptr)
    , m_selectNoneButton(nullptr)
    , m_expandAllButton(nullptr)
    , m_collapseAllButton(nullptr)
    , m_repoId(-1)
    , m_snapshotId()
    , m_isLoading(false)
    , m_fileWatcher(nullptr)
    , m_currentLoadingItem(nullptr)
{
    setTitle(tr("步骤 2/4: 选择要恢复的文件"));
    setSubTitle(tr("请勾选要恢复的文件和目录"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    // 快照信息标签
    QLabel* infoLabel = new QLabel(this);
    infoLabel->setObjectName("snapshotInfoLabel");
    infoLabel->setWordWrap(true);
    QFont boldFont = infoLabel->font();
    boldFont.setBold(true);
    infoLabel->setFont(boldFont);
    layout->addWidget(infoLabel);

    // 搜索框
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel(tr("搜索:"), this);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("输入文件名或路径..."));
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchEdit);
    layout->addLayout(searchLayout);

    // 文件树（带复选框）
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels(QStringList() << tr("名称") << tr("大小") << tr("类型") << tr("修改时间"));
    m_treeWidget->setColumnWidth(0, 350);
    m_treeWidget->setColumnWidth(1, 100);
    m_treeWidget->setColumnWidth(2, 80);
    m_treeWidget->setColumnWidth(3, 150);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSortingEnabled(false);
    layout->addWidget(m_treeWidget);

    // 状态栏和选择统计
    QHBoxLayout* statsLayout = new QHBoxLayout();
    m_statusLabel = new QLabel(tr("准备加载..."), this);
    m_selectionLabel = new QLabel(tr("已选择: 0个文件，0个文件夹 | 总大小: 0 B"), this);
    statsLayout->addWidget(m_statusLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(m_selectionLabel);
    layout->addLayout(statsLayout);

    // 操作按钮行
    QHBoxLayout* actionLayout = new QHBoxLayout();
    m_selectAllButton = new QPushButton(tr("全选"), this);
    m_selectNoneButton = new QPushButton(tr("全不选"), this);
    m_expandAllButton = new QPushButton(tr("展开全部"), this);
    m_collapseAllButton = new QPushButton(tr("折叠全部"), this);
    actionLayout->addWidget(m_selectAllButton);
    actionLayout->addWidget(m_selectNoneButton);
    actionLayout->addWidget(m_expandAllButton);
    actionLayout->addWidget(m_collapseAllButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    // 注册字段
    registerField("selectedPaths", this, "selectedPaths");

    // 初始化异步加载器
    m_fileWatcher = new QFutureWatcher<QList<Models::FileInfo>>(this);
    connect(m_fileWatcher, &QFutureWatcher<QList<Models::FileInfo>>::finished,
            this, &FileSelectionPage::onFilesLoaded);

    // 连接信号
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, &FileSelectionPage::onItemExpanded);
    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &FileSelectionPage::onItemChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &FileSelectionPage::onSearchTextChanged);
    connect(m_selectAllButton, &QPushButton::clicked, this, &FileSelectionPage::onSelectAll);
    connect(m_selectNoneButton, &QPushButton::clicked, this, &FileSelectionPage::onSelectNone);
    connect(m_expandAllButton, &QPushButton::clicked, this, &FileSelectionPage::onExpandAll);
    connect(m_collapseAllButton, &QPushButton::clicked, this, &FileSelectionPage::onCollapseAll);

    setLayout(layout);
}

FileSelectionPage::~FileSelectionPage()
{
}

void FileSelectionPage::initializePage()
{
    m_repoId = field("repositoryId").toInt();
    m_snapshotId = field("snapshotId").toString();
    QString snapshotInfo = field("snapshotInfo").toString();

    // 更新标签
    QLabel* infoLabel = findChild<QLabel*>("snapshotInfoLabel");
    if (infoLabel) {
        infoLabel->setText(tr("快照: %1").arg(snapshotInfo));
    }

    // 延迟加载根目录
    QTimer::singleShot(100, this, &FileSelectionPage::loadRootFiles);
}

bool FileSelectionPage::isComplete() const
{
    // 至少要选择一个文件或目录
    QStringList selectedPaths = field("selectedPaths").toStringList();
    return !selectedPaths.isEmpty();
}

int FileSelectionPage::nextId() const
{
    return RestoreWizard::Page_RestoreOptions;
}

void FileSelectionPage::loadRootFiles()
{
    if (m_isLoading) {
        return;
    }

    m_isLoading = true;
    m_currentLoadingItem = nullptr;
    m_statusLabel->setText(tr("正在加载快照根目录..."));
    m_treeWidget->clear();

    int repoId = m_repoId;
    QString snapshotId = m_snapshotId;

    QFuture<QList<Models::FileInfo>> future = QtConcurrent::run([repoId, snapshotId]() {
        Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
        return snapshotMgr->listFiles(repoId, snapshotId, QString());
    });

    m_fileWatcher->setFuture(future);
}

void FileSelectionPage::loadDirectoryFiles(QTreeWidgetItem* item, const QString& path)
{
    if (m_isLoading) {
        return;
    }

    m_isLoading = true;
    m_currentLoadingItem = item;
    m_statusLabel->setText(tr("正在加载目录: %1").arg(path));

    int repoId = m_repoId;
    QString snapshotId = m_snapshotId;

    QFuture<QList<Models::FileInfo>> future = QtConcurrent::run([repoId, snapshotId, path]() {
        Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
        return snapshotMgr->listFiles(repoId, snapshotId, path);
    });

    m_fileWatcher->setFuture(future);
}

void FileSelectionPage::onFilesLoaded()
{
    m_isLoading = false;
    QList<Models::FileInfo> files = m_fileWatcher->result();

    if (m_currentLoadingItem == nullptr) {
        addFileItems(nullptr, files);
        m_statusLabel->setText(tr("已加载 %1 个文件/目录").arg(files.size()));
    } else {
        m_currentLoadingItem->takeChildren();
        addFileItems(m_currentLoadingItem, files);
        m_statusLabel->setText(tr("已加载 %1 个文件/目录").arg(files.size()));
    }
}

void FileSelectionPage::addFileItems(QTreeWidgetItem* parent, const QList<Models::FileInfo>& files)
{
    QString currentPath;
    if (parent != nullptr) {
        currentPath = parent->data(0, Qt::UserRole).toString();
    }

    QSet<QString> addedPaths;

    for (const Models::FileInfo& fileInfo : files) {
        if (fileInfo.name.isEmpty()) {
            continue;
        }

        // 过滤：只添加当前目录的直接子项
        if (parent == nullptr) {
            QString path = fileInfo.path;
            if (path.startsWith("/")) {
                path = path.mid(1);
            }
            if (path.contains('/')) {
                continue;
            }
        } else {
            if (!fileInfo.path.startsWith(currentPath + "/")) {
                continue;
            }
            QString relativePath = fileInfo.path.mid(currentPath.length() + 1);
            if (relativePath.contains('/')) {
                continue;
            }
        }

        if (addedPaths.contains(fileInfo.path)) {
            continue;
        }
        addedPaths.insert(fileInfo.path);

        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Unchecked);
        item->setText(0, fileInfo.name);
        item->setIcon(0, getFileIcon(fileInfo));

        if (fileInfo.type == Models::FileType::File) {
            item->setText(1, formatFileSize(fileInfo.size));
        } else {
            item->setText(1, "-");
        }

        if (fileInfo.type == Models::FileType::Directory) {
            item->setText(2, tr("文件夹"));
            QTreeWidgetItem* placeholder = new QTreeWidgetItem();
            placeholder->setText(0, tr("加载中..."));
            item->addChild(placeholder);
        } else if (fileInfo.type == Models::FileType::Symlink) {
            item->setText(2, tr("符号链接"));
        } else {
            item->setText(2, tr("文件"));
        }

        if (fileInfo.mtime.isValid()) {
            item->setText(3, fileInfo.mtime.toString("yyyy-MM-dd HH:mm:ss"));
        }

        item->setData(0, Qt::UserRole, fileInfo.path);
        item->setData(0, Qt::UserRole + 1, static_cast<int>(fileInfo.type));

        if (parent == nullptr) {
            m_treeWidget->addTopLevelItem(item);
        } else {
            parent->addChild(item);
        }
    }
}

void FileSelectionPage::onItemExpanded(QTreeWidgetItem* item)
{
    if (item->childCount() == 1 && item->child(0)->text(0) == tr("加载中...")) {
        QString path = item->data(0, Qt::UserRole).toString();
        int typeInt = item->data(0, Qt::UserRole + 1).toInt();
        Models::FileType type = static_cast<Models::FileType>(typeInt);

        if (type == Models::FileType::Directory) {
            loadDirectoryFiles(item, path);
        }
    }
}

void FileSelectionPage::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != 0) {
        return;
    }

    Qt::CheckState state = item->checkState(0);

    m_treeWidget->blockSignals(true);

    std::function<void(QTreeWidgetItem*)> setChildrenState = [&](QTreeWidgetItem* parent) {
        for (int i = 0; i < parent->childCount(); ++i) {
            QTreeWidgetItem* child = parent->child(i);
            if (child->text(0) != tr("加载中...")) {
                child->setCheckState(0, state);
                setChildrenState(child);
            }
        }
    };
    setChildrenState(item);

    m_treeWidget->blockSignals(false);

    updateSelectionStats();
}

void FileSelectionPage::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    // 简单实现：搜索功能可选
}

void FileSelectionPage::onSelectAll()
{
    m_treeWidget->blockSignals(true);
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        setItemChecked(m_treeWidget->topLevelItem(i), true);
    }
    m_treeWidget->blockSignals(false);
    updateSelectionStats();
}

void FileSelectionPage::onSelectNone()
{
    m_treeWidget->blockSignals(true);
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        setItemChecked(m_treeWidget->topLevelItem(i), false);
    }
    m_treeWidget->blockSignals(false);
    updateSelectionStats();
}

void FileSelectionPage::onExpandAll()
{
    m_treeWidget->expandAll();
}

void FileSelectionPage::onCollapseAll()
{
    m_treeWidget->collapseAll();
}

void FileSelectionPage::updateSelectionStats()
{
    int fileCount = 0;
    int dirCount = 0;
    qint64 totalSize = 0;
    QStringList selectedPaths;

    std::function<void(QTreeWidgetItem*)> countSelected = [&](QTreeWidgetItem* item) {
        if (item->checkState(0) == Qt::Checked) {
            int typeInt = item->data(0, Qt::UserRole + 1).toInt();
            Models::FileType type = static_cast<Models::FileType>(typeInt);
            QString path = item->data(0, Qt::UserRole).toString();

            if (!path.isEmpty()) {
                selectedPaths.append(path);
            }

            if (type == Models::FileType::Directory) {
                dirCount++;
            } else if (type == Models::FileType::File) {
                fileCount++;
                QString sizeText = item->text(1);
                if (!sizeText.isEmpty() && sizeText != "-") {
                    qint64 size = 0;
                    if (sizeText.endsWith(" MB")) {
                        size = (qint64)(sizeText.left(sizeText.length() - 3).toDouble() * 1024 * 1024);
                    } else if (sizeText.endsWith(" KB")) {
                        size = (qint64)(sizeText.left(sizeText.length() - 3).toDouble() * 1024);
                    } else if (sizeText.endsWith(" B")) {
                        size = sizeText.left(sizeText.length() - 2).toLongLong();
                    } else if (sizeText.endsWith(" GB")) {
                        size = (qint64)(sizeText.left(sizeText.length() - 3).toDouble() * 1024 * 1024 * 1024);
                    } else if (sizeText.endsWith(" TB")) {
                        size = (qint64)(sizeText.left(sizeText.length() - 3).toDouble() * 1024LL * 1024 * 1024 * 1024);
                    }
                    totalSize += size;
                }
            }
        }

        for (int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem* child = item->child(i);
            if (child->text(0) != tr("加载中...")) {
                countSelected(child);
            }
        }
    };

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        countSelected(m_treeWidget->topLevelItem(i));
    }

    m_selectionLabel->setText(tr("已选择: %1个文件，%2个文件夹 | 总大小: %3")
        .arg(fileCount)
        .arg(dirCount)
        .arg(formatFileSize(totalSize)));

    setField("selectedPaths", selectedPaths);
    emit completeChanged();
}

void FileSelectionPage::setItemChecked(QTreeWidgetItem* item, bool checked, bool updateChildren)
{
    if (!item) {
        return;
    }

    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);

    if (updateChildren) {
        for (int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem* child = item->child(i);
            if (child->text(0) != tr("加载中...")) {
                setItemChecked(child, checked, true);
            }
        }
    }
}

QIcon FileSelectionPage::getFileIcon(const Models::FileInfo& fileInfo)
{
    QStyle* style = QApplication::style();

    if (fileInfo.type == Models::FileType::Directory) {
        return style->standardIcon(QStyle::SP_DirIcon);
    } else if (fileInfo.type == Models::FileType::Symlink) {
        return style->standardIcon(QStyle::SP_FileLinkIcon);
    } else {
        return style->standardIcon(QStyle::SP_FileIcon);
    }
}

QString FileSelectionPage::formatFileSize(qint64 size)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;

    if (size < KB) {
        return QString::number(size) + " B";
    } else if (size < MB) {
        return QString::number(size / KB) + " KB";
    } else if (size < GB) {
        return QString::number(size / MB) + " MB";
    } else if (size < TB) {
        return QString::number(size / GB) + " GB";
    } else {
        return QString::number(size / TB) + " TB";
    }
}

// ============================================================================
// RestoreOptionsPage - 步骤3：恢复选项
// ============================================================================

RestoreOptionsPage::RestoreOptionsPage(QWidget* parent)
    : QWizardPage(parent)
    , m_targetPathEdit(nullptr)
    , m_browseButton(nullptr)
    , m_includeCheckBox(nullptr)
    , m_includeEdit(nullptr)
    , m_restorePermissionsCheckBox(nullptr)
    , m_restoreTimestampsCheckBox(nullptr)
    , m_verifyCheckBox(nullptr)
{
    setTitle(tr("步骤 3/4: 恢复选项"));
    setSubTitle(tr("设置恢复目标路径和其他选项"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    // 目标路径
    QGroupBox* targetGroup = new QGroupBox(tr("恢复目标"), this);
    QVBoxLayout* targetLayout = new QVBoxLayout(targetGroup);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel(tr("恢复到:"), this);
    m_targetPathEdit = new QLineEdit(this);
    m_targetPathEdit->setPlaceholderText(tr("选择恢复目标路径"));
    m_browseButton = new QPushButton(tr("浏览..."), this);
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(m_targetPathEdit, 1);
    pathLayout->addWidget(m_browseButton);
    targetLayout->addLayout(pathLayout);

    layout->addWidget(targetGroup);

    // 其他选项
    QGroupBox* optionsGroup = new QGroupBox(tr("其他选项"), this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);

    // 包含特定文件/目录（附加）
    QHBoxLayout* includeLayout = new QHBoxLayout();
    m_includeCheckBox = new QCheckBox(tr("包含特定文件/目录（附加）"), this);
    m_includeEdit = new QLineEdit(this);
    m_includeEdit->setEnabled(false);
    m_includeEdit->setPlaceholderText(tr("输入额外的包含路径（用分号分隔）"));
    includeLayout->addWidget(m_includeCheckBox);
    includeLayout->addWidget(m_includeEdit, 1);
    optionsLayout->addLayout(includeLayout);

    m_restorePermissionsCheckBox = new QCheckBox(tr("恢复文件权限"), this);
    m_restorePermissionsCheckBox->setChecked(true);
    optionsLayout->addWidget(m_restorePermissionsCheckBox);

    m_restoreTimestampsCheckBox = new QCheckBox(tr("恢复文件时间戳"), this);
    m_restoreTimestampsCheckBox->setChecked(true);
    optionsLayout->addWidget(m_restoreTimestampsCheckBox);

    m_verifyCheckBox = new QCheckBox(tr("恢复后验证数据完整性"), this);
    m_verifyCheckBox->setChecked(false);
    optionsLayout->addWidget(m_verifyCheckBox);

    layout->addWidget(optionsGroup);

    layout->addStretch();

    // 注册字段
    registerField("targetPath*", m_targetPathEdit);
    registerField("additionalIncludePaths", m_includeEdit);
    registerField("restorePermissions", m_restorePermissionsCheckBox);
    registerField("restoreTimestamps", m_restoreTimestampsCheckBox);
    registerField("verify", m_verifyCheckBox);

    // 连接信号
    connect(m_browseButton, &QPushButton::clicked, this, &RestoreOptionsPage::onBrowse);
    connect(m_includeCheckBox, &QCheckBox::toggled, this, &RestoreOptionsPage::onIncludeCheckBoxToggled);

    setLayout(layout);
}

void RestoreOptionsPage::initializePage()
{
    // 获取父向导中的预设路径
    RestoreWizard* wizard = qobject_cast<RestoreWizard*>(this->wizard());
    if (wizard) {
        QString presetPath = wizard->getPresetTargetPath();
        if (!presetPath.isEmpty()) {
            m_targetPathEdit->setText(presetPath);
        }
    }
}

bool RestoreOptionsPage::validatePage()
{
    QString targetPath = m_targetPathEdit->text().trimmed();
    if (targetPath.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入恢复目标路径"));
        return false;
    }

    // 检查目标路径是否存在
    QDir targetDir(targetPath);
    if (!targetDir.exists()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("确认"),
            tr("目标路径不存在，是否创建？\n%1").arg(targetPath),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            if (!targetDir.mkpath(".")) {
                QMessageBox::critical(this, tr("错误"), tr("无法创建目标路径"));
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

int RestoreOptionsPage::nextId() const
{
    return RestoreWizard::Page_Confirm;
}

void RestoreOptionsPage::onBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择恢复目标目录"),
        m_targetPathEdit->text().isEmpty() ? QDir::homePath() : m_targetPathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        m_targetPathEdit->setText(dir);
    }
}

void RestoreOptionsPage::onIncludeCheckBoxToggled(bool checked)
{
    m_includeEdit->setEnabled(checked);
    if (!checked) {
        m_includeEdit->clear();
    }
}

// ============================================================================
// RestoreConfirmPage - 步骤4：确认
// ============================================================================

RestoreConfirmPage::RestoreConfirmPage(QWidget* parent)
    : QWizardPage(parent)
    , m_summaryText(nullptr)
{
    setTitle(tr("步骤 4/4: 确认恢复"));
    setSubTitle(tr("请确认以下信息，然后点击\"完成\"开始恢复"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("即将执行以下恢复操作："), this);
    layout->addWidget(label);

    m_summaryText = new QTextEdit(this);
    m_summaryText->setReadOnly(true);
    layout->addWidget(m_summaryText);

    QLabel* warningLabel = new QLabel(
        tr("<b>警告：</b>恢复操作将覆盖目标路径中的同名文件！"),
        this);
    warningLabel->setStyleSheet("QLabel { color: red; }");
    warningLabel->setWordWrap(true);
    layout->addWidget(warningLabel);

    setLayout(layout);
}

void RestoreConfirmPage::initializePage()
{
    // 收集所有信息并显示摘要
    QString snapshotInfo = field("snapshotInfo").toString();
    QStringList selectedPaths = field("selectedPaths").toStringList();
    QString targetPath = field("targetPath").toString();
    QString additionalPaths = field("additionalIncludePaths").toString();
    bool restorePermissions = field("restorePermissions").toBool();
    bool restoreTimestamps = field("restoreTimestamps").toBool();
    bool verify = field("verify").toBool();

    QString summary;
    summary += tr("快照信息：\n");
    summary += "  " + snapshotInfo + "\n\n";

    summary += tr("选中的文件/目录：\n");
    summary += tr("  共 %1 项：\n").arg(selectedPaths.size());
    int displayCount = qMin(selectedPaths.size(), 10);
    for (int i = 0; i < displayCount; ++i) {
        summary += "  - " + selectedPaths[i] + "\n";
    }
    if (selectedPaths.size() > 10) {
        summary += tr("  ... 还有 %1 项\n").arg(selectedPaths.size() - 10);
    }
    summary += "\n";

    if (!additionalPaths.isEmpty()) {
        summary += tr("额外包含的路径：\n");
        summary += "  " + additionalPaths + "\n\n";
    }

    summary += tr("恢复目标：\n");
    summary += "  " + targetPath + "\n\n";

    summary += tr("恢复选项：\n");
    summary += tr("  恢复文件权限：%1\n").arg(restorePermissions ? tr("是") : tr("否"));
    summary += tr("  恢复文件时间戳：%1\n").arg(restoreTimestamps ? tr("是") : tr("否"));
    summary += tr("  恢复后验证：%1\n").arg(verify ? tr("是") : tr("否"));

    m_summaryText->setPlainText(summary);
}

int RestoreConfirmPage::nextId() const
{
    return -1;  // 最后一页
}

} // namespace UI
} // namespace ResticGUI
