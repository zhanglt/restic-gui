#include "SnapshotPage.h"
#include "ui_SnapshotPage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/PasswordManager.h"
#include <QMessageBox>
#include <QInputDialog>

namespace ResticGUI {
namespace UI {

SnapshotPage::SnapshotPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SnapshotPage)
    , m_currentRepositoryId(-1)
{
    ui->setupUi(this);

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
    // 手动触发一次快照加载（因为信号被阻塞了，需要手动调用）
    blocker.unblock();

    // 如果有仓库被选中，加载快照
    if (m_currentRepositoryId > 0) {
        loadSnapshots();
    }
}

void SnapshotPage::loadSnapshots()
{
    if (m_currentRepositoryId <= 0) {
        ui->tableWidget->setRowCount(0);
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

    // 获取快照列表
    Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
    QList<Models::Snapshot> snapshots = snapshotMgr->listSnapshots(m_currentRepositoryId, true);

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

void SnapshotPage::onRepositoryChanged(int index)
{
    m_currentRepositoryId = ui->repositoryComboBox->itemData(index).toInt();
    loadSnapshots();
}

void SnapshotPage::onDeleteSnapshot()
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

    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除快照 %1 吗？\n\n警告：此操作不可恢复！").arg(snapshotId.left(8)),
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
    QStringList snapshotIds;
    snapshotIds << snapshotId;
    if (snapshotMgr->deleteSnapshots(m_currentRepositoryId, snapshotIds)) {
        QMessageBox::information(this, tr("成功"),
            tr("快照已删除。\n\n注意：实际的数据将在下次运行 prune 命令后被移除。"));
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

    // TODO: 打开文件浏览对话框
    QMessageBox::information(this, tr("提示"),
        tr("快照文件浏览功能待实现。"));
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

} // namespace UI
} // namespace ResticGUI
