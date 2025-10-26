#ifndef BACKUPPAGE_H
#define BACKUPPAGE_H

#include <QWidget>

namespace Ui {
class BackupPage;
}

namespace ResticGUI {

namespace UI {
class ProgressDialog; // 前置声明
}

namespace UI {

class BackupPage : public QWidget
{
    Q_OBJECT

public:
    explicit BackupPage(QWidget* parent = nullptr);
    ~BackupPage();

public slots:
    void loadTasks();

private slots:
    void onCreateTask();
    void onEditTask();
    void onDeleteTask();
    void onToggleTask();
    void onRunTask();
    void onRefresh();
    void onPasswordError(int taskId, int repoId);
    void onTaskSelected(int currentRow, int previousRow = -1);

    // 备份进度相关槽函数
    void onBackupStarted(int taskId);
    void onBackupProgress(int percent, const QString& message);
    void onBackupFinished(int taskId, bool success);
    void onBackupCancelled();

private:
    void clearDetails();

    Ui::BackupPage* ui;
    ProgressDialog* m_progressDialog;  // 进度对话框
    int m_currentBackupTaskId;          // 当前执行的备份任务ID
};

} // namespace UI
} // namespace ResticGUI

#endif // BACKUPPAGE_H
