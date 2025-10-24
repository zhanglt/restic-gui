#ifndef SNAPSHOTPAGE_H
#define SNAPSHOTPAGE_H

#include <QWidget>
#include <QFutureWatcher>
#include "../../models/Snapshot.h"

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

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onRepositoryChanged(int index);
    void onDeleteSnapshot();
    void onBrowseSnapshot();
    void onRestoreSnapshot();
    void onRefresh();
    void onSnapshotsLoaded();
    void onSnapshotSelected(int currentRow, int previousRow = -1);
    void onSearch();

private:
    void displaySnapshots(const QList<Models::Snapshot>& snapshots);
    void showLoadingIndicator(bool show);
    void clearDetails();

    Ui::SnapshotPage* ui;
    int m_currentRepositoryId;
    bool m_firstShow;
    bool m_isLoading;
    QFutureWatcher<QList<Models::Snapshot>>* m_snapshotWatcher;
};

} // namespace UI
} // namespace ResticGUI

#endif // SNAPSHOTPAGE_H
