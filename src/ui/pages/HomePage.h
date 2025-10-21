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

private:
    void loadDashboardData();

    Ui::HomePage* ui;
};

} // namespace UI
} // namespace ResticGUI

#endif // HOMEPAGE_H
