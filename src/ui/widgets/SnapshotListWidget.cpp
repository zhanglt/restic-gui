/**
 * @file SnapshotListWidget.cpp
 * @brief 快照列表控件实现
 */

#include "SnapshotListWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace ResticGUI {
namespace UI {

SnapshotListWidget::SnapshotListWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

SnapshotListWidget::~SnapshotListWidget()
{
}

void SnapshotListWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels({
        tr("快照ID"),
        tr("时间"),
        tr("主机"),
        tr("路径"),
        tr("标签")
    });

    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setAlternatingRowColors(true);

    connect(m_tableWidget, &QTableWidget::itemSelectionChanged,
            this, &SnapshotListWidget::onSelectionChanged);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked,
            this, &SnapshotListWidget::onItemDoubleClicked);

    layout->addWidget(m_tableWidget);
}

void SnapshotListWidget::setSnapshots(const QList<Models::Snapshot>& snapshots)
{
    m_snapshots = snapshots;

    m_tableWidget->setRowCount(snapshots.size());

    for (int i = 0; i < snapshots.size(); ++i) {
        const Models::Snapshot& snapshot = snapshots[i];

        m_tableWidget->setItem(i, 0, new QTableWidgetItem(snapshot.id.left(8)));
        m_tableWidget->setItem(i, 1, new QTableWidgetItem(
            snapshot.time.toString("yyyy-MM-dd hh:mm:ss")));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(snapshot.hostname));
        m_tableWidget->setItem(i, 3, new QTableWidgetItem(
            snapshot.paths.join(", ")));
        m_tableWidget->setItem(i, 4, new QTableWidgetItem(
            snapshot.tags.join(", ")));
    }
}

QList<Models::Snapshot> SnapshotListWidget::getSelectedSnapshots() const
{
    QList<Models::Snapshot> selected;
    QList<int> selectedRows;

    foreach (QTableWidgetItem* item, m_tableWidget->selectedItems()) {
        int row = item->row();
        if (!selectedRows.contains(row)) {
            selectedRows.append(row);
        }
    }

    foreach (int row, selectedRows) {
        if (row < m_snapshots.size()) {
            selected.append(m_snapshots[row]);
        }
    }

    return selected;
}

void SnapshotListWidget::refresh()
{
    // TODO: 触发刷新快照列表
}

void SnapshotListWidget::onSelectionChanged()
{
    QList<Models::Snapshot> selected = getSelectedSnapshots();
    if (!selected.isEmpty()) {
        emit snapshotSelected(selected.first());
    }
}

void SnapshotListWidget::onItemDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    if (row < m_snapshots.size()) {
        emit snapshotDoubleClicked(m_snapshots[row]);
    }
}

} // namespace UI
} // namespace ResticGUI
