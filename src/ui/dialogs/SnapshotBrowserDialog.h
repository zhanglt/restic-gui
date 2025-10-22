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
 * 显示快照中的文件树结构，支持浏览目录和查看文件信息
 */
class SnapshotBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SnapshotBrowserDialog(int repoId, const QString& snapshotId,
                                   const QString& snapshotName, QWidget* parent = nullptr);
    ~SnapshotBrowserDialog();

private slots:
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onSearchTextChanged(const QString& text);
    void onFilesLoaded();

private:
    void setupUI();
    void loadRootFiles();
    void loadDirectoryFiles(QTreeWidgetItem* item, const QString& path);
    void addFileItems(QTreeWidgetItem* parent, const QList<Models::FileInfo>& files);
    QIcon getFileIcon(const Models::FileInfo& fileInfo);
    QString formatFileSize(qint64 size);

    int m_repoId;
    QString m_snapshotId;
    QString m_snapshotName;

    QTreeWidget* m_treeWidget;
    QLineEdit* m_searchEdit;
    QLabel* m_statusLabel;
    QPushButton* m_closeButton;

    bool m_isLoading;
    QFutureWatcher<QList<Models::FileInfo>>* m_fileWatcher;
    QTreeWidgetItem* m_currentLoadingItem;
};

} // namespace UI
} // namespace ResticGUI

#endif // SNAPSHOTBROWSERDIALOG_H
