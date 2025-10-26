#ifndef STATSPAGE_H
#define STATSPAGE_H

#include <QWidget>
#include <QFutureWatcher>

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

protected:
    void showEvent(QShowEvent* event) override;

public slots:
    void loadStats();

private slots:
    void onStatsLoaded();

private:
    QString collectStats();

private:
    Ui::StatsPage* ui;
    bool m_firstShow;
    QFutureWatcher<QString>* m_statsWatcher;
};

} // namespace UI
} // namespace ResticGUI

#endif // STATSPAGE_H
