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
#include <QHeaderView>

namespace ResticGUI {
namespace UI {

BackupPage::BackupPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::BackupPage)
{
    ui->setupUi(this);

    // ========== 美化界面样式 ==========

    // 设置按钮样式
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

    QString successButtonStyle = "QPushButton { "
                                  "background-color: #28a745; "
                                  "color: white; "
                                  "border: none; "
                                  "border-radius: 4px; "
                                  "padding: 6px 12px; "
                                  "min-height: 24px; "
                                  "font-weight: bold; "
                                  "} "
                                  "QPushButton:hover { "
                                  "background-color: #218838; "
                                  "} "
                                  "QPushButton:pressed { "
                                  "background-color: #1e7e34; "
                                  "}";

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

    // 应用按钮样式
    ui->createButton->setStyleSheet(primaryButtonStyle);
    ui->editButton->setStyleSheet(secondaryButtonStyle);
    ui->deleteButton->setStyleSheet(dangerButtonStyle);
    ui->toggleButton->setStyleSheet(secondaryButtonStyle);
    ui->runButton->setStyleSheet(successButtonStyle);
    ui->refreshButton->setStyleSheet(secondaryButtonStyle);

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
    ui->label_name->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_repo->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_paths->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_excludes->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_tags->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_schedule->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_lastRun->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_nextRun->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");

    // 设置数值标签样式
    ui->detailNameLabel->setStyleSheet(labelStyle);
    ui->detailRepoLabel->setStyleSheet(labelStyle);
    ui->detailPathsLabel->setStyleSheet(labelStyle);
    ui->detailExcludesLabel->setStyleSheet(labelStyle);
    ui->detailTagsLabel->setStyleSheet(labelStyle);
    ui->detailScheduleLabel->setStyleSheet(labelStyle);
    ui->detailLastRunLabel->setStyleSheet(labelStyle);
    ui->detailNextRunLabel->setStyleSheet(labelStyle);

    // 设置表格列宽
    ui->tableWidget->setColumnWidth(0, 150);  // 任务名称
    ui->tableWidget->setColumnWidth(1, 120);  // 目标仓库
    ui->tableWidget->setColumnWidth(2, 200);  // 备份路径
    ui->tableWidget->setColumnWidth(3, 100);  // 调度
    ui->tableWidget->setColumnWidth(4, 80);   // 状态
    ui->tableWidget->setColumnWidth(5, 120);  // 操作

    // 让备份路径列可以拉伸填充剩余空间
    ui->tableWidget->horizontalHeader()->setStretchLastSection(false);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // 连接信号
    connect(ui->createButton, &QPushButton::clicked, this, &BackupPage::onCreateTask);
    connect(ui->editButton, &QPushButton::clicked, this, &BackupPage::onEditTask);
    connect(ui->deleteButton, &QPushButton::clicked, this, &BackupPage::onDeleteTask);
    connect(ui->toggleButton, &QPushButton::clicked, this, &BackupPage::onToggleTask);
    connect(ui->runButton, &QPushButton::clicked, this, &BackupPage::onRunTask);
    connect(ui->refreshButton, &QPushButton::clicked, this, &BackupPage::onRefresh);

    // 连接表格选中信号
    connect(ui->tableWidget, &QTableWidget::currentCellChanged,
        [this](int currentRow, int currentColumn, int previousRow, int previousColumn) {
            Q_UNUSED(currentColumn);
            Q_UNUSED(previousColumn);
            onTaskSelected(currentRow, previousRow);
        });

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

    // 添加调试日志
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("BackupPage: 加载了 %1 个任务").arg(tasks.size()));

    // 清空表格
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(tasks.size());

    // 填充表格数据
    for (int i = 0; i < tasks.size(); ++i) {
        const Models::BackupTask& task = tasks[i];

        // 添加调试日志
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("BackupPage: 任务 \"%1\" (ID: %2) enabled=%3")
                .arg(task.name)
                .arg(task.id)
                .arg(task.enabled ? "true" : "false"));

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

        // 调度
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

        // 状态
        QString status = task.enabled ? tr("启用") : tr("禁用");
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        if (task.enabled) {
            statusItem->setForeground(QColor(40, 167, 69)); // 绿色
        } else {
            statusItem->setForeground(QColor(108, 117, 125)); // 灰色
        }
        ui->tableWidget->setItem(i, 4, statusItem);

        // 操作列（显示文本提示，实际操作在详情面板或顶部按钮）
        QTableWidgetItem* actionItem = new QTableWidgetItem(tr("[详情]"));
        actionItem->setForeground(QColor(0, 123, 255)); // 蓝色
        actionItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 5, actionItem);

        // 存储任务ID
        nameItem->setData(Qt::UserRole, task.id);
    }

    // 清空详情显示
    if (ui->tableWidget->rowCount() == 0) {
        clearDetails();
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

void BackupPage::onTaskSelected(int currentRow, int previousRow)
{
    Q_UNUSED(previousRow);

    if (currentRow < 0) {
        clearDetails();
        return;
    }

    // 获取任务ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
    if (!nameItem) {
        clearDetails();
        return;
    }

    int taskId = nameItem->data(Qt::UserRole).toInt();

    // 获取任务信息
    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    QList<Models::BackupTask> tasks = db->getAllBackupTasks();

    Models::BackupTask task;
    bool found = false;
    for (const auto& t : tasks) {
        if (t.id == taskId) {
            task = t;
            found = true;
            break;
        }
    }

    if (!found) {
        clearDetails();
        return;
    }

    // 填充详情面板

    // 任务名称
    ui->detailNameLabel->setText(task.name);

    // 目标仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(task.repositoryId);
    ui->detailRepoLabel->setText(repo.name);

    // 备份路径
    QString paths = task.sourcePaths.isEmpty() ? tr("无") : task.sourcePaths.join("\n");
    ui->detailPathsLabel->setText(paths);

    // 排除规则
    QString excludes = task.excludePatterns.isEmpty() ? tr("无") : task.excludePatterns.join("\n");
    ui->detailExcludesLabel->setText(excludes);

    // 标签
    QString tags = task.tags.isEmpty() ? tr("无") : task.tags.join(", ");
    ui->detailTagsLabel->setText(tags);

    // 调度设置
    QString schedule;
    switch (task.schedule.type) {
    case Models::Schedule::Manual:
        schedule = tr("手动");
        break;
    case Models::Schedule::Minutely:
        schedule = tr("每分钟 %1").arg(task.schedule.time.toString("HH:mm"));
        break;
    case Models::Schedule::Hourly:
        schedule = tr("每小时 %1 分").arg(task.schedule.time.minute());
        break;
    case Models::Schedule::Daily:
        schedule = tr("每天 %1").arg(task.schedule.time.toString("HH:mm"));
        break;
    case Models::Schedule::Weekly:
        schedule = tr("每周%1 %2").arg(task.schedule.dayOfWeek).arg(task.schedule.time.toString("HH:mm"));
        break;
    case Models::Schedule::Monthly:
        schedule = tr("每月%1日 %2").arg(task.schedule.dayOfMonth).arg(task.schedule.time.toString("HH:mm"));
        break;
    case Models::Schedule::Custom:
        schedule = tr("自定义");
        break;
    default:
        schedule = tr("未设置");
        break;
    }
    ui->detailScheduleLabel->setText(schedule);

    // 上次执行
    QString lastRun = task.lastRun.isValid() ?
        task.lastRun.toString("yyyy-MM-dd HH:mm:ss") : tr("从未");
    ui->detailLastRunLabel->setText(lastRun);

    // 下次执行
    QString nextRun = task.nextRun.isValid() ?
        task.nextRun.toString("yyyy-MM-dd HH:mm:ss") : tr("N/A");
    ui->detailNextRunLabel->setText(nextRun);

    // 加载最近5次执行记录
    ui->historyList->clear();
    QList<Models::BackupResult> history = db->getBackupHistory(taskId, 5);

    if (history.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(tr("暂无执行记录"));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        ui->historyList->addItem(item);
    } else {
        for (const auto& result : history) {
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
            QString displayText = QString("%1 %2 - %3\n    快照: %4 | 新增: %5个文件 | %6")
                .arg(statusIcon)
                .arg(statusText)
                .arg(result.startTime.toString("yyyy-MM-dd HH:mm:ss"))
                .arg(result.snapshotId.isEmpty() ? tr("无") : result.snapshotId.left(8))
                .arg(result.filesNew)
                .arg(sizeText);

            // 如果失败，添加错误信息
            if (result.status == Models::BackupStatus::Failed && !result.errorMessage.isEmpty()) {
                displayText += QString("\n    错误: %1").arg(result.errorMessage);
            }

            QListWidgetItem* item = new QListWidgetItem(displayText);

            // 根据状态设置颜色
            if (result.status == Models::BackupStatus::Success) {
                item->setForeground(QColor(0, 128, 0)); // 绿色
            } else if (result.status == Models::BackupStatus::Failed) {
                item->setForeground(QColor(200, 0, 0)); // 红色
            }

            ui->historyList->addItem(item);
        }
    }
}

void BackupPage::clearDetails()
{
    ui->detailNameLabel->setText("-");
    ui->detailRepoLabel->setText("-");
    ui->detailPathsLabel->setText("-");
    ui->detailExcludesLabel->setText("-");
    ui->detailTagsLabel->setText("-");
    ui->detailScheduleLabel->setText("-");
    ui->detailLastRunLabel->setText("-");
    ui->detailNextRunLabel->setText("-");
    ui->historyList->clear();
}

} // namespace UI
} // namespace ResticGUI
