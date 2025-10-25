#ifndef CREATETASKDIALOG_H
#define CREATETASKDIALOG_H

#include <QDialog>
#include "../../models/BackupTask.h"

class QLineEdit;
class QComboBox;
class QLabel;

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

private:
    Models::BackupTask m_task;

    // UI 控件
    QLineEdit* m_nameEdit;
    QComboBox* m_repoComboBox;
    QLineEdit* m_pathEdit;
    QLineEdit* m_tagsEdit;
    QComboBox* m_scheduleComboBox;
};

} // namespace UI
} // namespace ResticGUI

#endif // CREATETASKDIALOG_H
