#ifndef RESTOREPAGE_H
#define RESTOREPAGE_H

#include <QWidget>
#include <QFutureWatcher>
#include "../../models/Snapshot.h"

namespace Ui {
class RestorePage;
}

namespace ResticGUI {
namespace UI {

class RestorePage : public QWidget
{
    Q_OBJECT

public:
    explicit RestorePage(QWidget* parent = nullptr);
    ~RestorePage();

public slots:
    void loadRepositories();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onRepositoryChanged(int index);
    void loadSnapshots();
    void onBrowse();
    void onRestore();
    void onSnapshotsUpdated(int repoId);
    void onSnapshotsLoaded();

private:
    void displaySnapshots(const QList<Models::Snapshot>& snapshots);
    void showLoadingIndicator(bool show);

    Ui::RestorePage* ui;
    int m_currentRepositoryId;
    bool m_firstShow;
    bool m_isLoading;
    QFutureWatcher<QList<Models::Snapshot>>* m_snapshotWatcher;
};

} // namespace UI
} // namespace ResticGUI

#endif // RESTOREPAGE_H
