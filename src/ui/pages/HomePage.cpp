#include "HomePage.h"
#include "ui_HomePage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../data/DatabaseManager.h"
#include "../../data/PasswordManager.h"
#include "../../data/CacheManager.h"
#include "../wizards/CreateRepoWizard.h"
#include "../dialogs/CreateTaskDialog.h"
#include "../../utils/Logger.h"
#include <QMessageBox>

namespace ResticGUI {
namespace UI {

HomePage::HomePage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);

    // 设置统计卡片样式
    QString cardStyle = "QFrame { "
                        "background-color: #f8f9fa; "
                        "border: 1px solid #dee2e6; "
                        "border-radius: 8px; "
                        "padding: 15px; "
                        "}";
    ui->repoFrame->setStyleSheet(cardStyle);
    ui->taskFrame->setStyleSheet(cardStyle);
    ui->snapshotFrame->setStyleSheet(cardStyle);
    ui->storageFrame->setStyleSheet(cardStyle);

    // 设置按钮样式
    QString buttonStyle = "QPushButton { "
                          "background-color: #007bff; "
                          "color: white; "
                          "border: none; "
                          "border-radius: 6px; "
                          "padding: 15px; "
                          "text-align: center; "
                          "} "
                          "QPushButton:hover { "
                          "background-color: #0056b3; "
                          "} "
                          "QPushButton:pressed { "
                          "background-color: #004494; "
                          "}";
    ui->createRepoButton->setStyleSheet(buttonStyle);
    ui->createTaskButton->setStyleSheet(buttonStyle);
    ui->browseSnapshotsButton->setStyleSheet(buttonStyle);

    // 设置标题数字颜色
    ui->repoCountLabel->setStyleSheet("QLabel { color: #007bff; }");
    ui->taskCountLabel->setStyleSheet("QLabel { color: #28a745; }");
    ui->snapshotCountLabel->setStyleSheet("QLabel { color: #ffc107; }");
    ui->totalStorageLabel->setStyleSheet("QLabel { color: #dc3545; }");

    // 连接按钮信号
    connect(ui->createRepoButton, &QPushButton::clicked, this, &HomePage::onCreateRepository);
    connect(ui->createTaskButton, &QPushButton::clicked, this, &HomePage::onCreateTask);
    connect(ui->browseSnapshotsButton, &QPushButton::clicked, this, &HomePage::onRestoreData);

    // 监听快照更新信号
    Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
    connect(snapshotMgr, &Core::SnapshotManager::snapshotsUpdated,
            this, [this](int) { refreshData(); });

    loadDashboardData();
    loadRecentActivities();
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

    // 获取快照数量和总存储量（从缓存读取，不需要密码）
    int snapshotCount = 0;
    qint64 totalStorage = 0;
    Data::CacheManager* cacheMgr = Data::CacheManager::instance();
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();

    for (const auto& repo : repositories) {
        QList<Models::Snapshot> snapshots;

        // 优先从缓存读取
        if (cacheMgr->getCachedSnapshots(repo.id, snapshots)) {
            snapshotCount += snapshots.size();
            // 累加存储量
            for (const auto& snapshot : snapshots) {
                totalStorage += snapshot.size;
            }
        }
        // 如果缓存不存在且有密码，则从 SnapshotManager 获取
        else if (passMgr->hasPassword(repo.id)) {
            Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
            snapshots = snapshotMgr->listSnapshots(repo.id, false);
            snapshotCount += snapshots.size();
            // 累加存储量
            for (const auto& snapshot : snapshots) {
                totalStorage += snapshot.size;
            }
        }
    }

    // 格式化存储量显示
    QString storageText;
    if (totalStorage >= 1024LL * 1024 * 1024 * 1024) {
        storageText = QString::number(totalStorage / (1024.0 * 1024.0 * 1024.0 * 1024.0), 'f', 2) + " TB";
    } else if (totalStorage >= 1024LL * 1024 * 1024) {
        storageText = QString::number(totalStorage / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    } else if (totalStorage >= 1024 * 1024) {
        storageText = QString::number(totalStorage / (1024.0 * 1024.0), 'f', 2) + " MB";
    } else if (totalStorage >= 1024) {
        storageText = QString::number(totalStorage / 1024.0, 'f', 2) + " KB";
    } else {
        storageText = QString::number(totalStorage) + " B";
    }

    // 更新UI
    ui->repoCountLabel->setText(QString::number(repoCount));
    ui->taskCountLabel->setText(QString::number(taskCount));
    ui->snapshotCountLabel->setText(QString::number(snapshotCount));
    ui->totalStorageLabel->setText(storageText);
}

void HomePage::loadRecentActivities()
{
    // 清空列表
    ui->recentActivitiesList->clear();

    // 获取最近的备份历史
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    QList<Models::BackupResult> recentHistory = db->getRecentBackupHistory(10);

    if (recentHistory.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(tr("暂无备份活动"));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        ui->recentActivitiesList->addItem(item);
        return;
    }

    // 遍历历史记录,添加到列表
    for (const auto& result : recentHistory) {
        QString statusText;
        QString statusIcon;

        // 根据状态设置图标和文本
        switch (result.status) {
        case Models::BackupStatus::Success:
            statusText = tr("成功");
            statusIcon = "[✓]";
            break;
        case Models::BackupStatus::Failed:
            statusText = tr("失败");
            statusIcon = "[✗]";
            break;
        case Models::BackupStatus::Running:
            statusText = tr("运行中");
            statusIcon = "[→]";
            break;
        case Models::BackupStatus::Cancelled:
            statusText = tr("已取消");
            statusIcon = "[!]";
            break;
        }

        // 格式化数据大小
        QString sizeText;
        if (result.dataAdded >= 1024 * 1024 * 1024) {
            sizeText = QString::number(result.dataAdded / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
        } else if (result.dataAdded >= 1024 * 1024) {
            sizeText = QString::number(result.dataAdded / (1024.0 * 1024.0), 'f', 2) + " MB";
        } else if (result.dataAdded >= 1024) {
            sizeText = QString::number(result.dataAdded / 1024.0, 'f', 2) + " KB";
        } else {
            sizeText = QString::number(result.dataAdded) + " B";
        }

        // 构建显示文本
        QString displayText = QString("%1 %2 - %3    %4\n    → 快照ID: %5 | 新增: %6个文件 | %7")
            .arg(statusIcon)
            .arg(result.taskName.isEmpty() ? tr("未知任务") : result.taskName)
            .arg(statusText)
            .arg(result.startTime.toString("yyyy-MM-dd HH:mm"))
            .arg(result.snapshotId.isEmpty() ? tr("无") : result.snapshotId.left(8))
            .arg(result.filesNew)
            .arg(sizeText);

        // 如果失败,添加错误信息
        if (result.status == Models::BackupStatus::Failed && !result.errorMessage.isEmpty()) {
            displayText += QString("\n    → 错误: %1").arg(result.errorMessage);
        }

        QListWidgetItem* item = new QListWidgetItem(displayText);

        // 根据状态设置颜色
        if (result.status == Models::BackupStatus::Success) {
            item->setForeground(QColor(0, 128, 0)); // 绿色
        } else if (result.status == Models::BackupStatus::Failed) {
            item->setForeground(QColor(200, 0, 0)); // 红色
        }

        ui->recentActivitiesList->addItem(item);
    }
}

void HomePage::onCreateRepository()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "首页: 点击创建新仓库按钮");

    CreateRepoWizard wizard(this);
    if (wizard.exec() == QDialog::Accepted) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库创建成功");

        // 刷新统计数据
        loadDashboardData();

        // 通知主窗口切换到仓库管理页面 (索引1)
        emit navigateToPage(1);

        QMessageBox::information(this, tr("成功"), tr("仓库创建成功!"));
    }
}

void HomePage::onCreateTask()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "首页: 点击创建备份任务按钮");

    // 检查是否有仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    if (repositories.isEmpty()) {
        QMessageBox::warning(this, tr("提示"),
            tr("请先创建一个仓库，然后再创建备份任务。"));
        return;
    }

    // 打开创建任务对话框
    CreateTaskDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        Models::BackupTask task = dialog.getTask();

        // 保存到数据库
        Data::DatabaseManager* db = Data::DatabaseManager::instance();
        int taskId = db->insertBackupTask(task);

        if (taskId > 0) {
            Utils::Logger::instance()->log(Utils::Logger::Info,
                QString("任务创建成功，ID: %1").arg(taskId));

            // 刷新统计数据
            loadDashboardData();

            // 通知主窗口切换到备份任务页面 (索引2)
            emit navigateToPage(2);

            QMessageBox::information(this, tr("成功"), tr("备份任务创建成功!"));
        } else {
            Utils::Logger::instance()->log(Utils::Logger::Error,
                QString("任务创建失败: %1").arg(db->lastError()));
            QMessageBox::critical(this, tr("错误"),
                tr("创建备份任务失败!\n\n%1").arg(db->lastError()));
        }
    }
}

void HomePage::onRestoreData()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "首页: 点击恢复数据按钮");

    // 检查是否有仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    if (repositories.isEmpty()) {
        QMessageBox::warning(this, tr("提示"),
            tr("请先创建一个仓库，然后才能恢复数据。"));
        return;
    }

    // 通知主窗口切换到数据恢复页面 (索引4)
    emit navigateToPage(4);
}

void HomePage::refreshData()
{
    loadDashboardData();
    loadRecentActivities();
}

} // namespace UI
} // namespace ResticGUI
