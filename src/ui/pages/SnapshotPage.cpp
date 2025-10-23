#include "SnapshotPage.h"
#include "ui_SnapshotPage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/PasswordManager.h"
#include "../../utils/Logger.h"
#include "../dialogs/SnapshotBrowserDialog.h"
#include <QMessageBox>
#include <QInputDialog>
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
        QString password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

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
        QString password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

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
        QString password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

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

        // 标签
        QString tagsStr = snapshot.tags.join(", ");
        QTableWidgetItem* tagsItem = new QTableWidgetItem(tagsStr);
        ui->tableWidget->setItem(i, 4, tagsItem);
    }
}

void SnapshotPage::showLoadingIndicator(bool show)
{
    if (show) {
        // 清空表格并显示"加载中..."提示
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setColumnCount(5);
        QTableWidgetItem* loadingItem = new QTableWidgetItem(tr("正在加载快照列表，请稍候..."));
        loadingItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, loadingItem);
        ui->tableWidget->setSpan(0, 0, 1, 5); // 合并所有列

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

} // namespace UI
} // namespace ResticGUI
