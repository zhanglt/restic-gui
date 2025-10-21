#ifndef FILETREEWIDGET_H
#define FILETREEWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include "../../models/FileInfo.h"

namespace ResticGUI {
namespace UI {

/**
 * @brief 文件树控件
 *
 * 用于浏览快照中的文件结构
 */
class FileTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreeWidget(QWidget* parent = nullptr);
    ~FileTreeWidget();

    void setFiles(const QList<Models::FileInfo>& files);
    QList<Models::FileInfo> getSelectedFiles() const;
    void clear();

signals:
    void fileSelected(const Models::FileInfo& file);
    void directoryExpanded(const QString& path);

private slots:
    void onItemExpanded(QTreeWidgetItem* item);
    void onSelectionChanged();

private:
    void setupUI();
    QTreeWidgetItem* createTreeItem(const Models::FileInfo& file);
    QString formatSize(qint64 size);

    QTreeWidget* m_treeWidget;
    QList<Models::FileInfo> m_files;
};

} // namespace UI
} // namespace ResticGUI

#endif // FILETREEWIDGET_H
