#include "StatsPage.h"
#include "ui_StatsPage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/DatabaseManager.h"
#include "../../data/PasswordManager.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QShowEvent>

namespace ResticGUI {
namespace UI {

StatsPage::StatsPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatsPage)
    , m_firstShow(true)
{
    ui->setupUi(this);

    // 连接信号
    connect(ui->refreshButton, &QPushButton::clicked, this, &StatsPage::loadStats);

    // 不在构造时加载统计信息，等待用户点击页面时再加载
}

StatsPage::~StatsPage()
{
    delete ui;
}

void StatsPage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    // 第一次显示时自动加载统计信息
    if (m_firstShow) {
        m_firstShow = false;
        loadStats();
    }
}

void StatsPage::loadStats()
{
    // 获取仓库统计
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    // 在加载统计信息前，先请求所有需要的密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    for (const auto& repo : repositories) {
        if (!passMgr->hasPassword(repo.id)) {
            bool ok;
            QString password = QInputDialog::getText(this, tr("输入密码"),
                tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
                QLineEdit::Password, QString(), &ok);

            if (ok && !password.isEmpty()) {
                // 保存密码到密码管理器
                passMgr->setPassword(repo.id, password);
            }
            // 如果用户取消或输入为空，继续下一个仓库
        }
    }

    QString statsText;
    statsText += "=================================\n";
    statsText += "        Restic GUI 统计信息\n";
    statsText += "=================================\n\n";

    statsText += QString("仓库总数: %1\n\n").arg(repositories.size());

    // 获取备份任务统计
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    QList<Models::BackupTask> tasks = db->getAllBackupTasks();

    int enabledTasks = 0;
    for (const auto& task : tasks) {
        if (task.enabled) enabledTasks++;
    }

    statsText += QString("备份任务总数: %1\n").arg(tasks.size());
    statsText += QString("  - 已启用: %1\n").arg(enabledTasks);
    statsText += QString("  - 已禁用: %1\n\n").arg(tasks.size() - enabledTasks);

    // 统计快照和备份历史
    int totalSnapshots = 0;
    int totalBackups = 0;
    Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();

    statsText += "各仓库详细信息:\n";
    statsText += "---------------------------------\n\n";

    for (const auto& repo : repositories) {
        statsText += QString("仓库: %1\n").arg(repo.name);
        statsText += QString("  类型: %1\n").arg(repo.typeDisplayName());
        statsText += QString("  路径: %1\n").arg(repo.path);

        // 获取快照数量（仅对有密码的仓库）
        if (passMgr->hasPassword(repo.id)) {
            QList<Models::Snapshot> snapshots = snapshotMgr->listSnapshots(repo.id, false);
            int snapshotCount = snapshots.size();
            totalSnapshots += snapshotCount;

            statsText += QString("  快照数量: %1\n").arg(snapshotCount);

            if (!snapshots.isEmpty()) {
                statsText += QString("  最新快照: %1\n")
                    .arg(snapshots.last().time.toString("yyyy-MM-dd HH:mm:ss"));
            }
        } else {
            statsText += "  快照数量: (需要密码)\n";
        }

        // 统计该仓库的备份任务
        int repoTasks = 0;
        for (const auto& task : tasks) {
            if (task.repositoryId == repo.id) repoTasks++;
        }
        statsText += QString("  关联任务: %1\n").arg(repoTasks);

        // 统计备份次数
        int backupCount = 0;
        for (const auto& task : tasks) {
            if (task.repositoryId == repo.id) {
                QList<Models::BackupResult> history = db->getBackupHistory(task.id, 1000);
                backupCount += history.size();
            }
        }
        totalBackups += backupCount;
        statsText += QString("  备份次数: %1\n").arg(backupCount);

        statsText += "\n";
    }

    statsText += "=================================\n";
    statsText += QString("快照总数: %1\n").arg(totalSnapshots);
    statsText += QString("备份总次数: %1\n").arg(totalBackups);
    statsText += "=================================\n";

    ui->statsTextEdit->setText(statsText);
}

} // namespace UI
} // namespace ResticGUI
