#include "CreateTaskDialog.h"
#include "../../core/RepositoryManager.h"
#include "../../utils/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QInputDialog>
#include <QMouseEvent>
#include <QEvent>
#include <QScrollArea>

namespace ResticGUI {
namespace UI {

CreateTaskDialog::CreateTaskDialog(QWidget* parent)
    : QDialog(parent),
      m_advancedExpanded(false),
      m_includeExpanded(false),
      m_backupParamsExpanded(false)
{
    setWindowTitle(tr("创建备份任务"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(550, 450);
    resize(580, 500);

    // 设置对话框样式
    setStyleSheet(
        "QDialog {"
        "    background-color: white;"
        "}"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 20);

    // 表单布局
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(15);
    formLayout->setHorizontalSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 任务名称
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("例如：每日备份"));
    m_nameEdit->setMinimumHeight(30);
    m_nameEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #A0A0A0;"
        "}"
    );

    QLabel* nameLabel = new QLabel(tr("任务名称:"), this);
    nameLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    formLayout->addRow(nameLabel, m_nameEdit);

    // 选择仓库
    m_repoComboBox = new QComboBox(this);
    m_repoComboBox->setMinimumHeight(30);
    m_repoComboBox->setStyleSheet(
        "QComboBox {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QComboBox:hover {"
        "    border-color: #A0A0A0;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 25px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #666666;"
        "    margin-right: 6px;"
        "}"
        "QComboBox:disabled {"
        "    background-color: #F5F5F5;"
        "    color: #999999;"
        "}"
    );

    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    if (repositories.isEmpty()) {
        m_repoComboBox->addItem(tr("(无可用仓库)"), 0);
        m_repoComboBox->setEnabled(false);
    } else {
        for (const Models::Repository& repo : repositories) {
            m_repoComboBox->addItem(repo.name, repo.id);
        }
    }

    QLabel* repoLabel = new QLabel(tr("目标仓库:"), this);
    repoLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    formLayout->addRow(repoLabel, m_repoComboBox);

    // 源路径（简化版，只支持单个路径）
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setMinimumHeight(30);
    m_pathEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #A0A0A0;"
        "}"
    );

    QPushButton* browseButton = new QPushButton(tr("浏览..."), this);
    browseButton->setMinimumHeight(30);
    browseButton->setMinimumWidth(75);
    browseButton->setCursor(Qt::PointingHandCursor);
    browseButton->setStyleSheet(
        "QPushButton {"
        "    padding: 5px 14px;"
        "    border: 1px solid #4A90E2;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    color: #4A90E2;"
        "    font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4A90E2;"
        "    color: white;"
        "    border-color: #4A90E2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #357ABD;"
        "    border-color: #357ABD;"
        "}"
    );

    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->setSpacing(10);
    pathLayout->addWidget(m_pathEdit, 1);
    pathLayout->addWidget(browseButton);

    QLabel* pathLabel = new QLabel(tr("备份路径:"), this);
    pathLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    formLayout->addRow(pathLabel, pathLayout);

    connect(browseButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择备份目录"));
        if (!dir.isEmpty()) {
            m_pathEdit->setText(dir);
        }
    });

    // 标签
    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText(tr("例如：重要,每日 (用逗号分隔多个标签)"));
    m_tagsEdit->setMinimumHeight(30);
    m_tagsEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #A0A0A0;"
        "}"
    );

    QLabel* tagsLabel = new QLabel(tr("标签 (可选):"), this);
    tagsLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    formLayout->addRow(tagsLabel, m_tagsEdit);

    // 计划类型
    m_scheduleComboBox = new QComboBox(this);
    m_scheduleComboBox->setMinimumHeight(30);
    m_scheduleComboBox->setStyleSheet(
        "QComboBox {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QComboBox:hover {"
        "    border-color: #A0A0A0;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 25px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #666666;"
        "    margin-right: 6px;"
        "}"
    );
    m_scheduleComboBox->addItem(tr("手动"), static_cast<int>(Models::Schedule::Manual));
    m_scheduleComboBox->addItem(tr("每分钟"), static_cast<int>(Models::Schedule::Minutely));
    m_scheduleComboBox->addItem(tr("每小时"), static_cast<int>(Models::Schedule::Hourly));
    m_scheduleComboBox->addItem(tr("每天"), static_cast<int>(Models::Schedule::Daily));
    m_scheduleComboBox->addItem(tr("每周"), static_cast<int>(Models::Schedule::Weekly));
    m_scheduleComboBox->addItem(tr("每月"), static_cast<int>(Models::Schedule::Monthly));

    QLabel* scheduleLabel = new QLabel(tr("计划:"), this);
    scheduleLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    formLayout->addRow(scheduleLabel, m_scheduleComboBox);

    mainLayout->addLayout(formLayout);

    // 添加高级排除选项
    setupAdvancedOptions();
    mainLayout->addWidget(m_advancedGroup);

    // 添加高级包含选项
    setupIncludeOptions();
    mainLayout->addWidget(m_includeGroup);

    // 添加高级备份参数选项
    setupBackupParams();
    mainLayout->addWidget(m_backupParamsGroup);

    mainLayout->addStretch();

    // 按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 美化按钮
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(30);
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(30);
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumWidth(75);
    buttonBox->button(QDialogButtonBox::Cancel)->setMinimumWidth(75);
    buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);

    buttonBox->setStyleSheet(
        "QPushButton {"
        "    padding: 5px 14px;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "}"
        "QPushButton:first-child {"  // OK按钮
        "    background-color: #4A90E2;"
        "    color: white;"
        "    border: 1px solid #4A90E2;"
        "    font-weight: bold;"
        "}"
        "QPushButton:first-child:hover {"
        "    background-color: #357ABD;"
        "    border-color: #357ABD;"
        "}"
        "QPushButton:first-child:pressed {"
        "    background-color: #2E6BA8;"
        "}"
        "QPushButton:last-child {"  // Cancel按钮
        "    background-color: white;"
        "    color: #666666;"
        "    border: 1px solid #CCCCCC;"
        "}"
        "QPushButton:last-child:hover {"
        "    background-color: #F5F5F5;"
        "    border-color: #999999;"
        "    color: #333333;"
        "}"
        "QPushButton:last-child:pressed {"
        "    background-color: #E0E0E0;"
        "}"
    );

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "CreateTaskDialog: 点击确定按钮");

        // 验证输入
        if (m_nameEdit->text().isEmpty()) {
            Utils::Logger::instance()->log(Utils::Logger::Warning, "任务名称为空");
            QMessageBox::warning(this, tr("警告"), tr("请输入任务名称"));
            return;
        }

        int repoId = m_repoComboBox->currentData().toInt();
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("选中的仓库ID: %1").arg(repoId));

        if (repoId <= 0) {
            Utils::Logger::instance()->log(Utils::Logger::Warning, "未选择有效仓库");
            QMessageBox::warning(this, tr("警告"), tr("请先创建一个仓库"));
            return;
        }

        if (m_pathEdit->text().isEmpty()) {
            Utils::Logger::instance()->log(Utils::Logger::Warning, "备份路径为空");
            QMessageBox::warning(this, tr("警告"), tr("请选择备份路径"));
            return;
        }

        // 填充任务信息
        m_task.name = m_nameEdit->text();
        m_task.repositoryId = repoId;
        m_task.sourcePaths.clear();
        m_task.sourcePaths.append(m_pathEdit->text());

        // 处理标签（将逗号分隔的字符串转换为 QStringList）
        m_task.tags.clear();
        QString tagsText = m_tagsEdit->text().trimmed();
        if (!tagsText.isEmpty()) {
            QStringList tagList = tagsText.split(',', Qt::SkipEmptyParts);
            for (QString& tag : tagList) {
                tag = tag.trimmed();
                if (!tag.isEmpty()) {
                    m_task.tags.append(tag);
                }
            }
        }

        m_task.schedule.type = static_cast<Models::Schedule::Type>(m_scheduleComboBox->currentData().toInt());
        m_task.enabled = true;

        // 收集高级选项数据
        // 1. 排除模式
        m_task.excludePatterns.clear();
        for (int i = 0; i < m_excludeListWidget->count(); ++i) {
            m_task.excludePatterns.append(m_excludeListWidget->item(i)->text());
        }

        // 2. 排除文件
        m_task.excludeFile = m_excludeFileEdit->text().trimmed();

        // 3. 文件大小限制
        QString sizeValue = m_excludeLargerEdit->text().trimmed();
        if (!sizeValue.isEmpty()) {
            QString unit = m_excludeLargerUnitCombo->currentData().toString();
            m_task.excludeLargerThan = sizeValue + unit;
        } else {
            m_task.excludeLargerThan.clear();
        }

        // 4. 排除缓存
        m_task.excludeCaches = m_excludeCachesCheck->isChecked();

        // 5. 条件排除
        m_task.excludeIfPresent = m_excludeIfPresentEdit->text().trimmed();

        // 收集包含文件选项数据
        m_task.filesFrom = m_filesFromEdit->text().trimmed();
        m_task.filesFromVerbatim = m_filesFromVerbatimEdit->text().trimmed();
        m_task.filesFromRaw = m_filesFromRawEdit->text().trimmed();

        // 收集备份参数数据
        m_task.noScan = m_noScanCheck->isChecked();
        m_task.compression = m_compressionCombo->currentData().toString();
        if (m_task.compression == "auto") {
            m_task.compression.clear(); // 默认值不需要传递
        }
        m_task.noExtraVerify = m_noExtraVerifyCheck->isChecked();

        QString readConcurrencyText = m_readConcurrencyEdit->text().trimmed();
        m_task.readConcurrency = readConcurrencyText.isEmpty() ? 0 : readConcurrencyText.toInt();

        QString packSizeText = m_packSizeEdit->text().trimmed();
        m_task.packSize = packSizeText.isEmpty() ? 0 : packSizeText.toInt();

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("创建任务对话框完成: name=%1, repoId=%2, path=%3, tags=%4, scheduleType=%5, excludePatterns=%6, filesFrom=%7, noScan=%8, compression=%9")
                .arg(m_task.name)
                .arg(m_task.repositoryId)
                .arg(m_pathEdit->text())
                .arg(m_task.tags.join(", "))
                .arg(static_cast<int>(m_task.schedule.type))
                .arg(m_task.excludePatterns.join(", "))
                .arg(m_task.filesFrom)
                .arg(m_task.noScan)
                .arg(m_task.compression));

        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

CreateTaskDialog::~CreateTaskDialog()
{
}

Models::BackupTask CreateTaskDialog::getTask() const
{
    return m_task;
}

void CreateTaskDialog::setTask(const Models::BackupTask& task)
{
    m_task = task;

    // 填充任务名称
    m_nameEdit->setText(task.name);

    // 设置仓库选择
    for (int i = 0; i < m_repoComboBox->count(); ++i) {
        if (m_repoComboBox->itemData(i).toInt() == task.repositoryId) {
            m_repoComboBox->setCurrentIndex(i);
            break;
        }
    }

    // 设置备份路径（只显示第一个路径）
    if (!task.sourcePaths.isEmpty()) {
        m_pathEdit->setText(task.sourcePaths.first());
    }

    // 设置标签（将 QStringList 转换为逗号分隔的字符串）
    if (!task.tags.isEmpty()) {
        m_tagsEdit->setText(task.tags.join(", "));
    }

    // 设置计划类型
    for (int i = 0; i < m_scheduleComboBox->count(); ++i) {
        if (m_scheduleComboBox->itemData(i).toInt() == static_cast<int>(task.schedule.type)) {
            m_scheduleComboBox->setCurrentIndex(i);
            break;
        }
    }

    // 填充高级选项数据
    // 1. 排除模式
    m_excludeListWidget->clear();
    for (const QString& pattern : task.excludePatterns) {
        m_excludeListWidget->addItem(pattern);
    }

    // 2. 排除文件
    m_excludeFileEdit->setText(task.excludeFile);

    // 3. 文件大小限制
    if (!task.excludeLargerThan.isEmpty()) {
        // 解析大小和单位 (如 "100m" -> "100" + "m")
        QString sizeStr = task.excludeLargerThan;
        QString unit;
        QString value;

        // 提取最后一个字符作为单位
        if (sizeStr.length() > 0 && !sizeStr.at(sizeStr.length() - 1).isDigit()) {
            unit = sizeStr.right(1).toLower();
            value = sizeStr.left(sizeStr.length() - 1);
        } else {
            value = sizeStr;
            unit = "";
        }

        m_excludeLargerEdit->setText(value);

        // 设置单位下拉框
        for (int i = 0; i < m_excludeLargerUnitCombo->count(); ++i) {
            if (m_excludeLargerUnitCombo->itemData(i).toString() == unit) {
                m_excludeLargerUnitCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    // 4. 排除缓存
    m_excludeCachesCheck->setChecked(task.excludeCaches);

    // 5. 条件排除
    m_excludeIfPresentEdit->setText(task.excludeIfPresent);

    // 填充包含文件选项数据
    m_filesFromEdit->setText(task.filesFrom);
    m_filesFromVerbatimEdit->setText(task.filesFromVerbatim);
    m_filesFromRawEdit->setText(task.filesFromRaw);

    // 填充备份参数数据
    m_noScanCheck->setChecked(task.noScan);
    m_noExtraVerifyCheck->setChecked(task.noExtraVerify);

    // 设置压缩级别
    QString compression = task.compression.isEmpty() ? "auto" : task.compression;
    for (int i = 0; i < m_compressionCombo->count(); ++i) {
        if (m_compressionCombo->itemData(i).toString() == compression) {
            m_compressionCombo->setCurrentIndex(i);
            break;
        }
    }

    // 设置文件读取并发数
    if (task.readConcurrency > 0) {
        m_readConcurrencyEdit->setText(QString::number(task.readConcurrency));
    } else {
        m_readConcurrencyEdit->clear();
    }

    // 设置包大小
    if (task.packSize > 0) {
        m_packSizeEdit->setText(QString::number(task.packSize));
    } else {
        m_packSizeEdit->clear();
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("已填充任务数据到对话框: name=%1, repoId=%2, path=%3, scheduleType=%4, excludePatterns=%5, filesFrom=%6, noScan=%7, compression=%8")
            .arg(task.name)
            .arg(task.repositoryId)
            .arg(task.sourcePaths.isEmpty() ? "" : task.sourcePaths.first())
            .arg(static_cast<int>(task.schedule.type))
            .arg(task.excludePatterns.join(", "))
            .arg(task.filesFrom)
            .arg(task.noScan)
            .arg(task.compression));
}

bool CreateTaskDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // 只在点击标题区域时切换
        if (mouseEvent->y() < 30) {
            if (obj == m_advancedGroup) {
                onAdvancedOptionsToggled();
                return true;
            } else if (obj == m_includeGroup) {
                onIncludeOptionsToggled();
                return true;
            } else if (obj == m_backupParamsGroup) {
                onBackupParamsToggled();
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void CreateTaskDialog::setupAdvancedOptions()
{
    m_advancedGroup = new QGroupBox(tr("▶ 高级选项-排除"), this);
    m_advancedGroup->setCheckable(false);
    m_advancedGroup->setStyleSheet(
        "QGroupBox {"
        "    font-size: 10pt;"
        "    font-weight: bold;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 5px 10px;"
        "    color: #4A90E2;"
        "}"
    );
    m_advancedGroup->setCursor(Qt::PointingHandCursor);

    // 使标题可点击
    m_advancedGroup->installEventFilter(this);

    QVBoxLayout* advLayout = new QVBoxLayout(m_advancedGroup);
    advLayout->setSpacing(5);
    advLayout->setContentsMargins(15, 20, 15, 15);

    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(m_advancedGroup);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setMaximumHeight(300); // 限制最大高度
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(6);
    contentLayout->setContentsMargins(5, 5, 5, 5);

    // 1. 排除模式列表
    QLabel* excludeLabel = new QLabel(tr("排除模式:"), contentWidget);
    excludeLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(excludeLabel);

    m_excludeListWidget = new QListWidget(contentWidget);
    m_excludeListWidget->setMaximumHeight(70);
    m_excludeListWidget->setMinimumHeight(50);
    m_excludeListWidget->setStyleSheet(
        "QListWidget {"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    padding: 5px;"
        "}"
    );
    contentLayout->addWidget(m_excludeListWidget);

    QHBoxLayout* excludeBtnLayout = new QHBoxLayout();
    excludeBtnLayout->setSpacing(6);

    m_addExcludeBtn = new QPushButton(tr("添加"), contentWidget);
    m_removeExcludeBtn = new QPushButton(tr("删除"), contentWidget);
    m_editExcludeBtn = new QPushButton(tr("编辑"), contentWidget);

    QString btnStyle =
        "QPushButton {"
        "    padding: 2px 8px;"
        "    border: 1px solid #4A90E2;"
        "    border-radius: 2px;"
        "    background-color: white;"
        "    color: #4A90E2;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4A90E2;"
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #357ABD;"
        "}";

    m_addExcludeBtn->setStyleSheet(btnStyle);
    m_removeExcludeBtn->setStyleSheet(btnStyle);
    m_editExcludeBtn->setStyleSheet(btnStyle);
    m_addExcludeBtn->setCursor(Qt::PointingHandCursor);
    m_removeExcludeBtn->setCursor(Qt::PointingHandCursor);
    m_editExcludeBtn->setCursor(Qt::PointingHandCursor);

    excludeBtnLayout->addWidget(m_addExcludeBtn);
    excludeBtnLayout->addWidget(m_removeExcludeBtn);
    excludeBtnLayout->addWidget(m_editExcludeBtn);
    excludeBtnLayout->addStretch();
    contentLayout->addLayout(excludeBtnLayout);

    connect(m_addExcludeBtn, &QPushButton::clicked, this, &CreateTaskDialog::onAddExcludePattern);
    connect(m_removeExcludeBtn, &QPushButton::clicked, this, &CreateTaskDialog::onRemoveExcludePattern);
    connect(m_editExcludeBtn, &QPushButton::clicked, this, &CreateTaskDialog::onEditExcludePattern);
    connect(m_excludeListWidget, &QListWidget::itemDoubleClicked, this, &CreateTaskDialog::onEditExcludePattern);

    // 2. 从文件导入排除规则
    QLabel* excludeFileLabel = new QLabel(tr("从文件导入排除规则:"), contentWidget);
    excludeFileLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(excludeFileLabel);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    fileLayout->setSpacing(6);

    m_excludeFileEdit = new QLineEdit(contentWidget);
    m_excludeFileEdit->setPlaceholderText(tr("选择包含排除规则的文件"));
    m_excludeFileEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}"
    );

    m_browseExcludeFileBtn = new QPushButton(tr("浏览..."), contentWidget);
    m_browseExcludeFileBtn->setStyleSheet(btnStyle);
    m_browseExcludeFileBtn->setCursor(Qt::PointingHandCursor);
    m_browseExcludeFileBtn->setMinimumWidth(75);

    fileLayout->addWidget(m_excludeFileEdit, 1);
    fileLayout->addWidget(m_browseExcludeFileBtn);
    contentLayout->addLayout(fileLayout);

    connect(m_browseExcludeFileBtn, &QPushButton::clicked, this, &CreateTaskDialog::onBrowseExcludeFile);

    // 3. 文件大小限制
    QLabel* sizeLabel = new QLabel(tr("排除大于此大小的文件:"), contentWidget);
    sizeLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(sizeLabel);

    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->setSpacing(6);

    m_excludeLargerEdit = new QLineEdit(contentWidget);
    m_excludeLargerEdit->setPlaceholderText(tr("例如: 100"));
    m_excludeLargerEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}"
    );

    m_excludeLargerUnitCombo = new QComboBox(contentWidget);
    m_excludeLargerUnitCombo->addItem("B (字节)", "");
    m_excludeLargerUnitCombo->addItem("K (KB)", "k");
    m_excludeLargerUnitCombo->addItem("M (MB)", "m");
    m_excludeLargerUnitCombo->addItem("G (GB)", "g");
    m_excludeLargerUnitCombo->addItem("T (TB)", "t");
    m_excludeLargerUnitCombo->setCurrentIndex(2); // 默认 MB
    m_excludeLargerUnitCombo->setStyleSheet(
        "QComboBox {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "    min-width: 80px;"
        "}"
    );

    sizeLayout->addWidget(m_excludeLargerEdit, 1);
    sizeLayout->addWidget(m_excludeLargerUnitCombo);
    contentLayout->addLayout(sizeLayout);

    // 4. 排除缓存目录
    m_excludeCachesCheck = new QCheckBox(tr("排除包含 CACHEDIR.TAG 的缓存目录"), contentWidget);
    m_excludeCachesCheck->setStyleSheet("QCheckBox { font-size: 8pt; color: #333333; }");
    contentLayout->addWidget(m_excludeCachesCheck);

    // 5. 条件排除
    QLabel* ifPresentLabel = new QLabel(tr("排除包含此文件的目录:"), contentWidget);
    ifPresentLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(ifPresentLabel);

    m_excludeIfPresentEdit = new QLineEdit(contentWidget);
    m_excludeIfPresentEdit->setPlaceholderText(tr("例如: .nobackup"));
    m_excludeIfPresentEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}"
    );
    contentLayout->addWidget(m_excludeIfPresentEdit);

    // 将内容widget设置到滚动区域
    scrollArea->setWidget(contentWidget);
    advLayout->addWidget(scrollArea);

    // 初始状态：隐藏滚动区域
    scrollArea->setVisible(false);
}

void CreateTaskDialog::onAdvancedOptionsToggled()
{
    m_advancedExpanded = !m_advancedExpanded;

    // 获取滚动区域
    QScrollArea* scrollArea = m_advancedGroup->findChild<QScrollArea*>();
    if (scrollArea) {
        scrollArea->setVisible(m_advancedExpanded);
    }

    // 更新标题
    m_advancedGroup->setTitle(m_advancedExpanded ? tr("▼ 高级选项-排除") : tr("▶ 高级选项-排除"));

    // 调整窗口大小 - 使用固定的合理高度
    if (m_advancedExpanded) {
        setMaximumHeight(16777215); // 移除最大高度限制
        resize(width(), 750);
    } else {
        resize(width(), 500);
    }
}

void CreateTaskDialog::onAddExcludePattern()
{
    bool ok;
    QString pattern = QInputDialog::getText(this, tr("添加排除模式"),
                                           tr("请输入排除模式:"),
                                           QLineEdit::Normal, "", &ok);
    if (ok && !pattern.isEmpty()) {
        m_excludeListWidget->addItem(pattern);
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("添加排除模式: %1").arg(pattern));
    }
}

void CreateTaskDialog::onRemoveExcludePattern()
{
    QListWidgetItem* item = m_excludeListWidget->currentItem();
    if (item) {
        QString pattern = item->text();
        delete item;
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("删除排除模式: %1").arg(pattern));
    }
}

void CreateTaskDialog::onEditExcludePattern()
{
    QListWidgetItem* item = m_excludeListWidget->currentItem();
    if (item) {
        bool ok;
        QString newPattern = QInputDialog::getText(this, tr("编辑排除模式"),
                                                   tr("请输入排除模式:"),
                                                   QLineEdit::Normal,
                                                   item->text(), &ok);
        if (ok && !newPattern.isEmpty()) {
            item->setText(newPattern);
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("编辑排除模式: %1").arg(newPattern));
        }
    }
}

void CreateTaskDialog::onBrowseExcludeFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择排除规则文件"),
                                                    QString(),
                                                    tr("文本文件 (*.txt);;所有文件 (*)"));
    if (!fileName.isEmpty()) {
        m_excludeFileEdit->setText(fileName);
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("选择排除文件: %1").arg(fileName));
    }
}

void CreateTaskDialog::setupIncludeOptions()
{
    m_includeGroup = new QGroupBox(tr("▶ 高级选项-包含"), this);
    m_includeGroup->setCheckable(false);
    m_includeGroup->setStyleSheet(
        "QGroupBox {"
        "    font-size: 10pt;"
        "    font-weight: bold;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 5px 10px;"
        "    color: #4A90E2;"
        "}"
    );
    m_includeGroup->setCursor(Qt::PointingHandCursor);
    m_includeGroup->installEventFilter(this);

    QVBoxLayout* includeLayout = new QVBoxLayout(m_includeGroup);
    includeLayout->setSpacing(5);
    includeLayout->setContentsMargins(15, 20, 15, 15);

    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(m_includeGroup);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setMaximumHeight(200);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(6);
    contentLayout->setContentsMargins(5, 5, 5, 5);

    QString inputStyle =
        "QLineEdit {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}";

    QString btnStyle =
        "QPushButton {"
        "    padding: 2px 8px;"
        "    border: 1px solid #4A90E2;"
        "    border-radius: 2px;"
        "    background-color: white;"
        "    color: #4A90E2;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4A90E2;"
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #357ABD;"
        "}";

    // 1. files-from
    QLabel* filesFromLabel = new QLabel(tr("从文件读取模式列表:"), contentWidget);
    filesFromLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(filesFromLabel);

    QHBoxLayout* filesFromLayout = new QHBoxLayout();
    filesFromLayout->setSpacing(6);

    m_filesFromEdit = new QLineEdit(contentWidget);
    m_filesFromEdit->setPlaceholderText(tr("每行一个模式，支持通配符"));
    m_filesFromEdit->setStyleSheet(inputStyle);

    m_browseFilesFromBtn = new QPushButton(tr("浏览..."), contentWidget);
    m_browseFilesFromBtn->setStyleSheet(btnStyle);
    m_browseFilesFromBtn->setCursor(Qt::PointingHandCursor);
    m_browseFilesFromBtn->setMinimumWidth(65);

    filesFromLayout->addWidget(m_filesFromEdit, 1);
    filesFromLayout->addWidget(m_browseFilesFromBtn);
    contentLayout->addLayout(filesFromLayout);

    connect(m_browseFilesFromBtn, &QPushButton::clicked, this, &CreateTaskDialog::onBrowseFilesFrom);

    // 2. files-from-verbatim
    QLabel* verbatimLabel = new QLabel(tr("从文件逐字读取路径:"), contentWidget);
    verbatimLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(verbatimLabel);

    QHBoxLayout* verbatimLayout = new QHBoxLayout();
    verbatimLayout->setSpacing(6);

    m_filesFromVerbatimEdit = new QLineEdit(contentWidget);
    m_filesFromVerbatimEdit->setPlaceholderText(tr("每行一个路径，不展开通配符"));
    m_filesFromVerbatimEdit->setStyleSheet(inputStyle);

    m_browseFilesFromVerbatimBtn = new QPushButton(tr("浏览..."), contentWidget);
    m_browseFilesFromVerbatimBtn->setStyleSheet(btnStyle);
    m_browseFilesFromVerbatimBtn->setCursor(Qt::PointingHandCursor);
    m_browseFilesFromVerbatimBtn->setMinimumWidth(65);

    verbatimLayout->addWidget(m_filesFromVerbatimEdit, 1);
    verbatimLayout->addWidget(m_browseFilesFromVerbatimBtn);
    contentLayout->addLayout(verbatimLayout);

    connect(m_browseFilesFromVerbatimBtn, &QPushButton::clicked, this, &CreateTaskDialog::onBrowseFilesFromVerbatim);

    // 3. files-from-raw
    QLabel* rawLabel = new QLabel(tr("从文件读取 NUL 分隔路径:"), contentWidget);
    rawLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(rawLabel);

    QHBoxLayout* rawLayout = new QHBoxLayout();
    rawLayout->setSpacing(6);

    m_filesFromRawEdit = new QLineEdit(contentWidget);
    m_filesFromRawEdit->setPlaceholderText(tr("用于脚本生成的文件列表"));
    m_filesFromRawEdit->setStyleSheet(inputStyle);

    m_browseFilesFromRawBtn = new QPushButton(tr("浏览..."), contentWidget);
    m_browseFilesFromRawBtn->setStyleSheet(btnStyle);
    m_browseFilesFromRawBtn->setCursor(Qt::PointingHandCursor);
    m_browseFilesFromRawBtn->setMinimumWidth(65);

    rawLayout->addWidget(m_filesFromRawEdit, 1);
    rawLayout->addWidget(m_browseFilesFromRawBtn);
    contentLayout->addLayout(rawLayout);

    connect(m_browseFilesFromRawBtn, &QPushButton::clicked, this, &CreateTaskDialog::onBrowseFilesFromRaw);

    scrollArea->setWidget(contentWidget);
    includeLayout->addWidget(scrollArea);

    scrollArea->setVisible(false);
}

void CreateTaskDialog::onIncludeOptionsToggled()
{
    m_includeExpanded = !m_includeExpanded;

    QScrollArea* scrollArea = m_includeGroup->findChild<QScrollArea*>();
    if (scrollArea) {
        scrollArea->setVisible(m_includeExpanded);
    }

    m_includeGroup->setTitle(m_includeExpanded ? tr("▼ 高级选项-包含") : tr("▶ 高级选项-包含"));

    if (m_includeExpanded) {
        setMaximumHeight(16777215);
        int currentHeight = height();
        resize(width(), currentHeight + 220);
    } else {
        int currentHeight = height();
        resize(width(), currentHeight - 220);
    }
}

void CreateTaskDialog::onBrowseFilesFrom()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择包含模式文件"),
                                                    QString(),
                                                    tr("文本文件 (*.txt);;所有文件 (*)"));
    if (!fileName.isEmpty()) {
        m_filesFromEdit->setText(fileName);
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("选择 files-from 文件: %1").arg(fileName));
    }
}

void CreateTaskDialog::onBrowseFilesFromVerbatim()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择路径列表文件"),
                                                    QString(),
                                                    tr("文本文件 (*.txt);;所有文件 (*)"));
    if (!fileName.isEmpty()) {
        m_filesFromVerbatimEdit->setText(fileName);
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("选择 files-from-verbatim 文件: %1").arg(fileName));
    }
}

void CreateTaskDialog::onBrowseFilesFromRaw()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择 NUL 分隔文件"),
                                                    QString(),
                                                    tr("所有文件 (*)"));
    if (!fileName.isEmpty()) {
        m_filesFromRawEdit->setText(fileName);
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("选择 files-from-raw 文件: %1").arg(fileName));
    }
}

void CreateTaskDialog::setupBackupParams()
{
    m_backupParamsGroup = new QGroupBox(tr("▶ 高级选项-备份参数"), this);
    m_backupParamsGroup->setCheckable(false);
    m_backupParamsGroup->setStyleSheet(
        "QGroupBox {"
        "    font-size: 10pt;"
        "    font-weight: bold;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 5px 10px;"
        "    color: #4A90E2;"
        "}"
    );
    m_backupParamsGroup->setCursor(Qt::PointingHandCursor);
    m_backupParamsGroup->installEventFilter(this);

    QVBoxLayout* paramsLayout = new QVBoxLayout(m_backupParamsGroup);
    paramsLayout->setSpacing(5);
    paramsLayout->setContentsMargins(15, 20, 15, 15);

    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(m_backupParamsGroup);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setMaximumHeight(200);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(6);
    contentLayout->setContentsMargins(5, 5, 5, 5);

    QString inputStyle =
        "QLineEdit {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}";

    // 1. 禁用进度扫描
    m_noScanCheck = new QCheckBox(tr("禁用进度扫描 (--no-scan)"), contentWidget);
    m_noScanCheck->setStyleSheet("QCheckBox { font-size: 8pt; color: #333333; }");
    m_noScanCheck->setToolTip(tr("跳过扫描文件以估算进度，可减少网络文件系统的I/O开销"));
    contentLayout->addWidget(m_noScanCheck);

    // 2. 压缩级别
    QLabel* compressionLabel = new QLabel(tr("压缩级别:"), contentWidget);
    compressionLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(compressionLabel);

    m_compressionCombo = new QComboBox(contentWidget);
    m_compressionCombo->addItem(tr("自动 (默认)"), "auto");
    m_compressionCombo->addItem(tr("关闭"), "off");
    m_compressionCombo->addItem(tr("最快"), "fastest");
    m_compressionCombo->addItem(tr("更好"), "better");
    m_compressionCombo->addItem(tr("最大"), "max");
    m_compressionCombo->setCurrentIndex(0);
    m_compressionCombo->setStyleSheet(
        "QComboBox {"
        "    padding: 2px 6px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 2px;"
        "    font-size: 8pt;"
        "    min-height: 20px;"
        "    max-height: 20px;"
        "}");
    m_compressionCombo->setToolTip(tr("设置数据压缩级别（需要仓库格式版本2或更高）"));
    contentLayout->addWidget(m_compressionCombo);

    // 3. 禁用额外验证
    m_noExtraVerifyCheck = new QCheckBox(tr("禁用额外验证 (--no-extra-verify)"), contentWidget);
    m_noExtraVerifyCheck->setStyleSheet("QCheckBox { font-size: 8pt; color: #333333; }");
    m_noExtraVerifyCheck->setToolTip(tr("禁用生成文件的验证以减少CPU使用，但建议定期运行 'restic check --read-data'"));
    contentLayout->addWidget(m_noExtraVerifyCheck);

    // 4. 文件读取并发数
    QLabel* concurrencyLabel = new QLabel(tr("文件读取并发数:"), contentWidget);
    concurrencyLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(concurrencyLabel);

    m_readConcurrencyEdit = new QLineEdit(contentWidget);
    m_readConcurrencyEdit->setPlaceholderText(tr("留空使用默认值"));
    m_readConcurrencyEdit->setStyleSheet(inputStyle);
    m_readConcurrencyEdit->setToolTip(tr("从快速存储设备（如NVMe）备份时，增加此值可提高性能"));
    contentLayout->addWidget(m_readConcurrencyEdit);

    // 5. 包大小
    QLabel* packSizeLabel = new QLabel(tr("包大小 (MiB):"), contentWidget);
    packSizeLabel->setStyleSheet("QLabel { font-size: 8pt; font-weight: bold; color: #333333; }");
    contentLayout->addWidget(packSizeLabel);

    m_packSizeEdit = new QLineEdit(contentWidget);
    m_packSizeEdit->setPlaceholderText(tr("留空使用默认值 (16 MiB)"));
    m_packSizeEdit->setStyleSheet(inputStyle);
    m_packSizeEdit->setToolTip(tr("对于大型仓库或快速上传，可增加包大小。注意会增加临时空间和内存使用"));
    contentLayout->addWidget(m_packSizeEdit);

    scrollArea->setWidget(contentWidget);
    paramsLayout->addWidget(scrollArea);

    scrollArea->setVisible(false);
}

void CreateTaskDialog::onBackupParamsToggled()
{
    m_backupParamsExpanded = !m_backupParamsExpanded;

    QScrollArea* scrollArea = m_backupParamsGroup->findChild<QScrollArea*>();
    if (scrollArea) {
        scrollArea->setVisible(m_backupParamsExpanded);
    }

    m_backupParamsGroup->setTitle(m_backupParamsExpanded ? tr("▼ 高级选项-备份参数") : tr("▶ 高级选项-备份参数"));

    if (m_backupParamsExpanded) {
        setMaximumHeight(16777215);
        int currentHeight = height();
        resize(width(), currentHeight + 220);
    } else {
        int currentHeight = height();
        resize(width(), currentHeight - 220);
    }
}

} // namespace UI
} // namespace ResticGUI
