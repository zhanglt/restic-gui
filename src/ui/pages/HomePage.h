#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

namespace Ui {
class HomePage;
}

namespace ResticGUI {
namespace UI {

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);
    ~HomePage();

signals:
    void navigateToPage(int pageIndex);

public slots:
    void refreshData();  // 公共刷新方法

private slots:
    void onCreateRepository();
    void onCreateTask();
    void onRestoreData();

private:
    void loadDashboardData();
    void loadRecentActivities();

    Ui::HomePage* ui;
};

} // namespace UI
} // namespace ResticGUI

#endif // HOMEPAGE_H
