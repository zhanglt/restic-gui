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
    void onRefresh();
    void onBrowse();
    void onRestore();
    void onQuickRestore();
    void onSnapshotsUpdated(int repoId);
    void onSnapshotsLoaded();
    void onSnapshotSelected(int currentRow, int previousRow = -1);
    void onIncludeCheckBoxToggled(bool checked);
    void onTargetPathChanged();
    void onSearch();
    void onFilterTextChanged(const QString& text);

private:
    void displaySnapshots(const QList<Models::Snapshot>& snapshots);
    void showLoadingIndicator(bool show);
    void updateQuickRestoreButtonState();
    void filterSnapshots(const QString& filterText);

    Ui::RestorePage* ui;
    int m_currentRepositoryId;
    bool m_firstShow;
    bool m_isLoading;
    QFutureWatcher<QList<Models::Snapshot>>* m_snapshotWatcher;
    QList<Models::Snapshot> m_allSnapshots;  // 保存所有快照，用于筛选
};

} // namespace UI
} // namespace ResticGUI

#endif // RESTOREPAGE_H
