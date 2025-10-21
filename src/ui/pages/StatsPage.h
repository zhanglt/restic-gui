#ifndef STATSPAGE_H
#define STATSPAGE_H

#include <QWidget>

namespace Ui {
class StatsPage;
}

namespace ResticGUI {
namespace UI {

class StatsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatsPage(QWidget* parent = nullptr);
    ~StatsPage();

public slots:
    void loadStats();

private:
    Ui::StatsPage* ui;
};

} // namespace UI
} // namespace ResticGUI

#endif // STATSPAGE_H
