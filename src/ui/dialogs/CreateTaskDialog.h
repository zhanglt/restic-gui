#ifndef CREATETASKDIALOG_H
#define CREATETASKDIALOG_H

#include <QDialog>
#include "../../models/BackupTask.h"

class QLineEdit;
class QComboBox;
class QLabel;
class QListWidget;
class QGroupBox;
class QCheckBox;
class QPushButton;

namespace ResticGUI {
namespace UI {

/**
 * @brief 创建/编辑备份任务对话框 (简化版本)
 */
class CreateTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateTaskDialog(QWidget* parent = nullptr);
    ~CreateTaskDialog();

    Models::BackupTask getTask() const;
    void setTask(const Models::BackupTask& task);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onAdvancedOptionsToggled();
    void onAddExcludePattern();
    void onRemoveExcludePattern();
    void onEditExcludePattern();
    void onBrowseExcludeFile();

private:
    void setupAdvancedOptions();

    Models::BackupTask m_task;

    // 基本 UI 控件
    QLineEdit* m_nameEdit;
    QComboBox* m_repoComboBox;
    QLineEdit* m_pathEdit;
    QLineEdit* m_tagsEdit;
    QComboBox* m_scheduleComboBox;

    // 高级选项控件
    QGroupBox* m_advancedGroup;
    bool m_advancedExpanded;

    // 排除模式
    QListWidget* m_excludeListWidget;
    QPushButton* m_addExcludeBtn;
    QPushButton* m_removeExcludeBtn;
    QPushButton* m_editExcludeBtn;

    // 排除文件
    QLineEdit* m_excludeFileEdit;
    QPushButton* m_browseExcludeFileBtn;

    // 文件大小限制
    QLineEdit* m_excludeLargerEdit;
    QComboBox* m_excludeLargerUnitCombo;

    // 其他排除选项
    QCheckBox* m_excludeCachesCheck;
    QLineEdit* m_excludeIfPresentEdit;
};

} // namespace UI
} // namespace ResticGUI

#endif // CREATETASKDIALOG_H
