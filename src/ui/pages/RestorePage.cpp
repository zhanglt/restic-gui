#include "RestorePage.h"
#include "ui_RestorePage.h"
#include "../../core/RepositoryManager.h"
#include "../../core/SnapshotManager.h"
#include "../../core/RestoreManager.h"
#include "../../data/PasswordManager.h"
#include "../../utils/Logger.h"
#include "../../models/RestoreOptions.h"
#include "../dialogs/ProgressDialog.h"
#include "../wizards/RestoreWizard.h"
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

    // 应用样式
    QString primaryButtonStyle =
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: 1px solid #007bff;"
        "    padding: 6px 12px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0056b3;"
        "    border-color: #004085;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #6c757d;"
        "    border-color: #6c757d;"
        "}";

    QString secondaryButtonStyle =
        "QPushButton {"
        "    background-color: #6c757d;"
        "    color: white;"
        "    border: 1px solid #6c757d;"
        "    padding: 6px 12px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5a6268;"
        "    border-color: #545b62;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #c6c8ca;"
        "    border-color: #c6c8ca;"
        "}";

    QString successButtonStyle =
        "QPushButton {"
        "    background-color: #28a745;"
        "    color: white;"
        "    border: 1px solid #28a745;"
        "    padding: 6px 12px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #218838;"
        "    border-color: #1e7e34;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #6c757d;"
        "    border-color: #6c757d;"
        "}";

    QString comboBoxStyle =
        "QComboBox {"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "    background-color: white;"
        "}"
        "QComboBox:hover {"
        "    border-color: #80bdff;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
        "QComboBox::down-arrow {"
        "    image: url(:/icons/down-arrow.png);"
        "}";

    QString lineEditStyle =
        "QLineEdit {"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #80bdff;"
        "    outline: none;"
        "}"
        "QLineEdit:disabled {"
        "    background-color: #e9ecef;"
        "    color: #6c757d;"
        "}";

    QString tableStyle =
        "QTableWidget {"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    gridline-color: #dee2e6;"
        "}"
        "QTableWidget::item {"
        "    padding: 5px;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #007bff;"
        "    color: white;"
        "}";

    QString groupBoxStyle =
        "QGroupBox {"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 4px;"
        "    margin-top: 10px;"
        "    font-weight: bold;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px;"
        "}";

    QString checkBoxStyle =
        "QCheckBox {"
        "    spacing: 5px;"
        "}"
        "QCheckBox::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 3px;"
        "    background-color: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "    background-color: #007bff;"
        "    border-color: #007bff;"
        "}"
        "QCheckBox::indicator:hover {"
        "    border-color: #80bdff;"
        "}";

    // 应用样式到控件
    ui->repositoryComboBox->setStyleSheet(comboBoxStyle);
    ui->filterEdit->setStyleSheet(lineEditStyle);
    ui->searchButton->setStyleSheet(secondaryButtonStyle);
    ui->refreshButton->setStyleSheet(secondaryButtonStyle);
    ui->snapshotTable->setStyleSheet(tableStyle);
    ui->targetPathEdit->setStyleSheet(lineEditStyle);
    ui->includeEdit->setStyleSheet(lineEditStyle);
    ui->browseButton->setStyleSheet(secondaryButtonStyle);
    ui->quickRestoreButton->setStyleSheet(primaryButtonStyle);
    ui->restoreButton->setStyleSheet(successButtonStyle);
    ui->restoreOptionsGroup->setStyleSheet(groupBoxStyle);
    ui->includeCheckBox->setStyleSheet(checkBoxStyle);

    // 配置快照表格
    ui->snapshotTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->snapshotTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->snapshotTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->snapshotTable->setAlternatingRowColors(true);
    ui->snapshotTable->horizontalHeader()->setStretchLastSection(false);

    // 设置列宽和调整模式
    ui->snapshotTable->setColumnWidth(0, 100);  // 快照ID（显示前8位）
    ui->snapshotTable->setColumnWidth(1, 160);  // 创建时间
    ui->snapshotTable->setColumnWidth(2, 100);  // 主机名
    ui->snapshotTable->setColumnWidth(4, 100);  // 大小
    ui->snapshotTable->setColumnWidth(5, 120);  // 标签

    // 路径列使用拉伸模式，占据剩余空间
    ui->snapshotTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    // 初始化异步加载器
    m_snapshotWatcher = new QFutureWatcher<QList<Models::Snapshot>>(this);
    connect(m_snapshotWatcher, &QFutureWatcher<QList<Models::Snapshot>>::finished,
            this, &RestorePage::onSnapshotsLoaded);

    // 先加载仓库列表（此时不连接信号，避免触发密码输入）
    loadRepositories();

    // 加载完成后再连接信号
    connect(ui->repositoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RestorePage::onRepositoryChanged);
    connect(ui->filterEdit, &QLineEdit::textChanged, this, &RestorePage::onFilterTextChanged);
    connect(ui->searchButton, &QPushButton::clicked, this, &RestorePage::onSearch);
    connect(ui->refreshButton, &QPushButton::clicked, this, &RestorePage::onRefresh);
    connect(ui->browseButton, &QPushButton::clicked, this, &RestorePage::onBrowse);
    connect(ui->quickRestoreButton, &QPushButton::clicked, this, &RestorePage::onQuickRestore);
    connect(ui->restoreButton, &QPushButton::clicked, this, &RestorePage::onRestore);
    connect(ui->includeCheckBox, &QCheckBox::toggled, this, &RestorePage::onIncludeCheckBoxToggled);
    connect(ui->targetPathEdit, &QLineEdit::textChanged, this, &RestorePage::onTargetPathChanged);
    connect(ui->snapshotTable, &QTableWidget::currentCellChanged,
            this, [this](int currentRow, int, int previousRow, int) {
        onSnapshotSelected(currentRow, previousRow);
    });

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
        ui->snapshotTable->setRowCount(0);
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
            ui->snapshotTable->setRowCount(0);
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

void RestorePage::onRefresh()
{
    loadSnapshots();
}

void RestorePage::onIncludeCheckBoxToggled(bool checked)
{
    ui->includeEdit->setEnabled(checked);
    if (!checked) {
        ui->includeEdit->clear();
    }
}

void RestorePage::onSnapshotSelected(int currentRow, int previousRow)
{
    Q_UNUSED(previousRow);

    // 恢复向导按钮始终保持启用状态，因为向导内部会引导用户选择
    // 只更新快速恢复按钮的状态
    updateQuickRestoreButtonState();
}

void RestorePage::onRestore()
{
    // 显示恢复向导
    RestoreWizard wizard(this);

    // 如果主界面已经选择了恢复目录，预设到向导中
    QString targetPath = ui->targetPathEdit->text().trimmed();
    if (!targetPath.isEmpty()) {
        wizard.setPresetTargetPath(targetPath);
    }

    // 如果用户取消向导，直接返回
    if (wizard.exec() != QDialog::Accepted) {
        return;
    }

    // 从向导获取配置
    int repositoryId = wizard.getRepositoryId();
    QString snapshotId = wizard.getSnapshotId();
    QStringList selectedPaths = wizard.getSelectedPaths();
    Models::RestoreOptions options = wizard.getRestoreOptions();

    // 执行恢复
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始恢复快照 %1 到 %2").arg(snapshotId).arg(options.targetPath));

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
            this, [this, progressDialog, snapshotId, options](bool success) {
        progressDialog->close();
        progressDialog->deleteLater();

        if (success) {
            QMessageBox::information(this, tr("成功"),
                tr("数据恢复成功！\n\n快照：%1\n目标路径：%2")
                    .arg(snapshotId.left(8))
                    .arg(options.targetPath));

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
    bool started = restoreMgr->restore(repositoryId, snapshotId, options);

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

    // 保存所有快照用于筛选
    m_allSnapshots = snapshots;

    // 应用当前筛选条件
    QString filterText = ui->filterEdit->text().trimmed();
    if (!filterText.isEmpty()) {
        filterSnapshots(filterText);
    } else {
        // 显示快照列表
        displaySnapshots(snapshots);
    }
}

void RestorePage::displaySnapshots(const QList<Models::Snapshot>& snapshots)
{
    ui->snapshotTable->setRowCount(0);

    if (snapshots.isEmpty()) {
        return;
    }

    // 填充快照表格（按时间倒序，最新的在前面）
    ui->snapshotTable->setRowCount(snapshots.size());

    for (int i = 0; i < snapshots.size(); ++i) {
        const Models::Snapshot& snapshot = snapshots[snapshots.size() - 1 - i];

        // 快照ID（显示前8位）
        QString displayId = snapshot.id.left(8);
        QTableWidgetItem* idItem = new QTableWidgetItem(displayId);
        idItem->setData(Qt::UserRole, snapshot.fullId.isEmpty() ? snapshot.id : snapshot.fullId);  // 存储完整ID
        ui->snapshotTable->setItem(i, 0, idItem);

        // 创建时间
        QTableWidgetItem* timeItem = new QTableWidgetItem(snapshot.time.toString("yyyy-MM-dd HH:mm:ss"));
        ui->snapshotTable->setItem(i, 1, timeItem);

        // 主机名
        QTableWidgetItem* hostnameItem = new QTableWidgetItem(snapshot.hostname);
        ui->snapshotTable->setItem(i, 2, hostnameItem);

        // 路径（显示第一个路径，如果有多个显示省略号）
        QString pathsText = snapshot.paths.isEmpty() ? tr("(无路径)") : snapshot.paths.first();
        if (snapshot.paths.size() > 1) {
            pathsText += QString(" (+%1)").arg(snapshot.paths.size() - 1);
        }
        QTableWidgetItem* pathsItem = new QTableWidgetItem(pathsText);
        pathsItem->setToolTip(snapshot.paths.join("\n"));
        ui->snapshotTable->setItem(i, 3, pathsItem);

        // 大小（格式化显示）
        QString sizeText;
        qint64 size = snapshot.size;

        if (size >= 1024LL * 1024 * 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0 * 1024.0 * 1024.0), 'f', 2) + " TB";
        } else if (size >= 1024LL * 1024 * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
        } else if (size >= 1024LL * 1024) {
            sizeText = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
        } else if (size >= 1024) {
            sizeText = QString::number(size / 1024.0, 'f', 2) + " KB";
        } else if (size > 0) {
            sizeText = QString::number(size) + " B";
        } else {
            sizeText = "-";
        }

        QTableWidgetItem* sizeItem = new QTableWidgetItem(sizeText);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->snapshotTable->setItem(i, 4, sizeItem);

        // 标签
        QTableWidgetItem* tagsItem = new QTableWidgetItem(snapshot.tags.join(", "));
        ui->snapshotTable->setItem(i, 5, tagsItem);
    }
}

void RestorePage::showLoadingIndicator(bool show)
{
    if (show) {
        // 显示加载状态
        ui->snapshotTable->setRowCount(0);
        ui->snapshotTable->setEnabled(false);
        ui->restoreButton->setEnabled(false);
        ui->quickRestoreButton->setEnabled(false);
        ui->refreshButton->setEnabled(false);
    } else {
        // 启用控件
        ui->snapshotTable->setEnabled(true);
        ui->refreshButton->setEnabled(true);
        ui->restoreButton->setEnabled(true);  // 恢复向导始终启用
        // quickRestoreButton会在选中快照且填写路径时启用
    }
}

void RestorePage::onTargetPathChanged()
{
    updateQuickRestoreButtonState();
}

void RestorePage::updateQuickRestoreButtonState()
{
    // 只有在选中快照且填写了恢复目录时才启用快速恢复按钮
    bool hasSnapshot = ui->snapshotTable->currentRow() >= 0;
    bool hasTargetPath = !ui->targetPathEdit->text().isEmpty();

    ui->quickRestoreButton->setEnabled(hasSnapshot && hasTargetPath);
}

void RestorePage::onSearch()
{
    QString filterText = ui->filterEdit->text().trimmed();
    filterSnapshots(filterText);
}

void RestorePage::onFilterTextChanged(const QString& text)
{
    // 实时筛选（可选，如果不需要实时筛选，可以只在点击搜索按钮时筛选）
    Q_UNUSED(text);
    // 不做实时筛选，用户需要点击搜索按钮
}

void RestorePage::filterSnapshots(const QString& filterText)
{
    if (filterText.isEmpty()) {
        // 如果筛选文本为空，显示所有快照
        displaySnapshots(m_allSnapshots);
        return;
    }

    // 筛选快照
    QList<Models::Snapshot> filteredSnapshots;
    QString lowerFilter = filterText.toLower();

    for (const Models::Snapshot& snapshot : m_allSnapshots) {
        // 在多个字段中搜索
        bool matches = false;

        // 搜索快照ID
        if (snapshot.id.toLower().contains(lowerFilter)) {
            matches = true;
        }

        // 搜索主机名
        if (!matches && snapshot.hostname.toLower().contains(lowerFilter)) {
            matches = true;
        }

        // 搜索路径
        if (!matches) {
            for (const QString& path : snapshot.paths) {
                if (path.toLower().contains(lowerFilter)) {
                    matches = true;
                    break;
                }
            }
        }

        // 搜索标签
        if (!matches) {
            for (const QString& tag : snapshot.tags) {
                if (tag.toLower().contains(lowerFilter)) {
                    matches = true;
                    break;
                }
            }
        }

        if (matches) {
            filteredSnapshots.append(snapshot);
        }
    }

    // 显示筛选后的快照
    displaySnapshots(filteredSnapshots);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("筛选结果: %1 / %2 个快照")
            .arg(filteredSnapshots.size())
            .arg(m_allSnapshots.size()));
}

void RestorePage::onQuickRestore()
{
    // 获取选中的快照
    int currentRow = ui->snapshotTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要恢复的快照"));
        return;
    }

    // 获取快照ID
    QTableWidgetItem* idItem = ui->snapshotTable->item(currentRow, 0);
    if (!idItem) {
        QMessageBox::warning(this, tr("警告"), tr("无法获取快照信息"));
        return;
    }
    QString snapshotId = idItem->data(Qt::UserRole).toString();

    // 获取恢复目标路径
    QString targetPath = ui->targetPathEdit->text().trimmed();
    if (targetPath.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择恢复目标路径"));
        return;
    }

    // 检查目标路径是否存在
    if (!QDir(targetPath).exists()) {
        QMessageBox::warning(this, tr("警告"),
            tr("目标路径不存在：%1").arg(targetPath));
        return;
    }

    // 获取快照时间（用于显示）
    QTableWidgetItem* timeItem = ui->snapshotTable->item(currentRow, 1);
    QString snapshotTime = timeItem ? timeItem->text() : tr("未知");

    // 构建确认消息
    QString confirmMessage = tr("确认执行快速恢复？\n\n"
                               "快照ID: %1\n"
                               "创建时间: %2\n"
                               "恢复到: %3")
                               .arg(snapshotId.left(8))
                               .arg(snapshotTime)
                               .arg(targetPath);

    // 如果勾选了包含特定文件/目录，添加到确认消息中
    if (ui->includeCheckBox->isChecked() && !ui->includeEdit->text().isEmpty()) {
        confirmMessage += tr("\n包含路径: %1").arg(ui->includeEdit->text());
    }

    confirmMessage += tr("\n\n此操作将恢复整个快照的所有文件到指定目录。");

    // 弹出确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认恢复"),
        confirmMessage,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 创建恢复选项
    Models::RestoreOptions options;
    options.targetPath = targetPath;
    options.verify = false;  // 快速恢复默认不验证数据
    options.overwritePolicy = Models::RestoreOptions::Always;  // 允许覆盖现有文件

    // 如果勾选了包含特定文件/目录
    if (ui->includeCheckBox->isChecked() && !ui->includeEdit->text().isEmpty()) {
        QString includePath = ui->includeEdit->text().trimmed();
        if (!includePath.isEmpty()) {
            options.includePaths = QStringList() << includePath;
        }
    }

    // 执行恢复
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始快速恢复快照 %1 到 %2").arg(snapshotId).arg(targetPath));

    Core::RestoreManager* restoreMgr = Core::RestoreManager::instance();

    // 创建进度对话框
    ProgressDialog* progressDialog = new ProgressDialog(this);
    progressDialog->setTitle(tr("快速恢复进度"));
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
            this, [this, progressDialog, snapshotId, targetPath](bool success) {
        progressDialog->close();
        progressDialog->deleteLater();

        if (success) {
            QMessageBox::information(this, tr("成功"),
                tr("快速恢复成功！\n\n快照：%1\n目标路径：%2")
                    .arg(snapshotId.left(8))
                    .arg(targetPath));

            Utils::Logger::instance()->log(Utils::Logger::Info, "快速恢复成功");
        } else {
            QMessageBox::critical(this, tr("失败"),
                tr("快速恢复失败，请查看日志了解详情"));

            Utils::Logger::instance()->log(Utils::Logger::Error, "快速恢复失败");
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

} // namespace UI
} // namespace ResticGUI
