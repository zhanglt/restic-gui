#include "RepositoryPage.h"
#include "ui_RepositoryPage.h"
#include "../../core/RepositoryManager.h"
#include "../wizards/CreateRepoWizard.h"
#include "../dialogs/ProgressDialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

namespace ResticGUI {
namespace UI {

RepositoryPage::RepositoryPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RepositoryPage)
    , m_createRepoWatcher(nullptr)
    , m_progressDialog(nullptr)
    , m_progressTimer(nullptr)
    , m_timeoutTimer(nullptr)
    , m_progressValue(0)
{
    ui->setupUi(this);

    // 连接信号
    connect(ui->createButton, &QPushButton::clicked, this, &RepositoryPage::onCreateRepository);
    connect(ui->editButton, &QPushButton::clicked, this, &RepositoryPage::onEditRepository);
    connect(ui->deleteButton, &QPushButton::clicked, this, &RepositoryPage::onDeleteRepository);
    connect(ui->refreshButton, &QPushButton::clicked, this, &RepositoryPage::onRefresh);

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
        // waitForFinished() 在析构函数中可以阻塞
        if (m_createRepoWatcher->isRunning()) {
            m_createRepoWatcher->waitForFinished();
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

    // 清空表格
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(repositories.size());

    // 填充表格数据
    for (int i = 0; i < repositories.size(); ++i) {
        const Models::Repository& repo = repositories[i];

        // 名称
        QTableWidgetItem* nameItem = new QTableWidgetItem(repo.name);
        ui->tableWidget->setItem(i, 0, nameItem);

        // 类型
        QTableWidgetItem* typeItem = new QTableWidgetItem(repo.typeDisplayName());
        ui->tableWidget->setItem(i, 1, typeItem);

        // 路径
        QTableWidgetItem* pathItem = new QTableWidgetItem(repo.path);
        ui->tableWidget->setItem(i, 2, pathItem);

        // 最后备份
        QString lastBackup = repo.lastBackup.isValid()
            ? repo.lastBackup.toString("yyyy-MM-dd HH:mm")
            : tr("从未");
        QTableWidgetItem* backupItem = new QTableWidgetItem(lastBackup);
        ui->tableWidget->setItem(i, 3, backupItem);

        // 默认
        QTableWidgetItem* defaultItem = new QTableWidgetItem(repo.isDefault ? tr("是") : tr("否"));
        ui->tableWidget->setItem(i, 4, defaultItem);

        // 存储仓库ID到第一列的data中
        nameItem->setData(Qt::UserRole, repo.id);
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
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
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
    QTableWidgetItem* nameItem = ui->tableWidget->item(currentRow, 0);
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

void RepositoryPage::onRefresh()
{
    loadRepositories();
}

} // namespace UI
} // namespace ResticGUI
