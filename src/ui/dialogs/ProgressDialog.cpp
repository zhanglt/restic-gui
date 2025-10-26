/**
 * @file ProgressDialog.cpp
 * @brief 进度对话框实现
 */

#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"

namespace ResticGUI {
namespace UI {

ProgressDialog::ProgressDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ProgressDialog)
    , m_completed(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 连接信号
    connect(ui->detailsButton, &QPushButton::toggled,
            this, &ProgressDialog::onDetailsToggled);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &ProgressDialog::onCancelClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setTitle(const QString& title)
{
    ui->titleLabel->setText(title);
}

void ProgressDialog::setProgress(int percent)
{
    ui->progressBar->setValue(percent);
}

void ProgressDialog::setMessage(const QString& message)
{
    ui->messageLabel->setText(message);
}

void ProgressDialog::appendLog(const QString& log)
{
    ui->logText->append(log);
}

void ProgressDialog::setCompleted(bool success)
{
    m_completed = true;
    ui->progressBar->setValue(100);

    if (success) {
        setTitle(tr("操作完成"));
        setMessage(tr("操作已成功完成！"));
    } else {
        setTitle(tr("操作失败"));
        setMessage(tr("操作失败，请查看详细日志。"));
    }

    ui->cancelButton->setVisible(false);
    ui->closeButton->setVisible(true);
}

void ProgressDialog::onCancelClicked()
{
    if (m_completed) {
        reject();
        return;
    }

    emit cancelled();
    ui->cancelButton->setEnabled(false);
    setMessage(tr("正在取消..."));
}

void ProgressDialog::onDetailsToggled(bool checked)
{
    ui->logText->setVisible(checked);

    if (checked) {
        ui->detailsButton->setText(tr("隐藏详情"));
        resize(500, 500);
    } else {
        ui->detailsButton->setText(tr("显示详情"));
        resize(500, 300);
    }
}

} // namespace UI
} // namespace ResticGUI
