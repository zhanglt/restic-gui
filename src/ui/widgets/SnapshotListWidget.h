#ifndef SNAPSHOTLISTWIDGET_H
#define SNAPSHOTLISTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include "../../models/Snapshot.h"

namespace ResticGUI {
namespace UI {

/**
 * @brief 快照列表控件
 */
class SnapshotListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SnapshotListWidget(QWidget* parent = nullptr);
    ~SnapshotListWidget();

    void setSnapshots(const QList<Models::Snapshot>& snapshots);
    QList<Models::Snapshot> getSelectedSnapshots() const;
    void refresh();

signals:
    void snapshotSelected(const Models::Snapshot& snapshot);
    void snapshotDoubleClicked(const Models::Snapshot& snapshot);

private slots:
    void onSelectionChanged();
    void onItemDoubleClicked(int row, int column);

private:
    void setupUI();

    QTableWidget* m_tableWidget;
    QList<Models::Snapshot> m_snapshots;
};

} // namespace UI
} // namespace ResticGUI

#endif // SNAPSHOTLISTWIDGET_H
