#include "HomePage.h"
#include "ui_HomePage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/DatabaseManager.h"
#include "../../data/PasswordManager.h"
#include "../../data/CacheManager.h"

namespace ResticGUI {
namespace UI {

HomePage::HomePage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);
    loadDashboardData();
}

HomePage::~HomePage()
{
    delete ui;
}

void HomePage::loadDashboardData()
{
    // 获取仓库数量
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();
    int repoCount = repositories.size();

    // 获取备份任务数量
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    QList<Models::BackupTask> tasks = db->getAllBackupTasks();
    int taskCount = tasks.size();

    // 获取快照数量（从缓存读取，不需要密码）
    int snapshotCount = 0;
    Data::CacheManager* cacheMgr = Data::CacheManager::instance();
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();

    for (const auto& repo : repositories) {
        QList<Models::Snapshot> snapshots;

        // 优先从缓存读取
        if (cacheMgr->getCachedSnapshots(repo.id, snapshots)) {
            snapshotCount += snapshots.size();
        }
        // 如果缓存不存在且有密码，则从 SnapshotManager 获取
        else if (passMgr->hasPassword(repo.id)) {
            Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
            snapshots = snapshotMgr->listSnapshots(repo.id, false);
            snapshotCount += snapshots.size();
        }
    }

    // 获取最后备份时间（从备份任务中查找）
    QString lastBackupTime = tr("从未");
    QDateTime latestBackup;

    for (const auto& task : tasks) {
        if (task.lastRun.isValid()) {
            if (!latestBackup.isValid() || task.lastRun > latestBackup) {
                latestBackup = task.lastRun;
            }
        }
    }

    if (latestBackup.isValid()) {
        lastBackupTime = latestBackup.toString("yyyy-MM-dd HH:mm:ss");
    }

    // 更新UI
    ui->repoCountLabel->setText(QString::number(repoCount));
    ui->taskCountLabel->setText(QString::number(taskCount));
    ui->snapshotCountLabel->setText(QString::number(snapshotCount));
    ui->lastBackupLabel->setText(lastBackupTime);
}

} // namespace UI
} // namespace ResticGUI
