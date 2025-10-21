#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

namespace ResticGUI {
namespace UI {

/**
 * @brief 设置对话框
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void onBrowseResticPath();
    void onApplyClicked();

private:
    void loadSettings();
    void saveSettings();

    Ui::SettingsDialog* ui;
};

} // namespace UI
} // namespace ResticGUI

#endif // SETTINGSDIALOG_H
