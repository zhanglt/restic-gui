#include "SnapshotPage.h"
#include "ui_SnapshotPage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/PasswordManager.h"
#include "../../utils/Logger.h"
#include "../dialogs/SnapshotBrowserDialog.h"
#include "../dialogs/PasswordDialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QtConcurrent>

namespace ResticGUI {
namespace UI {

SnapshotPage::SnapshotPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SnapshotPage)
    , m_currentRepositoryId(-1)
    , m_firstShow(true)
    , m_isLoading(false)
    , m_snapshotWatcher(nullptr)
{
    ui->setupUi(this);

    // ========== 美化界面样式 ==========

    // 设置按钮样式
    QString dangerButtonStyle = "QPushButton { "
                                 "background-color: #dc3545; "
                                 "color: white; "
                                 "border: none; "
                                 "border-radius: 4px; "
                                 "padding: 6px 12px; "
                                 "min-height: 24px; "
                                 "} "
                                 "QPushButton:hover { "
                                 "background-color: #c82333; "
                                 "} "
                                 "QPushButton:pressed { "
                                 "background-color: #bd2130; "
                                 "}";

    QString primaryButtonStyle = "QPushButton { "
                                  "background-color: #007bff; "
                                  "color: white; "
                                  "border: none; "
                                  "border-radius: 4px; "
                                  "padding: 6px 12px; "
                                  "min-height: 24px; "
                                  "font-weight: bold; "
                                  "} "
                                  "QPushButton:hover { "
                                  "background-color: #0056b3; "
                                  "} "
                                  "QPushButton:pressed { "
                                  "background-color: #004494; "
                                  "}";

    QString secondaryButtonStyle = "QPushButton { "
                                    "background-color: #6c757d; "
                                    "color: white; "
                                    "border: none; "
                                    "border-radius: 4px; "
                                    "padding: 6px 12px; "
                                    "min-height: 24px; "
                                    "} "
                                    "QPushButton:hover { "
                                    "background-color: #5a6268; "
                                    "} "
                                    "QPushButton:pressed { "
                                    "background-color: #545b62; "
                                    "}";

    // 应用按钮样式
    ui->deleteButton->setStyleSheet(dangerButtonStyle);
    ui->browseButton->setStyleSheet(secondaryButtonStyle);
    ui->restoreButton->setStyleSheet(primaryButtonStyle);
    ui->refreshButton->setStyleSheet(secondaryButtonStyle);
    ui->searchButton->setStyleSheet(secondaryButtonStyle);

    // 美化下拉框
    QString comboBoxStyle = "QComboBox { "
                            "background-color: white; "
                            "border: 1px solid #ced4da; "
                            "border-radius: 4px; "
                            "padding: 5px 10px; "
                            "min-height: 24px; "
                            "} "
                            "QComboBox:hover { "
                            "border-color: #80bdff; "
                            "} "
                            "QComboBox::drop-down { "
                            "border: none; "
                            "width: 20px; "
                            "} "
                            "QComboBox::down-arrow { "
                            "image: none; "
                            "border-left: 5px solid transparent; "
                            "border-right: 5px solid transparent; "
                            "border-top: 5px solid #495057; "
                            "margin-right: 5px; "
                            "}";
    ui->repositoryComboBox->setStyleSheet(comboBoxStyle);

    // 美化输入框
    QString lineEditStyle = "QLineEdit { "
                            "background-color: white; "
                            "border: 1px solid #ced4da; "
                            "border-radius: 4px; "
                            "padding: 5px 10px; "
                            "min-height: 24px; "
                            "} "
                            "QLineEdit:focus { "
                            "border-color: #80bdff; "
                            "outline: none; "
                            "}";
    ui->filterEdit->setStyleSheet(lineEditStyle);

    // 美化表格
    QString tableStyle = "QTableWidget { "
                         "background-color: white; "
                         "border: 1px solid #dee2e6; "
                         "border-radius: 6px; "
                         "gridline-color: #dee2e6; "
                         "} "
                         "QTableWidget::item { "
                         "padding: 8px; "
                         "border-bottom: 1px solid #f0f0f0; "
                         "} "
                         "QTableWidget::item:selected { "
                         "background-color: #e7f3ff; "
                         "color: #212529; "
                         "} "
                         "QHeaderView::section { "
                         "background-color: #f8f9fa; "
                         "color: #495057; "
                         "padding: 5px; "
                         "border: none; "
                         "border-right: 1px solid #dee2e6; "
                         "border-bottom: 2px solid #dee2e6; "
                         "font-weight: bold; "
                         "} "
                         "QHeaderView::section:first { "
                         "border-top-left-radius: 6px; "
                         "} "
                         "QHeaderView::section:last { "
                         "border-top-right-radius: 6px; "
                         "border-right: none; "
                         "}";
    ui->tableWidget->setStyleSheet(tableStyle);

    // 美化详情面板
    QString groupBoxStyle = "QGroupBox { "
                            "background-color: #f8f9fa; "
                            "border: 1px solid #dee2e6; "
                            "border-radius: 8px; "
                            "margin-top: 12px; "
                            "padding: 15px; "
                            "font-weight: bold; "
                            "} "
                            "QGroupBox::title { "
                            "subcontrol-origin: margin; "
                            "subcontrol-position: top left; "
                            "padding: 5px 10px; "
                            "color: #495057; "
                            "}";
    ui->detailsGroup->setStyleSheet(groupBoxStyle);

    // 美化详情标签
    QString labelStyle = "QLabel { "
                         "color: #495057; "
                         "font-size: 13px; "
                         "padding: 3px; "
                         "}";

    // 设置标签样式（标题加粗）
    ui->label_snapshotId->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_createTime->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_hostname->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_username->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_paths->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_tags->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_parentId->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_fileCount->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_dirCount->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_size->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");

    // 设置数值标签样式
    ui->detailSnapshotIdLabel->setStyleSheet(labelStyle);
    ui->detailCreateTimeLabel->setStyleSheet(labelStyle);
    ui->detailHostnameLabel->setStyleSheet(labelStyle);
    ui->detailUsernameLabel->setStyleSheet(labelStyle);
    ui->detailPathsLabel->setStyleSheet(labelStyle);
    ui->detailTagsLabel->setStyleSheet(labelStyle);
    ui->detailParentIdLabel->setStyleSheet(labelStyle);
    ui->detailFileCountLabel->setStyleSheet(labelStyle);
    ui->detailDirCountLabel->setStyleSheet(labelStyle);
    ui->detailSizeLabel->setStyleSheet(labelStyle);

    // 设置表格列宽
    ui->tableWidget->setColumnWidth(0, 100);  // 快照ID
    ui->tableWidget->setColumnWidth(1, 150);  // 时间
    ui->tableWidget->setColumnWidth(2, 80);   // 主机名
    ui->tableWidget->setColumnWidth(3, 200);  // 路径
    ui->tableWidget->setColumnWidth(4, 100);  // 大小
    ui->tableWidget->setColumnWidth(5, 120);  // 标签

    // 让路径列可以拉伸填充剩余空间
    ui->tableWidget->horizontalHeader()->setStretchLastSection(false);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    // 初始化异步加载器
    m_snapshotWatcher = new QFutureWatcher<QList<Models::Snapshot>>(this);
    connect(m_snapshotWatcher, &QFutureWatcher<QList<Models::Snapshot>>::finished,
            this, &SnapshotPage::onSnapshotsLoaded);

    // 先加载仓库列表（此时不连接信号，避免触发密码输入）
    loadRepositories();

    // 加载完成后再连接信号
    connect(ui->repositoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SnapshotPage::onRepositoryChanged);
    connect(ui->deleteButton, &QPushButton::clicked, this, &SnapshotPage::onDeleteSnapshot);
    connect(ui->browseButton, &QPushButton::clicked, this, &SnapshotPage::onBrowseSnapshot);
    connect(ui->restoreButton, &QPushButton::clicked, this, &SnapshotPage::onRestoreSnapshot);
    connect(ui->refreshButton, &QPushButton::clicked, this, &SnapshotPage::onRefresh);
    connect(ui->searchButton, &QPushButton::clicked, this, &SnapshotPage::onSearch);

    // 连接表格选中信号
    connect(ui->tableWidget, &QTableWidget::currentCellChanged,
        [this](int currentRow, int currentColumn, int previousRow, int previousColumn) {
            Q_UNUSED(currentColumn);
            Q_UNUSED(previousColumn);
            onSnapshotSelected(currentRow, previousRow);
        });
}

SnapshotPage::~SnapshotPage()
{
    delete ui;
}

void SnapshotPage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    // 第一次显示时自动加载快照
    if (m_firstShow && m_currentRepositoryId > 0) {
        m_firstShow = false;
        loadSnapshots();
    }
}

void SnapshotPage::loadRepositories()
{
    // 使用 QSignalBlocker 临时阻塞信号，避免在填充列表时触发 currentIndexChanged
    QSignalBlocker blocker(ui->repositoryComboBox);

    ui->repositoryComboBox->clear();

    // 获取所有仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    if (repositories.isEmpty()) {
        ui->repositoryComboBox->addItem(tr("(无仓库)"), -1);
        m_currentRepositoryId = -1;
        return;
    }

    // 填充下拉框
    for (const Models::Repository& repo : repositories) {
        ui->repositoryComboBox->addItem(repo.name, repo.id);
    }

    // 如果有默认仓库，选中它
    Models::Repository defaultRepo = repoMgr->getDefaultRepository();
    if (defaultRepo.id > 0) {
        int index = ui->repositoryComboBox->findData(defaultRepo.id);
        if (index >= 0) {
            ui->repositoryComboBox->setCurrentIndex(index);
            m_currentRepositoryId = defaultRepo.id;
        }
    } else if (ui->repositoryComboBox->count() > 0) {
        // 如果没有默认仓库，选择第一个
        m_currentRepositoryId = ui->repositoryComboBox->itemData(0).toInt();
    }

    // blocker 在这里析构，信号解除阻塞
    // 不在构造时自动加载快照，避免启动时不必要的restic命令执行
    // 用户切换到该页面或选择仓库时才会加载快照
    blocker.unblock();
}

void SnapshotPage::loadSnapshots()
{
    if (m_currentRepositoryId <= 0) {
        ui->tableWidget->setRowCount(0);
        return;
    }

    // 防止重复加载
    if (m_isLoading) {
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            "快照正在加载中，忽略重复请求");
        return;
    }

    // 获取仓库密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    if (!passMgr->hasPassword(m_currentRepositoryId)) {
        // 获取仓库名称
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        Models::Repository repo = repoMgr->getRepository(m_currentRepositoryId);

        bool ok;
        QString password = PasswordDialog::getPassword(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(m_currentRepositoryId, password);
    }

    // 如果已经有任务在运行，取消它
    if (m_snapshotWatcher->isRunning()) {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "取消之前的快照加载任务");
        m_snapshotWatcher->cancel();
        m_snapshotWatcher->waitForFinished();
    }

    // 设置加载状态
    m_isLoading = true;

    // 显示加载提示
    showLoadingIndicator(true);

    // 在后台线程异步加载快照列表
    int repoId = m_currentRepositoryId;
    QFuture<QList<Models::Snapshot>> future = QtConcurrent::run([repoId]() {
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("开始异步加载快照，仓库ID: %1").arg(repoId));
        Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
        return snapshotMgr->listSnapshots(repoId, true);
    });

    m_snapshotWatcher->setFuture(future);
}

void SnapshotPage::onRepositoryChanged(int index)
{
    m_currentRepositoryId = ui->repositoryComboBox->itemData(index).toInt();

    // 切换仓库时，重置加载状态以允许加载新仓库的快照
    m_isLoading = false;

    loadSnapshots();
}

void SnapshotPage::onDeleteSnapshot()
{
    // 获取所有选中的行
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要删除的快照"));
        return;
    }

    // 提取所有选中的快照ID（去重，因为每行有多列都会被选中）
    QSet<int> selectedRows;
    for (QTableWidgetItem* item : selectedItems) {
        selectedRows.insert(item->row());
    }

    QStringList snapshotIds;
    QStringList snapshotShortIds;
    for (int row : selectedRows) {
        QTableWidgetItem* idItem = ui->tableWidget->item(row, 0);
        if (idItem) {
            QString snapshotId = idItem->data(Qt::UserRole).toString();
            snapshotIds << snapshotId;
            snapshotShortIds << snapshotId.left(8);
        }
    }

    if (snapshotIds.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("未找到有效的快照"));
        return;
    }

    // 确认删除
    QString confirmMsg;
    if (snapshotIds.size() == 1) {
        confirmMsg = tr("确定要删除快照 %1 吗？\n\n警告：此操作不可恢复！").arg(snapshotShortIds.first());
    } else {
        confirmMsg = tr("确定要删除选中的 %1 个快照吗？\n\n快照ID：\n%2\n\n警告：此操作不可恢复！")
            .arg(snapshotIds.size())
            .arg(snapshotShortIds.join(", "));
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        confirmMsg,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 获取密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    if (!passMgr->hasPassword(m_currentRepositoryId)) {
        // 获取仓库名称
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        Models::Repository repo = repoMgr->getRepository(m_currentRepositoryId);

        bool ok;
        QString password = PasswordDialog::getPassword(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(m_currentRepositoryId, password);
    }

    // 删除快照
    Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
    if (snapshotMgr->deleteSnapshots(m_currentRepositoryId, snapshotIds)) {
        QString successMsg;
        if (snapshotIds.size() == 1) {
            successMsg = tr("快照已删除。\n\n注意：实际的数据将在下次运行 prune 命令后被移除。");
        } else {
            successMsg = tr("已成功删除 %1 个快照。\n\n注意：实际的数据将在下次运行 prune 命令后被移除。").arg(snapshotIds.size());
        }
        QMessageBox::information(this, tr("成功"), successMsg);
        loadSnapshots();
    } else {
        QMessageBox::critical(this, tr("错误"),
            tr("删除快照失败，请查看日志了解详情。"));
    }
}

void SnapshotPage::onBrowseSnapshot()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个快照"));
        return;
    }

    // 获取快照信息
    QTableWidgetItem* idItem = ui->tableWidget->item(currentRow, 0);
    QString snapshotId = idItem->data(Qt::UserRole).toString();

    QTableWidgetItem* timeItem = ui->tableWidget->item(currentRow, 1);
    QString snapshotName = timeItem->text();

    // 检查是否有仓库密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    if (!passMgr->hasPassword(m_currentRepositoryId)) {
        // 获取仓库名称
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        Models::Repository repo = repoMgr->getRepository(m_currentRepositoryId);

        bool ok;
        QString password = PasswordDialog::getPassword(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(m_currentRepositoryId, password);
    }

    // 打开文件浏览对话框
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("打开快照浏览器: %1").arg(snapshotId));

    SnapshotBrowserDialog dialog(m_currentRepositoryId, snapshotId, snapshotName, this);
    dialog.exec();
}

void SnapshotPage::onRestoreSnapshot()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个快照"));
        return;
    }

    // 获取快照ID
    QTableWidgetItem* idItem = ui->tableWidget->item(currentRow, 0);
    QString snapshotId = idItem->data(Qt::UserRole).toString();

    // TODO: 打开恢复对话框
    QMessageBox::information(this, tr("提示"),
        tr("快照恢复功能待实现。\n\n请切换到\"数据恢复\"页面进行恢复操作。"));
}

void SnapshotPage::onRefresh()
{
    loadSnapshots();
}

void SnapshotPage::onSnapshotsLoaded()
{
    // 重置加载状态
    m_isLoading = false;

    // 隐藏加载提示
    showLoadingIndicator(false);

    // 获取异步加载的结果
    QList<Models::Snapshot> snapshots = m_snapshotWatcher->result();

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("已加载 %1 个快照").arg(snapshots.size()));

    // 显示快照列表
    displaySnapshots(snapshots);
}

void SnapshotPage::displaySnapshots(const QList<Models::Snapshot>& snapshots)
{
    // 清空表格
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(snapshots.size());

    // 填充表格
    for (int i = 0; i < snapshots.size(); ++i) {
        const Models::Snapshot& snapshot = snapshots[i];

        // 快照ID（显示短ID）
        QString shortId = snapshot.id.left(8);
        QTableWidgetItem* idItem = new QTableWidgetItem(shortId);
        idItem->setData(Qt::UserRole, snapshot.id); // 存储完整ID
        ui->tableWidget->setItem(i, 0, idItem);

        // 时间
        QString timeStr = snapshot.time.toString("yyyy-MM-dd HH:mm:ss");
        QTableWidgetItem* timeItem = new QTableWidgetItem(timeStr);
        ui->tableWidget->setItem(i, 1, timeItem);

        // 主机名
        QTableWidgetItem* hostnameItem = new QTableWidgetItem(snapshot.hostname);
        ui->tableWidget->setItem(i, 2, hostnameItem);

        // 路径（显示第一个路径，如果有多个则显示数量）
        QString pathStr = snapshot.paths.isEmpty() ? tr("(无)") :
            (snapshot.paths.size() == 1 ? snapshot.paths.first() :
             QString("%1 (+%2个)").arg(snapshot.paths.first()).arg(snapshot.paths.size() - 1));
        QTableWidgetItem* pathItem = new QTableWidgetItem(pathStr);
        ui->tableWidget->setItem(i, 3, pathItem);

        // 大小
        QString sizeText;
        qint64 size = snapshot.size;
        if (size >= 1024LL * 1024 * 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0 * 1024.0 * 1024.0), 'f', 2) + " TB";
        } else if (size >= 1024LL * 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
        } else if (size >= 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
        } else if (size >= 1024) {
            sizeText = QString::number(size / 1024.0, 'f', 2) + " KB";
        } else {
            sizeText = QString::number(size) + " B";
        }
        QTableWidgetItem* sizeItem = new QTableWidgetItem(sizeText);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 4, sizeItem);

        // 标签
        QString tagsStr = snapshot.tags.join(", ");
        QTableWidgetItem* tagsItem = new QTableWidgetItem(tagsStr);
        ui->tableWidget->setItem(i, 5, tagsItem);
    }

    // 清空详情显示
    if (snapshots.isEmpty()) {
        clearDetails();
    }
}

void SnapshotPage::showLoadingIndicator(bool show)
{
    if (show) {
        // 清空表格并显示"加载中..."提示
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setColumnCount(6);
        QTableWidgetItem* loadingItem = new QTableWidgetItem(tr("正在加载快照列表，请稍候..."));
        loadingItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, loadingItem);
        ui->tableWidget->setSpan(0, 0, 1, 6); // 合并所有列

        // 禁用操作按钮
        ui->deleteButton->setEnabled(false);
        ui->browseButton->setEnabled(false);
        ui->restoreButton->setEnabled(false);
        ui->refreshButton->setEnabled(false);
    } else {
        // 恢复表格列数
        ui->tableWidget->clearSpans();
        ui->tableWidget->setRowCount(0);

        // 启用操作按钮
        ui->deleteButton->setEnabled(true);
        ui->browseButton->setEnabled(true);
        ui->restoreButton->setEnabled(true);
        ui->refreshButton->setEnabled(true);
    }
}

void SnapshotPage::onSnapshotSelected(int currentRow, int previousRow)
{
    Q_UNUSED(previousRow);

    if (currentRow < 0) {
        clearDetails();
        return;
    }

    // 获取快照ID
    QTableWidgetItem* idItem = ui->tableWidget->item(currentRow, 0);
    if (!idItem) {
        clearDetails();
        return;
    }

    QString snapshotId = idItem->data(Qt::UserRole).toString();

    // 从表格中获取快照信息
    QTableWidgetItem* timeItem = ui->tableWidget->item(currentRow, 1);
    QTableWidgetItem* hostnameItem = ui->tableWidget->item(currentRow, 2);
    QTableWidgetItem* pathItem = ui->tableWidget->item(currentRow, 3);
    QTableWidgetItem* sizeItem = ui->tableWidget->item(currentRow, 4);
    QTableWidgetItem* tagsItem = ui->tableWidget->item(currentRow, 5);

    // 填充详情面板
    ui->detailSnapshotIdLabel->setText(snapshotId);
    ui->detailCreateTimeLabel->setText(timeItem ? timeItem->text() : "-");
    ui->detailHostnameLabel->setText(hostnameItem ? hostnameItem->text() : "-");
    ui->detailUsernameLabel->setText("-"); // 用户名在表格中没有显示，需要从完整数据获取
    ui->detailPathsLabel->setText(pathItem ? pathItem->text() : "-");
    ui->detailSizeLabel->setText(sizeItem ? sizeItem->text() : "-");
    ui->detailTagsLabel->setText(tagsItem ? tagsItem->text() : tr("无"));
    ui->detailParentIdLabel->setText("-"); // 父快照需要从完整数据获取
    ui->detailFileCountLabel->setText("-"); // 文件数需要从完整数据获取
    ui->detailDirCountLabel->setText("-"); // 目录数需要从完整数据获取

    // TODO: 如果需要更详细的信息，可以从 SnapshotManager 获取完整的快照数据
}

void SnapshotPage::onSearch()
{
    QString filterText = ui->filterEdit->text().trimmed();

    if (filterText.isEmpty()) {
        // 如果筛选为空，重新加载所有快照
        loadSnapshots();
        return;
    }

    // 简单的客户端筛选
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        bool match = false;

        // 检查每一列是否包含筛选文本
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = ui->tableWidget->item(i, col);
            if (item && item->text().contains(filterText, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }

        // 根据匹配结果显示或隐藏行
        ui->tableWidget->setRowHidden(i, !match);
    }
}

void SnapshotPage::clearDetails()
{
    ui->detailSnapshotIdLabel->setText("-");
    ui->detailCreateTimeLabel->setText("-");
    ui->detailHostnameLabel->setText("-");
    ui->detailUsernameLabel->setText("-");
    ui->detailPathsLabel->setText("-");
    ui->detailTagsLabel->setText("-");
    ui->detailParentIdLabel->setText("-");
    ui->detailFileCountLabel->setText("-");
    ui->detailDirCountLabel->setText("-");
    ui->detailSizeLabel->setText("-");
}

} // namespace UI
} // namespace ResticGUI
