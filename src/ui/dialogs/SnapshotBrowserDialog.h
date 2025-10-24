#ifndef SNAPSHOTBROWSERDIALOG_H
#define SNAPSHOTBROWSERDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFutureWatcher>
#include "../../models/FileInfo.h"

namespace ResticGUI {
namespace UI {

/**
 * @brief 快照文件浏览对话框
 *
 * 显示快照中的文件树结构，支持浏览目录、选择文件和查看文件信息
 */
class SnapshotBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SnapshotBrowserDialog(int repoId, const QString& snapshotId,
                                   const QString& snapshotName, QWidget* parent = nullptr);
    ~SnapshotBrowserDialog();

    // 获取用户选择的文件路径列表
    QStringList getSelectedPaths() const;

private slots:
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onSearchTextChanged(const QString& text);
    void onFilesLoaded();
    void onSearchTimerTimeout();
    void onSelectAll();
    void onSelectNone();
    void onExpandAll();
    void onCollapseAll();
    void onConfirm();

private:
    void setupUI();
    void loadRootFiles();
    void loadDirectoryFiles(QTreeWidgetItem* item, const QString& path);
    void addFileItems(QTreeWidgetItem* parent, const QList<Models::FileInfo>& files);
    QIcon getFileIcon(const Models::FileInfo& fileInfo);
    QString formatFileSize(qint64 size);
    bool filterTreeItem(QTreeWidgetItem* item, const QString& searchText);
    void expandAllUnloadedDirectories(QTreeWidgetItem* item = nullptr);
    void performSearch(const QString& searchText);
    void updateSelectionStats();
    void setItemChecked(QTreeWidgetItem* item, bool checked, bool updateChildren = true);
    void collectSelectedPaths(QTreeWidgetItem* item, QStringList& paths) const;

    int m_repoId;
    QString m_snapshotId;
    QString m_snapshotName;

    QTreeWidget* m_treeWidget;
    QLineEdit* m_searchEdit;
    QLabel* m_statusLabel;
    QLabel* m_selectionLabel;
    QPushButton* m_selectAllButton;
    QPushButton* m_selectNoneButton;
    QPushButton* m_expandAllButton;
    QPushButton* m_collapseAllButton;
    QPushButton* m_cancelButton;
    QPushButton* m_confirmButton;

    bool m_isLoading;
    QFutureWatcher<QList<Models::FileInfo>>* m_fileWatcher;
    QTreeWidgetItem* m_currentLoadingItem;

    QString m_pendingSearchText;
    QTimer* m_searchTimer;
    bool m_isSearching;

    QStringList m_selectedPaths;
};

} // namespace UI
} // namespace ResticGUI

#endif // SNAPSHOTBROWSERDIALOG_H
