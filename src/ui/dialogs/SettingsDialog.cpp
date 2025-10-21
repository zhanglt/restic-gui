/**
 * @file SettingsDialog.cpp
 * @brief 设置对话框实现
 */

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "../../data/ConfigManager.h"
#include <QFileDialog>
#include <QPushButton>
#include <QDialogButtonBox>

namespace ResticGUI {
namespace UI {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // 连接信号
    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsDialog::onBrowseResticPath);
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::onApplyClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        saveSettings();
        accept();
    });

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadSettings()
{
    Data::ConfigManager* config = Data::ConfigManager::instance();

    ui->resticPathEdit->setText(config->getResticPath());
    ui->autoStartCheck->setChecked(config->getAutoStart());
    ui->minimizeToTrayCheck->setChecked(config->getMinimizeToTray());

    ui->maxParallelSpin->setValue(config->getMaxParallelBackups());
    ui->logRetentionSpin->setValue(config->getLogRetentionDays());
    ui->showNotificationsCheck->setChecked(config->getShowBackupNotifications());

    ui->passwordModeCombo->setCurrentIndex(config->getPasswordStorageMode());
    ui->cacheTimeoutSpin->setValue(config->getPasswordCacheTimeout());

    ui->useProxyCheck->setChecked(config->getUseProxy());
    ui->proxyHostEdit->setText(config->getProxyHost());
    ui->proxyPortSpin->setValue(config->getProxyPort());
}

void SettingsDialog::saveSettings()
{
    Data::ConfigManager* config = Data::ConfigManager::instance();

    config->setResticPath(ui->resticPathEdit->text());
    config->setAutoStart(ui->autoStartCheck->isChecked());
    config->setMinimizeToTray(ui->minimizeToTrayCheck->isChecked());

    config->setMaxParallelBackups(ui->maxParallelSpin->value());
    config->setLogRetentionDays(ui->logRetentionSpin->value());
    config->setShowBackupNotifications(ui->showNotificationsCheck->isChecked());

    config->setPasswordStorageMode(ui->passwordModeCombo->currentIndex());
    config->setPasswordCacheTimeout(ui->cacheTimeoutSpin->value());

    config->setUseProxy(ui->useProxyCheck->isChecked());
    config->setProxyHost(ui->proxyHostEdit->text());
    config->setProxyPort(ui->proxyPortSpin->value());

    config->sync();
}

void SettingsDialog::onBrowseResticPath()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择 Restic 可执行文件"),
        QString(),
        tr("可执行文件 (*.exe);;所有文件 (*.*)")
    );

    if (!filePath.isEmpty()) {
        ui->resticPathEdit->setText(filePath);
    }
}

void SettingsDialog::onApplyClicked()
{
    saveSettings();
}

} // namespace UI
} // namespace ResticGUI
