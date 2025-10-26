#include "StatsPage.h"
#include "ui_StatsPage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/DatabaseManager.h"
#include "../../data/PasswordManager.h"
#include "../../utils/Logger.h"
#include "../dialogs/PasswordDialog.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QShowEvent>
#include <QtConcurrent>

namespace ResticGUI {
namespace UI {

StatsPage::StatsPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatsPage)
    , m_firstShow(true)
    , m_statsWatcher(nullptr)
{
    ui->setupUi(this);

    // 初始化异步任务监视器
    m_statsWatcher = new QFutureWatcher<QString>(this);
    connect(m_statsWatcher, &QFutureWatcher<QString>::finished,
            this, &StatsPage::onStatsLoaded);

    // 连接信号
    connect(ui->refreshButton, &QPushButton::clicked, this, &StatsPage::loadStats);

    // 不在构造时加载统计信息，等待用户点击页面时再加载
}

StatsPage::~StatsPage()
{
    // 取消正在运行的任务
    if (m_statsWatcher && m_statsWatcher->isRunning()) {
        m_statsWatcher->cancel();
        m_statsWatcher->waitForFinished();
    }

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
    // 如果已经在加载中，不重复加载
    if (m_statsWatcher && m_statsWatcher->isRunning()) {
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            "统计信息正在加载中，忽略重复请求");
        return;
    }

    // 获取仓库列表
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    // 在开始异步加载前，先请求所有需要的密码（这必须在主线程中进行）
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    for (const auto& repo : repositories) {
        if (!passMgr->hasPassword(repo.id)) {
            bool ok;
            QString password = PasswordDialog::getPassword(this, tr("输入密码"),
                tr("请输入仓库 \"%1\" 的密码：").arg(repo.name), &ok);

            if (ok && !password.isEmpty()) {
                // 保存密码到密码管理器
                passMgr->setPassword(repo.id, password);
            }
            // 如果用户取消或输入为空，继续下一个仓库
        }
    }

    // 显示加载提示
    ui->statsTextEdit->setText(tr("正在加载统计信息，请稍候...\n\n"
                                   "这可能需要几秒钟时间，具体取决于仓库大小和网络速度。"));

    // 禁用刷新按钮
    ui->refreshButton->setEnabled(false);

    Utils::Logger::instance()->log(Utils::Logger::Info, "开始异步加载统计信息");

    // 启动异步任务
    QFuture<QString> future = QtConcurrent::run([this]() {
        return collectStats();
    });

    m_statsWatcher->setFuture(future);
}

QString StatsPage::collectStats()
{
    // 这个方法在后台线程中执行，不能直接访问UI

    QString statsText;
    statsText += "=================================\n";
    statsText += "        Restic GUI 统计信息\n";
    statsText += "=================================\n\n";

    // 获取仓库统计
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    statsText += QString("仓库总数: %1\n\n").arg(repositories.size());

    // 获取备份任务统计
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    QList<Models::BackupTask> tasks = db->getAllBackupTasks();

    int enabledTasks = 0;
    for (const auto& task : tasks) {
        // 添加调试日志
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("任务 \"%1\" (ID: %2) enabled字段值: %3")
                .arg(task.name)
                .arg(task.id)
                .arg(task.enabled ? "true" : "false"));

        if (task.enabled) enabledTasks++;
    }

    statsText += QString("备份任务总数: %1\n").arg(tasks.size());
    statsText += QString("  - 已启用: %1\n").arg(enabledTasks);
    statsText += QString("  - 已禁用: %1\n\n").arg(tasks.size() - enabledTasks);

    // 统计快照和备份历史
    int totalSnapshots = 0;
    int totalBackups = 0;
    Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();

    statsText += "各仓库详细信息:\n";
    statsText += "---------------------------------\n\n";

    for (const auto& repo : repositories) {
        statsText += QString("仓库: %1\n").arg(repo.name);
        statsText += QString("  类型: %1\n").arg(repo.typeDisplayName());
        statsText += QString("  路径: %1\n").arg(repo.path);

        // 获取快照数量（仅对有密码的仓库）
        if (passMgr->hasPassword(repo.id)) {
            try {
                // 这里是耗时操作，但现在在后台线程执行，不会阻塞UI
                QList<Models::Snapshot> snapshots = snapshotMgr->listSnapshots(repo.id, false);
                int snapshotCount = snapshots.size();
                totalSnapshots += snapshotCount;

                statsText += QString("  快照数量: %1\n").arg(snapshotCount);

                if (!snapshots.isEmpty()) {
                    statsText += QString("  最新快照: %1\n")
                        .arg(snapshots.last().time.toString("yyyy-MM-dd HH:mm:ss"));
                }
            } catch (...) {
                statsText += "  快照数量: (加载失败)\n";
                Utils::Logger::instance()->log(Utils::Logger::Warning,
                    QString("加载仓库 %1 的快照失败").arg(repo.name));
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

    return statsText;
}

void StatsPage::onStatsLoaded()
{
    // 这个方法在主线程中执行，可以安全访问UI

    if (m_statsWatcher->isCanceled()) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "统计信息加载已取消");
        ui->statsTextEdit->setText(tr("统计信息加载已取消"));
        ui->refreshButton->setEnabled(true);
        return;
    }

    // 获取结果并更新UI
    QString statsText = m_statsWatcher->result();
    ui->statsTextEdit->setText(statsText);

    // 恢复刷新按钮
    ui->refreshButton->setEnabled(true);

    Utils::Logger::instance()->log(Utils::Logger::Info, "统计信息加载完成");
}

} // namespace UI
} // namespace ResticGUI
