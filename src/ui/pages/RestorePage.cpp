#include "RestorePage.h"
#include "ui_RestorePage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../core/RestoreManager.h"
#include "../../data/PasswordManager.h"
#include "../../utils/Logger.h"
#include "../../models/RestoreOptions.h"
#include "../dialogs/ProgressDialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QtConcurrent>

namespace ResticGUI {
namespace UI {

RestorePage::RestorePage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RestorePage)
    , m_currentRepositoryId(-1)
    , m_firstShow(true)
    , m_isLoading(false)
    , m_snapshotWatcher(nullptr)
{
    ui->setupUi(this);

    // 初始化异步加载器
    m_snapshotWatcher = new QFutureWatcher<QList<Models::Snapshot>>(this);
    connect(m_snapshotWatcher, &QFutureWatcher<QList<Models::Snapshot>>::finished,
            this, &RestorePage::onSnapshotsLoaded);

    // 先加载仓库列表（此时不连接信号，避免触发密码输入）
    loadRepositories();

    // 加载完成后再连接信号
    connect(ui->repositoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RestorePage::onRepositoryChanged);
    connect(ui->browseButton, &QPushButton::clicked, this, &RestorePage::onBrowse);
    connect(ui->restoreButton, &QPushButton::clicked, this, &RestorePage::onRestore);

    // 监听快照更新信号
    Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
    connect(snapshotMgr, &Core::SnapshotManager::snapshotsUpdated,
            this, &RestorePage::onSnapshotsUpdated);
}

RestorePage::~RestorePage()
{
    delete ui;
}

void RestorePage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    // 第一次显示时自动加载快照
    if (m_firstShow && m_currentRepositoryId > 0) {
        m_firstShow = false;
        loadSnapshots();
    }
}

void RestorePage::loadRepositories()
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

void RestorePage::onRepositoryChanged(int index)
{
    m_currentRepositoryId = ui->repositoryComboBox->itemData(index).toInt();

    // 切换仓库时，重置加载状态以允许加载新仓库的快照
    m_isLoading = false;

    loadSnapshots();
}

void RestorePage::loadSnapshots()
{
    if (m_currentRepositoryId <= 0) {
        ui->snapshotComboBox->clear();
        ui->snapshotComboBox->addItem(tr("(无快照)"), QString());
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
            ui->snapshotComboBox->clear();
            ui->snapshotComboBox->addItem(tr("(需要密码)"), QString());
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

void RestorePage::onBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择恢复目标目录"),
        ui->targetPathEdit->text().isEmpty() ? QDir::homePath() : ui->targetPathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        ui->targetPathEdit->setText(dir);
    }
}

void RestorePage::onSnapshotsUpdated(int repoId)
{
    // 如果更新的是当前选中的仓库，刷新快照列表
    if (repoId == m_currentRepositoryId) {
        loadSnapshots();
    }
}

void RestorePage::onRestore()
{
    // 1. 验证输入
    if (m_currentRepositoryId <= 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个仓库"));
        return;
    }

    QString snapshotId = ui->snapshotComboBox->currentData().toString();
    if (snapshotId.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个快照"));
        return;
    }

    QString targetPath = ui->targetPathEdit->text().trimmed();
    if (targetPath.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入恢复目标路径"));
        return;
    }

    // 检查目标路径是否存在
    QDir targetDir(targetPath);
    if (!targetDir.exists()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("确认"),
            tr("目标路径不存在，是否创建？\n%1").arg(targetPath),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            if (!targetDir.mkpath(".")) {
                QMessageBox::critical(this, tr("错误"), tr("无法创建目标路径"));
                return;
            }
        } else {
            return;
        }
    }

    // 2. 准备恢复选项
    Models::RestoreOptions options;
    options.targetPath = targetPath;
    options.overwritePolicy = Models::RestoreOptions::Always;
    options.restorePermissions = true;
    options.restoreTimestamps = true;
    options.restoreOwnership = false;
    options.sparse = false;
    options.verify = false;

    // 3. 确认操作
    QString snapshotText = ui->snapshotComboBox->currentText();
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this,
        tr("确认恢复"),
        tr("确定要恢复以下快照吗？\n\n快照：%1\n目标路径：%2\n\n注意：目标路径中的同名文件将被覆盖！")
            .arg(snapshotText)
            .arg(targetPath),
        QMessageBox::Yes | QMessageBox::No
    );

    if (confirm != QMessageBox::Yes) {
        return;
    }

    // 4. 执行恢复
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始恢复快照 %1 到 %2").arg(snapshotId).arg(targetPath));

    Core::RestoreManager* restoreMgr = Core::RestoreManager::instance();

    // 创建进度对话框
    ProgressDialog* progressDialog = new ProgressDialog(this);
    progressDialog->setTitle(tr("恢复进度"));
    progressDialog->setMessage(tr("正在恢复数据..."));
    progressDialog->setProgress(0);

    // 连接信号
    connect(restoreMgr, &Core::RestoreManager::restoreProgress,
            progressDialog, [progressDialog](int percent, const QString& message) {
        progressDialog->setProgress(percent);
        progressDialog->setMessage(message);
        progressDialog->appendLog(message);
    });

    connect(restoreMgr, &Core::RestoreManager::restoreFinished,
            this, [this, progressDialog, snapshotText, targetPath](bool success) {
        progressDialog->close();
        progressDialog->deleteLater();

        if (success) {
            QMessageBox::information(this, tr("成功"),
                tr("数据恢复成功！\n\n快照：%1\n目标路径：%2")
                    .arg(snapshotText)
                    .arg(targetPath));

            Utils::Logger::instance()->log(Utils::Logger::Info, "数据恢复成功");
        } else {
            QMessageBox::critical(this, tr("失败"),
                tr("数据恢复失败，请查看日志了解详情"));

            Utils::Logger::instance()->log(Utils::Logger::Error, "数据恢复失败");
        }
    });

    connect(restoreMgr, &Core::RestoreManager::restoreError,
            this, [progressDialog](const QString& error) {
        progressDialog->close();
        progressDialog->deleteLater();

        QMessageBox::critical(nullptr, QObject::tr("错误"),
            QObject::tr("恢复失败：%1").arg(error));
    });

    // 开始恢复
    bool started = restoreMgr->restore(m_currentRepositoryId, snapshotId, options);

    if (started) {
        progressDialog->show();
    } else {
        progressDialog->deleteLater();
        QMessageBox::critical(this, tr("错误"), tr("无法启动恢复操作"));
    }
}

void RestorePage::onSnapshotsLoaded()
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

void RestorePage::displaySnapshots(const QList<Models::Snapshot>& snapshots)
{
    ui->snapshotComboBox->clear();

    if (snapshots.isEmpty()) {
        ui->snapshotComboBox->addItem(tr("(无快照)"), QString());
        return;
    }

    // 填充快照下拉框（按时间倒序，最新的在前面）
    for (int i = snapshots.size() - 1; i >= 0; --i) {
        const Models::Snapshot& snapshot = snapshots[i];
        QString displayText = QString("%1 - %2")
            .arg(snapshot.time.toString("yyyy-MM-dd HH:mm:ss"))
            .arg(snapshot.paths.isEmpty() ? tr("(无路径)") : snapshot.paths.first());

        if (displayText.length() > 80) {
            displayText = displayText.left(77) + "...";
        }

        ui->snapshotComboBox->addItem(displayText, snapshot.id);
    }
}

void RestorePage::showLoadingIndicator(bool show)
{
    if (show) {
        // 显示"加载中..."提示
        ui->snapshotComboBox->clear();
        ui->snapshotComboBox->addItem(tr("正在加载快照列表..."), QString());
        ui->snapshotComboBox->setEnabled(false);
        ui->restoreButton->setEnabled(false);
    } else {
        // 启用控件
        ui->snapshotComboBox->setEnabled(true);
        ui->restoreButton->setEnabled(true);
    }
}

} // namespace UI
} // namespace ResticGUI
