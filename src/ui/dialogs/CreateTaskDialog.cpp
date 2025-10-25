#include "CreateTaskDialog.h"
#include "../../core/RepositoryManager.h"
#include "../../utils/Logger.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

namespace ResticGUI {
namespace UI {

CreateTaskDialog::CreateTaskDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("创建备份任务"));
    setMinimumSize(550, 350);
    resize(580, 380);

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
        m_task.schedule.type = static_cast<Models::Schedule::Type>(m_scheduleComboBox->currentData().toInt());
        m_task.enabled = true;

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("创建任务对话框完成: name=%1, repoId=%2, path=%3, scheduleType=%4")
                .arg(m_task.name)
                .arg(m_task.repositoryId)
                .arg(m_pathEdit->text())
                .arg(static_cast<int>(m_task.schedule.type)));

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

    // 设置计划类型
    for (int i = 0; i < m_scheduleComboBox->count(); ++i) {
        if (m_scheduleComboBox->itemData(i).toInt() == static_cast<int>(task.schedule.type)) {
            m_scheduleComboBox->setCurrentIndex(i);
            break;
        }
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("已填充任务数据到对话框: name=%1, repoId=%2, path=%3, scheduleType=%4")
            .arg(task.name)
            .arg(task.repositoryId)
            .arg(task.sourcePaths.isEmpty() ? "" : task.sourcePaths.first())
            .arg(static_cast<int>(task.schedule.type)));
}

} // namespace UI
} // namespace ResticGUI
