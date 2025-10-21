#ifndef SNAPSHOTPAGE_H
#define SNAPSHOTPAGE_H

#include <QWidget>

namespace Ui {
class SnapshotPage;
}

namespace ResticGUI {
namespace UI {

class SnapshotPage : public QWidget
{
    Q_OBJECT

public:
    explicit SnapshotPage(QWidget* parent = nullptr);
    ~SnapshotPage();

public slots:
    void loadRepositories();
    void loadSnapshots();

private slots:
    void onRepositoryChanged(int index);
    void onDeleteSnapshot();
    void onBrowseSnapshot();
    void onRestoreSnapshot();
    void onRefresh();

private:
    Ui::SnapshotPage* ui;
    int m_currentRepositoryId;
};

} // namespace UI
} // namespace ResticGUI

#endif // SNAPSHOTPAGE_H
