#include "BackupPage.h"
#include "ui_BackupPage.h"
#include "../dialogs/CreateTaskDialog.h"
#include "../../data/DatabaseManager.h"
#include "../../data/PasswordManager.h"
#include "../../core/RepositoryManager.h"
#include "../../core/BackupManager.h"
#include "../../core/SchedulerManager.h"
#include "../../utils/Logger.h"
#include <QMessageBox>
#include <QInputDialog>

namespace ResticGUI {
namespace UI {

BackupPage::BackupPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BackupPage)
{
    ui->setupUi(this);

    // 连接信号
    connect(ui->createButton, &QPushButton::clicked, this, &BackupPage::onCreateTask);
    connect(ui->editButton, &QPushButton::clicked, this, &BackupPage::onEditTask);
    connect(ui->deleteButton, &QPushButton::clicked, this, &BackupPage::onDeleteTask);
    connect(ui->toggleButton, &QPushButton::clicked, this, &BackupPage::onToggleTask);
    connect(ui->runButton, &QPushButton::clicked, this, &BackupPage::onRunTask);
    connect(ui->refreshButton, &QPushButton::clicked, this, &BackupPage::onRefresh);

    // 连接 BackupManager 信号，当备份完成时自动刷新任务列表
    Core::BackupManager* backupMgr = Core::BackupManager::instance();
    connect(backupMgr, &Core::BackupManager::backupFinished, this, [this](int taskId, bool success) {
        Q_UNUSED(taskId);
        Q_UNUSED(success);
        loadTasks();
    });

    // 监听密码错误信号
    connect(backupMgr, &Core::BackupManager::passwordError, this, &BackupPage::onPasswordError);

    loadTasks();
}

BackupPage::~BackupPage()
{
    delete ui;
}

void BackupPage::loadTasks()
{
    // 获取所有备份任务
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    QList<Models::BackupTask> tasks = db->getAllBackupTasks();

    // 清空表格
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(tasks.size());

    // 填充表格数据
    for (int i = 0; i < tasks.size(); ++i) {
        const Models::BackupTask& task = tasks[i];

        // 任务名称
        QTableWidgetItem* nameItem = new QTableWidgetItem(task.name);
        ui->tableWidget->setItem(i, 0, nameItem);

        // 获取仓库名称
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        Models::Repository repo = repoMgr->getRepository(task.repositoryId);
        QTableWidgetItem* repoItem = new QTableWidgetItem(repo.name);
        ui->tableWidget->setItem(i, 1, repoItem);

        // 源路径（显示第一个路径，如果有多个则显示数量）
        QString sourcePath = task.sourcePaths.isEmpty() ? tr("无") :
            (task.sourcePaths.size() == 1 ? task.sourcePaths.first() :
             QString("%1 (+%2个)").arg(task.sourcePaths.first()).arg(task.sourcePaths.size() - 1));
        QTableWidgetItem* sourceItem = new QTableWidgetItem(sourcePath);
        ui->tableWidget->setItem(i, 2, sourceItem);

        // 计划
        QString schedule;
        switch (task.schedule.type) {
        case Models::Schedule::Manual:
            schedule = tr("手动");
            break;
        case Models::Schedule::Minutely:
            schedule = tr("每分钟");
            break;
        case Models::Schedule::Hourly:
            schedule = tr("每小时");
            break;
        case Models::Schedule::Daily:
            schedule = tr("每天");
            break;
        case Models::Schedule::Weekly:
            schedule = tr("每周");
            break;
        case Models::Schedule::Monthly:
            schedule = tr("每月");
            break;
        case Models::Schedule::Custom:
            schedule = tr("自定义");
            break;
        default:
            schedule = tr("未设置");
            break;
        }
        QTableWidgetItem* scheduleItem = new QTableWidgetItem(schedule);
        ui->tableWidget->setItem(i, 3, scheduleItem);

        // 上次运行
        QString lastRun = task.lastRun.isValid() ?
            task.lastRun.toString("yyyy-MM-dd HH:mm") : tr("从未");
        QTableWidgetItem* lastRunItem = new QTableWidgetItem(lastRun);
        ui->tableWidget->setItem(i, 4, lastRunItem);

        // 下次运行
        QString nextRun = task.nextRun.isValid() ?
            task.nextRun.toString("yyyy-MM-dd HH:mm") : tr("N/A");
        QTableWidgetItem* nextRunItem = new QTableWidgetItem(nextRun);
        ui->tableWidget->setItem(i, 5, nextRunItem);

        // 启用状态
        QTableWidgetItem* enabledItem = new QTableWidgetItem(task.enabled ? tr("是") : tr("否"));
        ui->tableWidget->setItem(i, 6, enabledItem);

        // 存储任务ID
        nameItem->setData(Qt::UserRole, task.id);
    }
}

void BackupPage::onCreateTask()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "点击创建任务按钮");

    // 检查数据库是否正常
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    if (!db) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "数据库管理器未初始化！");
        QMessageBox::critical(this, tr("错误"),
            tr("数据库管理器未初始化，请重启应用程序！"));
        return;
    }

    // 检查是否有可用的仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("当前仓库数量: %1").arg(repositories.size()));

    if (repositories.isEmpty()) {
        Utils::Logger::instance()->log(Utils::Logger::Warning, "没有可用的仓库");
        QMessageBox::warning(this, tr("警告"),
            tr("请先创建至少一个仓库！\n\n您可以在\"仓库管理\"页面创建新仓库。"));
        return;
    }

    // 创建任务对话框
    CreateTaskDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        Models::BackupTask task = dialog.getTask();

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("从对话框获取任务: name=%1, repoId=%2, paths=%3")
                .arg(task.name).arg(task.repositoryId).arg(task.sourcePaths.join(",")));

        // 保存到数据库
        Data::DatabaseManager* db = Data::DatabaseManager::instance();
        int taskId = db->insertBackupTask(task);

        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("数据库插入返回ID: %1").arg(taskId));

        if (taskId > 0) {
            Utils::Logger::instance()->log(Utils::Logger::Info,
                QString("任务 \"%1\" 创建成功，ID=%2").arg(task.name).arg(taskId));

            // 如果任务有调度计划且启用，计算下次运行时间
            if (task.enabled &&
                task.schedule.type != Models::Schedule::None &&
                task.schedule.type != Models::Schedule::Manual) {
                Core::SchedulerManager::instance()->updateTaskNextRun(taskId);
            }

            QMessageBox::information(this, tr("成功"),
                tr("备份任务 \"%1\" 创建成功！").arg(task.name));
            loadTasks();
        } else {
            QString dbError = db->lastError();
            Utils::Logger::instance()->log(Utils::Logger::Error,
                QString("创建任务失败，数据库错误: %1").arg(dbError));
            QMessageBox::critical(this, tr("错误"),
                tr("创建备份任务失败！\n\n数据库错误: %1").arg(dbError));
        }
    } else {
        Utils::Logger::instance()->log(Utils::Logger::Info, "用户取消了创建任务");
    }
}

void BackupPage::onEditTask()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个任务"));
        return;
    }

    // 获取任务ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
    int taskId = nameItem->data(Qt::UserRole).toInt();

    // 获取任务信息
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    Models::BackupTask task = db->getBackupTask(taskId);

    if (task.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取任务信息"));
        return;
    }

    // 打开编辑对话框
    CreateTaskDialog dialog(this);
    dialog.setTask(task);
    dialog.setWindowTitle(tr("编辑备份任务"));

    if (dialog.exec() == QDialog::Accepted) {
        Models::BackupTask updatedTask = dialog.getTask();
        updatedTask.id = taskId; // 保持ID不变

        if (db->updateBackupTask(updatedTask)) {
            // 如果任务有调度计划且启用，重新计算下次运行时间
            if (updatedTask.enabled &&
                updatedTask.schedule.type != Models::Schedule::None &&
                updatedTask.schedule.type != Models::Schedule::Manual) {
                Core::SchedulerManager::instance()->updateTaskNextRun(taskId);
            } else {
                // 如果任务被禁用或改为手动，从调度器中移除
                Core::SchedulerManager::instance()->removeTask(taskId);
            }

            QMessageBox::information(this, tr("成功"), tr("任务已更新"));
            loadTasks();
        } else {
            QMessageBox::critical(this, tr("错误"), tr("更新任务失败"));
        }
    }
}

void BackupPage::onToggleTask()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个任务"));
        return;
    }

    // 获取任务ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
    int taskId = nameItem->data(Qt::UserRole).toInt();

    // 获取任务信息
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    Models::BackupTask task = db->getBackupTask(taskId);

    if (task.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取任务信息"));
        return;
    }

    // 切换启用状态
    task.enabled = !task.enabled;

    // 更新数据库
    if (!db->updateBackupTask(task)) {
        QMessageBox::critical(this, tr("错误"), tr("更新任务失败"));
        return;
    }

    // 更新调度器
    if (task.enabled &&
        task.schedule.type != Models::Schedule::None &&
        task.schedule.type != Models::Schedule::Manual) {
        // 任务被启用且有调度计划，添加到调度器
        Core::SchedulerManager::instance()->updateTaskNextRun(taskId);
        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("任务 \"%1\" 已启用").arg(task.name));
    } else {
        // 任务被禁用或是手动任务，从调度器中移除
        Core::SchedulerManager::instance()->removeTask(taskId);
        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("任务 \"%1\" 已禁用").arg(task.name));
    }

    // 提示用户
    QString status = task.enabled ? tr("启用") : tr("禁用");
    QMessageBox::information(this, tr("成功"),
        tr("任务 \"%1\" 已%2").arg(task.name).arg(status));

    // 刷新任务列表
    loadTasks();
}

void BackupPage::onDeleteTask()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个任务"));
        return;
    }

    // 获取任务ID和名称
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
    int taskId = nameItem->data(Qt::UserRole).toInt();
    QString taskName = nameItem->text();

    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除任务 \"%1\" 吗？").arg(taskName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        Data::DatabaseManager* db = Data::DatabaseManager::instance();
        if (db->deleteBackupTask(taskId)) {
            // 从调度器中移除任务
            Core::SchedulerManager::instance()->removeTask(taskId);

            QMessageBox::information(this, tr("成功"), tr("任务已删除"));
            loadTasks();
        } else {
            QMessageBox::critical(this, tr("错误"), tr("删除任务失败"));
        }
    }
}

void BackupPage::onRunTask()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个任务"));
        return;
    }

    // 获取任务ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
    int taskId = nameItem->data(Qt::UserRole).toInt();

    // 获取任务信息
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    Models::BackupTask task = db->getBackupTask(taskId);

    if (task.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取任务信息"));
        return;
    }

    // 获取仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(task.repositoryId);

    if (repo.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取仓库信息"));
        return;
    }

    // 检查密码是否存在，如果不存在则提示输入
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    if (!passMgr->hasPassword(task.repositoryId)) {
        bool ok;
        QString password = QInputDialog::getText(
            this,
            tr("输入仓库密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password,
            QString(),
            &ok
        );

        if (!ok || password.isEmpty()) {
            Utils::Logger::instance()->log(Utils::Logger::Info, "用户取消输入密码");
            return;
        }

        // 保存密码到密码管理器
        if (!passMgr->setPassword(task.repositoryId, password)) {
            QMessageBox::critical(this, tr("错误"), tr("保存密码失败"));
            return;
        }

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("已为仓库 %1 设置密码").arg(repo.name));
    }

    // 执行备份
    Core::BackupManager* backupMgr = Core::BackupManager::instance();
    bool started = backupMgr->runBackupTask(taskId);

    if (started) {
        QMessageBox::information(this, tr("开始备份"),
            tr("备份任务 \"%1\" 已开始执行。\n\n"
               "请在日志中查看备份进度。").arg(task.name));
    } else {
        QMessageBox::critical(this, tr("错误"),
            tr("无法启动备份任务 \"%1\"。\n\n"
               "可能的原因：\n"
               "1. 已有备份任务正在运行\n"
               "2. 仓库不存在或配置错误\n"
               "3. 密码验证失败\n\n"
               "请查看日志了解详情。").arg(task.name));
    }
}

void BackupPage::onRefresh()
{
    loadTasks();
}

void BackupPage::onPasswordError(int taskId, int repoId)
{
    Q_UNUSED(taskId);

    // 获取仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        return;
    }

    // 提示用户密码错误
    QMessageBox::warning(this, tr("密码错误"),
        tr("仓库 \"%1\" 的密码不正确。\n\n"
           "错误的密码已被清除，下次运行时将提示重新输入。").arg(repo.name));

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("已清除仓库 %1 的错误密码").arg(repo.name));
}

} // namespace UI
} // namespace ResticGUI
