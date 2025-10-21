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
    QLineEdit* nameEdit = new QLineEdit(this);
    formLayout->addRow(tr("任务名称:"), nameEdit);

    // 选择仓库
    QComboBox* repoComboBox = new QComboBox(this);
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    QList<Models::Repository> repositories = repoMgr->getAllRepositories();

    if (repositories.isEmpty()) {
        repoComboBox->addItem(tr("(无可用仓库)"), 0);
        repoComboBox->setEnabled(false);
    } else {
        for (const Models::Repository& repo : repositories) {
            repoComboBox->addItem(repo.name, repo.id);
        }
    }
    formLayout->addRow(tr("目标仓库:"), repoComboBox);

    // 源路径（简化版，只支持单个路径）
    QLineEdit* pathEdit = new QLineEdit(this);
    QPushButton* browseButton = new QPushButton(tr("浏览..."), this);
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    formLayout->addRow(tr("备份路径:"), pathLayout);

    connect(browseButton, &QPushButton::clicked, this, [=]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择备份目录"));
        if (!dir.isEmpty()) {
            pathEdit->setText(dir);
        }
    });

    // 计划类型
    QComboBox* scheduleComboBox = new QComboBox(this);
    scheduleComboBox->addItem(tr("手动"), static_cast<int>(Models::Schedule::Manual));
    scheduleComboBox->addItem(tr("每小时"), static_cast<int>(Models::Schedule::Hourly));
    scheduleComboBox->addItem(tr("每天"), static_cast<int>(Models::Schedule::Daily));
    scheduleComboBox->addItem(tr("每周"), static_cast<int>(Models::Schedule::Weekly));
    scheduleComboBox->addItem(tr("每月"), static_cast<int>(Models::Schedule::Monthly));
    formLayout->addRow(tr("计划:"), scheduleComboBox);

    mainLayout->addLayout(formLayout);

    // 按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [=]() {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "CreateTaskDialog: 点击确定按钮");

        // 验证输入
        if (nameEdit->text().isEmpty()) {
            Utils::Logger::instance()->log(Utils::Logger::Warning, "任务名称为空");
            QMessageBox::warning(this, tr("警告"), tr("请输入任务名称"));
            return;
        }

        int repoId = repoComboBox->currentData().toInt();
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("选中的仓库ID: %1").arg(repoId));

        if (repoId <= 0) {
            Utils::Logger::instance()->log(Utils::Logger::Warning, "未选择有效仓库");
            QMessageBox::warning(this, tr("警告"), tr("请先创建一个仓库"));
            return;
        }

        if (pathEdit->text().isEmpty()) {
            Utils::Logger::instance()->log(Utils::Logger::Warning, "备份路径为空");
            QMessageBox::warning(this, tr("警告"), tr("请选择备份路径"));
            return;
        }

        // 填充任务信息
        m_task.name = nameEdit->text();
        m_task.repositoryId = repoId;
        m_task.sourcePaths.clear();
        m_task.sourcePaths.append(pathEdit->text());
        m_task.schedule.type = static_cast<Models::Schedule::Type>(scheduleComboBox->currentData().toInt());
        m_task.enabled = true;

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("创建任务对话框完成: name=%1, repoId=%2, path=%3, scheduleType=%4")
                .arg(m_task.name)
                .arg(m_task.repositoryId)
                .arg(pathEdit->text())
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
    // TODO: 填充界面控件
}

} // namespace UI
} // namespace ResticGUI
