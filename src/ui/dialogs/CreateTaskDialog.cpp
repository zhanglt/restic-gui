#include "CreateTaskDialog.h"
#include "../../core/RepositoryManager.h"
#include "../../utils/Logger.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
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
    setMinimumWidth(500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 表单布局
    QFormLayout* formLayout = new QFormLayout();

    // 任务名称
    m_nameEdit = new QLineEdit(this);
    formLayout->addRow(tr("任务名称:"), m_nameEdit);

    // 选择仓库
    m_repoComboBox = new QComboBox(this);
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
    formLayout->addRow(tr("目标仓库:"), m_repoComboBox);

    // 源路径（简化版，只支持单个路径）
    m_pathEdit = new QLineEdit(this);
    QPushButton* browseButton = new QPushButton(tr("浏览..."), this);
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(browseButton);
    formLayout->addRow(tr("备份路径:"), pathLayout);

    connect(browseButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择备份目录"));
        if (!dir.isEmpty()) {
            m_pathEdit->setText(dir);
        }
    });

    // 计划类型
    m_scheduleComboBox = new QComboBox(this);
    m_scheduleComboBox->addItem(tr("手动"), static_cast<int>(Models::Schedule::Manual));
    m_scheduleComboBox->addItem(tr("每分钟"), static_cast<int>(Models::Schedule::Minutely));
    m_scheduleComboBox->addItem(tr("每小时"), static_cast<int>(Models::Schedule::Hourly));
    m_scheduleComboBox->addItem(tr("每天"), static_cast<int>(Models::Schedule::Daily));
    m_scheduleComboBox->addItem(tr("每周"), static_cast<int>(Models::Schedule::Weekly));
    m_scheduleComboBox->addItem(tr("每月"), static_cast<int>(Models::Schedule::Monthly));
    formLayout->addRow(tr("计划:"), m_scheduleComboBox);

    mainLayout->addLayout(formLayout);

    // 按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
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
