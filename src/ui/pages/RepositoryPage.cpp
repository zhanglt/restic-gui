#include "RepositoryPage.h"
#include "ui_RepositoryPage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/BackupManager.h"
#include "../../core/ResticWrapper.h"
#include "../../data/PasswordManager.h"
#include "../../data/CacheManager.h"
#include "../wizards/CreateRepoWizard.h"
#include "../dialogs/ProgressDialog.h"
#include "../dialogs/PruneOptionsDialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QApplication>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

namespace ResticGUI {
namespace UI {

RepositoryPage::RepositoryPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RepositoryPage)
    , m_createRepoWatcher(nullptr)
    , m_checkRepoWatcher(nullptr)
    , m_repairRepoWatcher(nullptr)
    , m_unlockRepoWatcher(nullptr)
    , m_pruneRepoWatcher(nullptr)
    , m_progressDialog(nullptr)
    , m_progressTimer(nullptr)
    , m_timeoutTimer(nullptr)
    , m_progressValue(0)
    , m_currentOperationRepoId(-1)
{
    ui->setupUi(this);

    // ========== 美化界面样式 ==========

    // 设置顶部按钮样式
    QString primaryButtonStyle = "QPushButton { "
                                  "background-color: #007bff; "
                                  "color: white; "
                                  "border: none; "
                                  "border-radius: 4px; "
                                  "padding: 6px 12px; "
                                  "font-weight: bold; "
                                  "min-height: 24px; "
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

    QString warningButtonStyle = "QPushButton { "
                                  "background-color: #ffc107; "
                                  "color: #212529; "
                                  "border: none; "
                                  "border-radius: 4px; "
                                  "padding: 6px 12px; "
                                  "min-height: 24px; "
                                  "font-weight: bold; "
                                  "} "
                                  "QPushButton:hover { "
                                  "background-color: #e0a800; "
                                  "} "
                                  "QPushButton:pressed { "
                                  "background-color: #d39e00; "
                                  "}";

    // 应用按钮样式
    ui->createButton->setStyleSheet(primaryButtonStyle);
    ui->connectButton->setStyleSheet(primaryButtonStyle);
    ui->editButton->setStyleSheet(secondaryButtonStyle);
    ui->checkButton->setStyleSheet(secondaryButtonStyle);
    ui->repairButton->setStyleSheet(warningButtonStyle);
    ui->pruneButton->setStyleSheet(warningButtonStyle);
    ui->deleteButton->setStyleSheet(dangerButtonStyle);

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
    ui->label_type->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_path->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_created->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_lastBackup->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_snapshots->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_size->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");
    ui->label_original->setStyleSheet("QLabel { color: #212529; font-weight: bold; }");

    // 设置数值标签样式
    ui->detailNameLabel->setStyleSheet(labelStyle);
    ui->detailTypeLabel->setStyleSheet(labelStyle);
    ui->detailPathLabel->setStyleSheet(labelStyle);
    ui->detailCreatedLabel->setStyleSheet(labelStyle);
    ui->detailLastBackupLabel->setStyleSheet(labelStyle);
    ui->detailSnapshotsLabel->setStyleSheet(labelStyle);
    ui->detailSizeLabel->setStyleSheet(labelStyle);
    ui->detailOriginalLabel->setStyleSheet(labelStyle);

    // 设置表格列宽
    ui->tableWidget->setColumnWidth(0, 60);   // 默认(星标)
    ui->tableWidget->setColumnWidth(1, 150);  // 仓库名称
    ui->tableWidget->setColumnWidth(2, 80);   // 类型
    ui->tableWidget->setColumnWidth(3, 250);  // 位置
    ui->tableWidget->setColumnWidth(4, 80);   // 快照数
    ui->tableWidget->setColumnWidth(5, 100);  // 大小

    // 让位置列可以拉伸填充剩余空间
    ui->tableWidget->horizontalHeader()->setStretchLastSection(false);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    // 连接信号
    connect(ui->createButton, &QPushButton::clicked, this, &RepositoryPage::onCreateRepository);
    connect(ui->connectButton, &QPushButton::clicked, this, &RepositoryPage::onCreateRepository); // 暂时复用创建仓库
    connect(ui->editButton, &QPushButton::clicked, this, &RepositoryPage::onEditRepository);
    connect(ui->deleteButton, &QPushButton::clicked, this, &RepositoryPage::onDeleteRepository);
    connect(ui->checkButton, &QPushButton::clicked, this, &RepositoryPage::onCheckRepository);
    connect(ui->repairButton, &QPushButton::clicked, this, &RepositoryPage::onRepairRepository);
    connect(ui->pruneButton, &QPushButton::clicked, this, &RepositoryPage::onPruneRepository);

    // 连接表格选中信号
    connect(ui->tableWidget, &QTableWidget::currentCellChanged, [this](int currentRow, int currentColumn, int previousRow, int previousColumn) {
        Q_UNUSED(currentColumn);
        Q_UNUSED(previousColumn);
        onRepositorySelected(currentRow, previousRow);
    });

    // 连接 BackupManager 的信号，当备份完成时自动刷新仓库列表
    Core::BackupManager* backupMgr = Core::BackupManager::instance();
    connect(backupMgr, &Core::BackupManager::backupFinished, this, [this](int taskId, bool success) {
        Q_UNUSED(taskId);
        if (success) {
            // 备份成功后刷新仓库列表，以更新"最后备份"列
            loadRepositories();
        }
    });

    // 创建进度更新定时器
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &RepositoryPage::onUpdateProgress);

    // 创建超时定时器
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, [this]() {
        onProgressCancelled();
        QMessageBox::warning(this, tr("超时"), tr("创建仓库操作超时，请检查网络连接和仓库路径。"));
    });

    loadRepositories();
}

RepositoryPage::~RepositoryPage()
{
    if (m_progressTimer) {
        m_progressTimer->stop();
    }
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }
    if (m_createRepoWatcher) {
        m_createRepoWatcher->cancel();
        if (m_createRepoWatcher->isRunning()) {
            m_createRepoWatcher->waitForFinished();
        }
    }
    if (m_checkRepoWatcher) {
        m_checkRepoWatcher->cancel();
        if (m_checkRepoWatcher->isRunning()) {
            m_checkRepoWatcher->waitForFinished();
        }
    }
    if (m_repairRepoWatcher) {
        m_repairRepoWatcher->cancel();
        if (m_repairRepoWatcher->isRunning()) {
            m_repairRepoWatcher->waitForFinished();
        }
    }
    if (m_unlockRepoWatcher) {
        m_unlockRepoWatcher->cancel();
        if (m_unlockRepoWatcher->isRunning()) {
            m_unlockRepoWatcher->waitForFinished();
        }
    }
    if (m_pruneRepoWatcher) {
        m_pruneRepoWatcher->cancel();
        if (m_pruneRepoWatcher->isRunning()) {
            m_pruneRepoWatcher->waitForFinished();
        }
    }
    // 注意：定时器和 watcher 的 parent 是 this，会自动删除
    // progressDialog 已使用 deleteLater()，不需要在这里删除
    delete ui;
}

void RepositoryPage::loadRepositories()
{
    // 获取所有仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();
    Data::CacheManager* cacheMgr = Data::CacheManager::instance();
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();

    // 清空表格
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(repositories.size());

    // 填充表格数据
    for (int i = 0; i < repositories.size(); ++i) {
        const Models::Repository& repo = repositories[i];

        // 0. 默认(星标)
        QTableWidgetItem* defaultItem = new QTableWidgetItem(repo.isDefault ? "★" : "");
        defaultItem->setTextAlignment(Qt::AlignCenter);
        if (repo.isDefault) {
            defaultItem->setForeground(QColor(255, 193, 7)); // 金色
        }
        ui->tableWidget->setItem(i, 0, defaultItem);

        // 1. 仓库名称
        QTableWidgetItem* nameItem = new QTableWidgetItem(repo.name);
        nameItem->setData(Qt::UserRole, repo.id);
        ui->tableWidget->setItem(i, 1, nameItem);

        // 2. 类型
        QTableWidgetItem* typeItem = new QTableWidgetItem(repo.typeDisplayName());
        ui->tableWidget->setItem(i, 2, typeItem);

        // 3. 位置
        QTableWidgetItem* pathItem = new QTableWidgetItem(repo.path);
        ui->tableWidget->setItem(i, 3, pathItem);

        // 4. 快照数
        int snapshotCount = 0;
        qint64 totalSize = 0;
        QList<Models::Snapshot> snapshots;

        // 优先从缓存读取
        if (cacheMgr->getCachedSnapshots(repo.id, snapshots)) {
            snapshotCount = snapshots.size();
            for (const auto& snap : snapshots) {
                totalSize += snap.size;
            }
        }

        QTableWidgetItem* snapshotItem = new QTableWidgetItem(QString::number(snapshotCount));
        snapshotItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 4, snapshotItem);

        // 5. 大小
        QString sizeText;
        if (totalSize >= 1024LL * 1024 * 1024) {
            sizeText = QString::number(totalSize / (1024.0 * 1024.0 * 1024.0), 'f', 1) + "GB";
        } else if (totalSize >= 1024 * 1024) {
            sizeText = QString::number(totalSize / (1024.0 * 1024.0), 'f', 1) + "MB";
        } else if (totalSize > 0) {
            sizeText = QString::number(totalSize / 1024.0, 'f', 1) + "KB";
        } else {
            sizeText = "-";
        }
        QTableWidgetItem* sizeItem = new QTableWidgetItem(sizeText);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 5, sizeItem);
    }

    // 清空详情显示
    if (ui->tableWidget->rowCount() == 0) {
        clearDetails();
    }
}

void RepositoryPage::onCreateRepository()
{
    // 创建并显示向导
    CreateRepoWizard wizard(this);

    if (wizard.exec() == QDialog::Accepted) {
        // 获取向导中填写的信息
        m_pendingRepo = wizard.getRepository();
        m_pendingPassword = wizard.getPassword();

        // 创建进度对话框
        if (m_progressDialog) {
            delete m_progressDialog;
        }
        m_progressDialog = new ProgressDialog(this);
        m_progressDialog->setTitle(tr("正在创建仓库"));
        m_progressDialog->setMessage(tr("正在初始化仓库 \"%1\"，请稍候...").arg(m_pendingRepo.name));
        m_progressDialog->setProgress(0);

        // 使用 ApplicationModal 而不是 setModal(true)
        m_progressDialog->setWindowModality(Qt::ApplicationModal);

        // 连接取消信号
        connect(m_progressDialog, &ProgressDialog::cancelled,
                this, &RepositoryPage::onProgressCancelled);

        // 显示进度对话框（非阻塞）
        m_progressDialog->show();

        // 启动进度更新定时器（每200ms更新一次）
        m_progressValue = 0;
        m_progressTimer->start(200);

        // 启动超时定时器（30秒超时）
        m_timeoutTimer->start(30000);

        // 在后台线程创建仓库
        if (m_createRepoWatcher) {
            delete m_createRepoWatcher;
        }
        m_createRepoWatcher = new QFutureWatcher<int>(this);
        connect(m_createRepoWatcher, &QFutureWatcher<int>::finished,
                this, &RepositoryPage::onCreateRepositoryFinished);

        // 延迟一下启动后台任务，让UI先响应
        // 复制值到局部变量，避免跨线程访问成员变量
        Models::Repository repoToCreate = m_pendingRepo;
        QString passwordToUse = m_pendingPassword;

        QTimer::singleShot(50, [this, repoToCreate, passwordToUse]() {
            // 使用 QtConcurrent 在后台线程执行
            // 注意：只捕获值，不捕获this避免线程问题
            QFuture<int> future = QtConcurrent::run([repoToCreate, passwordToUse]() {
                Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
                return repoMgr->createRepository(repoToCreate, passwordToUse, true);
            });

            m_createRepoWatcher->setFuture(future);
        });
    }
}

void RepositoryPage::onCreateRepositoryFinished()
{
    // 停止进度更新定时器
    m_progressTimer->stop();

    // 停止超时定时器
    m_timeoutTimer->stop();

    int repoId = m_createRepoWatcher->result();

    // 关闭进度对话框
    if (m_progressDialog) {
        m_progressDialog->setCompleted(repoId > 0);
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    if (repoId > 0) {
        // 获取仓库信息
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        Models::Repository repo = repoMgr->getRepository(repoId);

        QMessageBox::information(this, tr("成功"),
            tr("仓库 \"%1\" 创建成功！").arg(repo.name));
        // 刷新列表
        loadRepositories();
    } else {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(tr("创建仓库失败"));
        msgBox.setText(tr("创建仓库失败，可能的原因："));
        msgBox.setInformativeText(
            tr("1. Restic 可执行文件未配置或路径不正确\n"
               "2. 仓库路径无效或没有写入权限\n"
               "3. 密码设置有误\n\n"
               "请检查设置并查看日志了解详情。"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void RepositoryPage::onUpdateProgress()
{
    // 模拟进度更新（因为 restic init 不提供实时进度）
    if (m_progressDialog && m_progressTimer->isActive()) {
        // 增加进度，但不超过90%（留10%给真正完成时）
        if (m_progressValue < 90) {
            m_progressValue += 2;

            // 在80%附近放慢速度，避免卡住的感觉
            if (m_progressValue >= 75 && m_progressValue < 85) {
                m_progressValue += 1; // 慢一点
            }
        }

        m_progressDialog->setProgress(m_progressValue);

        // 添加一些动态提示信息
        QString msg;
        if (m_progressValue < 20) {
            msg = tr("正在准备创建仓库...");
        } else if (m_progressValue < 40) {
            msg = tr("正在初始化加密密钥...");
        } else if (m_progressValue < 60) {
            msg = tr("正在配置仓库结构...");
        } else if (m_progressValue < 80) {
            msg = tr("正在验证仓库配置...");
        } else {
            msg = tr("即将完成，请稍候...");
        }

        m_progressDialog->setMessage(msg);
    }
}

void RepositoryPage::onProgressCancelled()
{
    // 停止所有定时器
    m_progressTimer->stop();
    m_timeoutTimer->stop();

    // 取消后台任务
    if (m_createRepoWatcher && m_createRepoWatcher->isRunning()) {
        m_createRepoWatcher->cancel();
        // 注意：Qt 5.14 的 waitForFinished() 不支持超时参数
        // 这里不等待，让后台线程自然结束
    }

    // 关闭进度对话框
    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    QMessageBox::information(this, tr("已取消"), tr("仓库创建操作已取消。"));
}

void RepositoryPage::onEditRepository()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    // 获取仓库ID和当前名称
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    int repoId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();

    // 获取完整的仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取仓库信息"));
        return;
    }

    // 弹出输入框修改名称
    bool ok;
    QString newName = QInputDialog::getText(
        this,
        tr("编辑仓库"),
        tr("仓库名称:"),
        QLineEdit::Normal,
        currentName,
        &ok
    );

    if (ok && !newName.isEmpty() && newName != currentName) {
        // 更新仓库名称
        repo.name = newName;
        if (repoMgr->updateRepository(repo)) {
            QMessageBox::information(this, tr("成功"), tr("仓库名称已更新"));
            // 刷新列表
            loadRepositories();
        } else {
            QMessageBox::critical(this, tr("错误"),
                tr("更新仓库失败，请查看日志了解详情。"));
        }
    }
}

void RepositoryPage::onDeleteRepository()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    // 获取仓库ID和名称
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    int repoId = nameItem->data(Qt::UserRole).toInt();
    QString repoName = nameItem->text();

    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除仓库 \"%1\" 吗？\n\n警告：这将删除数据库中的仓库记录，但不会删除实际的备份数据。")
            .arg(repoName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        // 删除仓库
        Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
        if (repoMgr->deleteRepository(repoId)) {
            QMessageBox::information(this, tr("成功"), tr("仓库已删除"));
            // 刷新列表
            loadRepositories();
        } else {
            QMessageBox::critical(this, tr("错误"),
                tr("删除仓库失败，请查看日志了解详情。"));
        }
    }
}


void RepositoryPage::onCheckRepository()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    // 获取仓库ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    int repoId = nameItem->data(Qt::UserRole).toInt();

    // 获取仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取仓库信息"));
        return;
    }

    // 获取密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    QString password;
    if (!passMgr->getPassword(repo.id, password)) {
        bool ok;
        password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(repo.id, password);
    }

    // 询问是否进行彻底检查
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("检查选项"),
        tr("是否进行彻底检查（读取所有数据包）？\n\n"
           "• 快速检查：仅验证仓库结构和索引（推荐，速度快）\n"
           "• 彻底检查：读取并验证所有数据包（耗时较长，但更完整）"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::No
    );

    if (reply == QMessageBox::Cancel) {
        return;
    }

    bool readData = (reply == QMessageBox::Yes);

    // 保存当前操作的仓库ID和密码
    m_currentOperationRepoId = repo.id;
    m_currentOperationPassword = password;

    // 创建进度对话框
    if (m_progressDialog) {
        delete m_progressDialog;
    }
    m_progressDialog = new ProgressDialog(this);
    m_progressDialog->setTitle(tr("检查仓库"));
    m_progressDialog->setMessage(tr("正在检查仓库 \"%1\"，请稍候...").arg(repo.name));
    m_progressDialog->setProgress(0);
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    connect(m_progressDialog, &ProgressDialog::cancelled,
            this, &RepositoryPage::onProgressCancelled);
    m_progressDialog->show();

    // 启动进度更新定时器
    m_progressValue = 0;
    m_progressTimer->start(200);

    // 启动超时定时器（5分钟超时）
    m_timeoutTimer->start(300000);

    // 在后台线程执行检查
    if (m_checkRepoWatcher) {
        delete m_checkRepoWatcher;
    }
    m_checkRepoWatcher = new QFutureWatcher<bool>(this);
    connect(m_checkRepoWatcher, &QFutureWatcher<bool>::finished,
            this, &RepositoryPage::onCheckRepositoryFinished);

    // 复制值避免跨线程问题
    Models::Repository repoToCheck = repo;
    QString passwordToUse = password;

    QTimer::singleShot(50, [this, repoToCheck, passwordToUse, readData]() {
        QFuture<bool> future = QtConcurrent::run([repoToCheck, passwordToUse, readData]() {
            Core::ResticWrapper wrapper;
            return wrapper.checkRepository(repoToCheck, passwordToUse, readData);
        });

        m_checkRepoWatcher->setFuture(future);
    });
}

void RepositoryPage::onRepairRepository()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    // 获取仓库ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    int repoId = nameItem->data(Qt::UserRole).toInt();

    // 获取仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取仓库信息"));
        return;
    }

    // 获取密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    QString password;
    if (!passMgr->getPassword(repo.id, password)) {
        bool ok;
        password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(repo.id, password);
    }

    // 确认修复
    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        tr("确认修复"),
        tr("确定要修复仓库 \"%1\" 吗？\n\n"
           "修复操作将：\n"
           "1. 修复仓库索引\n"
           "2. 修复快照元数据\n\n"
           "此操作可能需要较长时间，请耐心等待。").arg(repo.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 保存当前操作的仓库ID和密码
    m_currentOperationRepoId = repo.id;
    m_currentOperationPassword = password;

    // 创建进度对话框
    if (m_progressDialog) {
        delete m_progressDialog;
    }
    m_progressDialog = new ProgressDialog(this);
    m_progressDialog->setTitle(tr("修复仓库"));
    m_progressDialog->setMessage(tr("正在修复仓库 \"%1\"，请稍候...").arg(repo.name));
    m_progressDialog->setProgress(0);
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    connect(m_progressDialog, &ProgressDialog::cancelled,
            this, &RepositoryPage::onProgressCancelled);
    m_progressDialog->show();

    // 启动进度更新定时器
    m_progressValue = 0;
    m_progressTimer->start(200);

    // 启动超时定时器（10分钟超时，修复可能需要较长时间）
    m_timeoutTimer->start(600000);

    // 在后台线程执行修复
    if (m_repairRepoWatcher) {
        delete m_repairRepoWatcher;
    }
    m_repairRepoWatcher = new QFutureWatcher<bool>(this);
    connect(m_repairRepoWatcher, &QFutureWatcher<bool>::finished,
            this, &RepositoryPage::onRepairRepositoryFinished);

    // 复制值避免跨线程问题
    Models::Repository repoToRepair = repo;
    QString passwordToUse = password;

    QTimer::singleShot(50, [this, repoToRepair, passwordToUse]() {
        QFuture<bool> future = QtConcurrent::run([repoToRepair, passwordToUse]() {
            Core::ResticWrapper wrapper;
            return wrapper.repairRepository(repoToRepair, passwordToUse);
        });

        m_repairRepoWatcher->setFuture(future);
    });
}

void RepositoryPage::onUnlockRepository()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    // 获取仓库ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    int repoId = nameItem->data(Qt::UserRole).toInt();

    // 获取仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取仓库信息"));
        return;
    }

    // 获取密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    QString password;
    if (!passMgr->getPassword(repo.id, password)) {
        bool ok;
        password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(repo.id, password);
    }

    // 确认解锁
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认解锁"),
        tr("确定要解锁仓库 \"%1\" 吗？\n\n"
           "这将删除该仓库的锁文件。\n"
           "通常在异常中断后需要解锁仓库才能继续操作。").arg(repo.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 保存当前操作的仓库ID和密码
    m_currentOperationRepoId = repo.id;
    m_currentOperationPassword = password;

    // 创建进度对话框
    if (m_progressDialog) {
        delete m_progressDialog;
    }
    m_progressDialog = new ProgressDialog(this);
    m_progressDialog->setTitle(tr("解锁仓库"));
    m_progressDialog->setMessage(tr("正在解锁仓库 \"%1\"，请稍候...").arg(repo.name));
    m_progressDialog->setProgress(0);
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    connect(m_progressDialog, &ProgressDialog::cancelled,
            this, &RepositoryPage::onProgressCancelled);
    m_progressDialog->show();

    // 启动进度更新定时器
    m_progressValue = 0;
    m_progressTimer->start(200);

    // 启动超时定时器（1分钟超时）
    m_timeoutTimer->start(60000);

    // 在后台线程执行解锁
    if (m_unlockRepoWatcher) {
        delete m_unlockRepoWatcher;
    }
    m_unlockRepoWatcher = new QFutureWatcher<bool>(this);
    connect(m_unlockRepoWatcher, &QFutureWatcher<bool>::finished,
            this, &RepositoryPage::onUnlockRepositoryFinished);

    // 复制值避免跨线程问题
    Models::Repository repoToUnlock = repo;
    QString passwordToUse = password;

    QTimer::singleShot(50, [this, repoToUnlock, passwordToUse]() {
        QFuture<bool> future = QtConcurrent::run([repoToUnlock, passwordToUse]() {
            Core::ResticWrapper wrapper;
            return wrapper.unlockRepository(repoToUnlock, passwordToUse);
        });

        m_unlockRepoWatcher->setFuture(future);
    });
}

void RepositoryPage::onPruneRepository()
{
    // 获取选中的行
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    // 获取仓库ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    int repoId = nameItem->data(Qt::UserRole).toInt();

    // 获取仓库信息
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取仓库信息"));
        return;
    }

    // 获取密码
    Data::PasswordManager* passMgr = Data::PasswordManager::instance();
    QString password;
    if (!passMgr->getPassword(repo.id, password)) {
        bool ok;
        password = QInputDialog::getText(this, tr("输入密码"),
            tr("请输入仓库 \"%1\" 的密码：").arg(repo.name),
            QLineEdit::Password, QString(), &ok);

        if (!ok || password.isEmpty()) {
            return;
        }

        // 保存密码到密码管理器
        passMgr->setPassword(repo.id, password);
    }

    // 显示保留策略配置对话框
    PruneOptionsDialog optionsDialog(this);
    if (optionsDialog.exec() != QDialog::Accepted) {
        return;
    }

    // 获取保留策略
    int keepLast = optionsDialog.getKeepLast();
    int keepDaily = optionsDialog.getKeepDaily();
    int keepWeekly = optionsDialog.getKeepWeekly();
    int keepMonthly = optionsDialog.getKeepMonthly();
    int keepYearly = optionsDialog.getKeepYearly();

    // 检查至少设置了一个保留策略
    if (keepLast == 0 && keepDaily == 0 && keepWeekly == 0 &&
        keepMonthly == 0 && keepYearly == 0) {
        QMessageBox::warning(this, tr("警告"),
            tr("请至少设置一个保留策略！"));
        return;
    }

    // 确认维护
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认维护"),
        tr("确定要维护仓库 \"%1\" 吗？\n\n"
           "这将执行 prune 操作，删除不符合保留策略的快照数据。\n"
           "⚠️ 此操作不可恢复！").arg(repo.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 保存当前操作的仓库ID和密码
    m_currentOperationRepoId = repo.id;
    m_currentOperationPassword = password;

    // 创建进度对话框
    if (m_progressDialog) {
        delete m_progressDialog;
    }
    m_progressDialog = new ProgressDialog(this);
    m_progressDialog->setTitle(tr("维护仓库"));
    m_progressDialog->setMessage(tr("正在维护仓库 \"%1\"，请稍候...").arg(repo.name));
    m_progressDialog->setProgress(0);
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    connect(m_progressDialog, &ProgressDialog::cancelled,
            this, &RepositoryPage::onProgressCancelled);
    m_progressDialog->show();

    // 启动进度更新定时器
    m_progressValue = 0;
    m_progressTimer->start(200);

    // 启动超时定时器（10分钟超时）
    m_timeoutTimer->start(600000);

    // 在后台线程执行 prune
    if (m_pruneRepoWatcher) {
        delete m_pruneRepoWatcher;
    }
    m_pruneRepoWatcher = new QFutureWatcher<bool>(this);
    connect(m_pruneRepoWatcher, &QFutureWatcher<bool>::finished,
            this, &RepositoryPage::onPruneRepositoryFinished);

    // 复制值避免跨线程问题
    Models::Repository repoToPrune = repo;
    QString passwordToUse = password;

    QTimer::singleShot(50, [this, repoToPrune, passwordToUse, keepLast, keepDaily,
                             keepWeekly, keepMonthly, keepYearly]() {
        QFuture<bool> future = QtConcurrent::run([repoToPrune, passwordToUse, keepLast,
                                                   keepDaily, keepWeekly, keepMonthly, keepYearly]() {
            Core::ResticWrapper wrapper;
            return wrapper.prune(repoToPrune, passwordToUse, keepLast, keepDaily,
                               keepWeekly, keepMonthly, keepYearly);
        });

        m_pruneRepoWatcher->setFuture(future);
    });
}

void RepositoryPage::onCheckRepositoryFinished()
{
    // 停止进度更新定时器
    m_progressTimer->stop();

    // 停止超时定时器
    m_timeoutTimer->stop();

    bool success = m_checkRepoWatcher->result();

    // 关闭进度对话框
    if (m_progressDialog) {
        m_progressDialog->setCompleted(success);
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    // 获取仓库名称
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(m_currentOperationRepoId);

    if (success) {
        QMessageBox::information(this, tr("检查完成"),
            tr("仓库 \"%1\" 检查完成！\n\n仓库状态正常，未发现错误。").arg(repo.name));
    } else {
        QMessageBox::critical(this, tr("检查失败"),
            tr("仓库 \"%1\" 检查失败！\n\n请查看日志了解详情。").arg(repo.name));
    }
}

void RepositoryPage::onRepairRepositoryFinished()
{
    // 停止进度更新定时器
    m_progressTimer->stop();

    // 停止超时定时器
    m_timeoutTimer->stop();

    bool success = m_repairRepoWatcher->result();

    // 关闭进度对话框
    if (m_progressDialog) {
        m_progressDialog->setCompleted(success);
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    // 获取仓库名称
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(m_currentOperationRepoId);

    if (success) {
        QMessageBox::information(this, tr("修复完成"),
            tr("仓库 \"%1\" 修复完成！\n\n已成功修复仓库索引和快照。").arg(repo.name));
    } else {
        QMessageBox::critical(this, tr("修复失败"),
            tr("仓库 \"%1\" 修复失败！\n\n请查看日志了解详情。").arg(repo.name));
    }
}

void RepositoryPage::onUnlockRepositoryFinished()
{
    // 停止进度更新定时器
    m_progressTimer->stop();

    // 停止超时定时器
    m_timeoutTimer->stop();

    bool success = m_unlockRepoWatcher->result();

    // 关闭进度对话框
    if (m_progressDialog) {
        m_progressDialog->setCompleted(success);
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    // 获取仓库名称
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(m_currentOperationRepoId);

    if (success) {
        QMessageBox::information(this, tr("解锁成功"),
            tr("仓库 \"%1\" 已成功解锁！\n\n现在可以正常进行备份和恢复操作。").arg(repo.name));
    } else {
        QMessageBox::critical(this, tr("解锁失败"),
            tr("仓库 \"%1\" 解锁失败！\n\n请查看日志了解详情。").arg(repo.name));
    }
}

void RepositoryPage::onPruneRepositoryFinished()
{
    // 停止进度更新定时器
    m_progressTimer->stop();

    // 停止超时定时器
    m_timeoutTimer->stop();

    bool success = m_pruneRepoWatcher->result();

    // 关闭进度对话框
    if (m_progressDialog) {
        m_progressDialog->setCompleted(success);
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    // 获取仓库名称
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(m_currentOperationRepoId);

    if (success) {
        QMessageBox::information(this, tr("维护完成"),
            tr("仓库 \"%1\" 维护完成！\n\n"
               "已删除不符合保留策略的快照数据，释放了存储空间。").arg(repo.name));
    } else {
        QMessageBox::critical(this, tr("维护失败"),
            tr("仓库 \"%1\" 维护失败！\n\n请查看日志了解详情。").arg(repo.name));
    }
}

void RepositoryPage::onRepositorySelected(int currentRow, int previousRow)
{
    Q_UNUSED(previousRow);

    if (currentRow < 0) {
        clearDetails();
        return;
    }

    // 获取仓库ID
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 1);
    if (!nameItem) {
        clearDetails();
        return;
    }

    int repoId = nameItem->data(Qt::UserRole).toInt();

    // 获取仓库详情
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getRepository(repoId);

    if (repo.id <= 0) {
        clearDetails();
        return;
    }

    // 填充详情
    ui->detailNameLabel->setText(repo.name);
    ui->detailTypeLabel->setText(repo.typeDisplayName());
    ui->detailPathLabel->setText(repo.path);
    ui->detailCreatedLabel->setText(repo.createdAt.toString("yyyy-MM-dd HH:mm:ss"));

    QString lastBackup = repo.lastBackup.isValid()
        ? repo.lastBackup.toString("yyyy-MM-dd HH:mm:ss")
        : tr("从未");
    ui->detailLastBackupLabel->setText(lastBackup);

    // 获取快照信息
    Data::CacheManager* cacheMgr = Data::CacheManager::instance();
    QList<Models::Snapshot> snapshots;
    int snapshotCount = 0;
    qint64 totalSize = 0;

    if (cacheMgr->getCachedSnapshots(repo.id, snapshots)) {
        snapshotCount = snapshots.size();
        for (const auto& snap : snapshots) {
            totalSize += snap.size;
        }
    }

    ui->detailSnapshotsLabel->setText(QString::number(snapshotCount));

    // 格式化大小显示
    QString sizeText;
    if (totalSize >= 1024LL * 1024 * 1024) {
        sizeText = QString::number(totalSize / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    } else if (totalSize >= 1024 * 1024) {
        sizeText = QString::number(totalSize / (1024.0 * 1024.0), 'f', 2) + " MB";
    } else if (totalSize > 0) {
        sizeText = QString::number(totalSize / 1024.0, 'f', 2) + " KB";
    } else {
        sizeText = tr("未知");
    }
    ui->detailSizeLabel->setText(sizeText);

    // 原始数据大小(估算)
    // 假设去重率为75%
    qint64 originalSize = totalSize * 4; // 简单估算
    QString originalText;
    if (originalSize >= 1024LL * 1024 * 1024) {
        originalText = QString::number(originalSize / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
    } else if (originalSize >= 1024 * 1024) {
        originalText = QString::number(originalSize / (1024.0 * 1024.0), 'f', 1) + " MB";
    } else {
        originalText = QString::number(originalSize / 1024.0, 'f', 1) + " KB";
    }

    if (totalSize > 0) {
        int dedupeRate = 75; // 假设去重率
        originalText += QString(" (去重率: %1%)").arg(dedupeRate);
    }
    ui->detailOriginalLabel->setText(originalText);
}

void RepositoryPage::clearDetails()
{
    ui->detailNameLabel->setText("-");
    ui->detailTypeLabel->setText("-");
    ui->detailPathLabel->setText("-");
    ui->detailCreatedLabel->setText("-");
    ui->detailLastBackupLabel->setText("-");
    ui->detailSnapshotsLabel->setText("-");
    ui->detailSizeLabel->setText("-");
    ui->detailOriginalLabel->setText("-");
}

} // namespace UI
} // namespace ResticGUI
