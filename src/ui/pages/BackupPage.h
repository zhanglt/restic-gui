#ifndef BACKUPPAGE_H
#define BACKUPPAGE_H

#include <QWidget>

namespace Ui {
class BackupPage;
}

namespace ResticGUI {
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
    void onRunTask();
    void onRefresh();
    void onPasswordError(int taskId, int repoId);

private:
    Ui::BackupPage* ui;
};

} // namespace UI
} // namespace ResticGUI

#endif // BACKUPPAGE_H
