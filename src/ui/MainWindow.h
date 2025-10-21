#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace ResticGUI {
namespace UI {

class HomePage;
class RepositoryPage;
class BackupPage;
class SnapshotPage;
class RestorePage;
class StatsPage;

/**
 * @brief 主窗口
 *
 * 采用侧边栏导航 + 内容区域的布局
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onNavigationChanged(int index);
    void onRepositoryChanged();

    // 文件菜单
    void onNewRepository();
    void onOpenRepository();

    // 工具菜单
    void onCheckRepository();
    void onUnlockRepository();
    void onPruneRepository();

    // 帮助菜单
    void showDocumentation();
    void showSettingsDialog();
    void showAboutDialog();

    // 工具栏
    void onBackupNow();
    void onRefresh();
    void onStop();

private:
    void createPages();
    void setupConnections();
    void loadSettings();
    void saveSettings();

    Ui::MainWindow* ui;

    // 页面
    HomePage* m_homePage;
    RepositoryPage* m_repositoryPage;
    BackupPage* m_backupPage;
    SnapshotPage* m_snapshotPage;
    RestorePage* m_restorePage;
    StatsPage* m_statsPage;

    // 当前选中的仓库ID
    int m_currentRepositoryId;
};

} // namespace UI
} // namespace ResticGUI

#endif // MAINWINDOW_H
