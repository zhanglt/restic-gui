/**
 * @file FileTreeWidget.cpp
 * @brief 文件树控件实现
 */

#include "FileTreeWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace ResticGUI {
namespace UI {

FileTreeWidget::FileTreeWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

FileTreeWidget::~FileTreeWidget()
{
}

void FileTreeWidget::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("名称"), tr("大小"), tr("修改时间")});
    m_treeWidget->setAlternatingRowColors(true);

    connect(m_treeWidget, &QTreeWidget::itemExpanded,
            this, &FileTreeWidget::onItemExpanded);
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged,
            this, &FileTreeWidget::onSelectionChanged);

    layout->addWidget(m_treeWidget);
}

void FileTreeWidget::setFiles(const QList<Models::FileInfo>& files)
{
    m_files = files;
    m_treeWidget->clear();

    for (const Models::FileInfo& file : files) {
        QTreeWidgetItem* item = createTreeItem(file);
        m_treeWidget->addTopLevelItem(item);
    }
}

QList<Models::FileInfo> FileTreeWidget::getSelectedFiles() const
{
    QList<Models::FileInfo> selected;

    foreach (QTreeWidgetItem* item, m_treeWidget->selectedItems()) {
        QString path = item->data(0, Qt::UserRole).toString();
        for (const Models::FileInfo& file : m_files) {
            if (file.path == path) {
                selected.append(file);
                break;
            }
        }
    }

    return selected;
}

void FileTreeWidget::clear()
{
    m_treeWidget->clear();
    m_files.clear();
}

QTreeWidgetItem* FileTreeWidget::createTreeItem(const Models::FileInfo& file)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();

    item->setText(0, file.name);
    item->setData(0, Qt::UserRole, file.path);

    if (file.type == Models::FileType::Directory) {
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        item->setText(1, tr("<DIR>"));
        // 添加一个占位子项，以便显示展开箭头
        item->addChild(new QTreeWidgetItem());
    } else {
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        item->setText(1, formatSize(file.size));
    }

    item->setText(2, file.mtime.toString("yyyy-MM-dd hh:mm"));

    return item;
}

QString FileTreeWidget::formatSize(qint64 size)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (size >= GB) {
        return QString::number(size / (double)GB, 'f', 2) + " GB";
    } else if (size >= MB) {
        return QString::number(size / (double)MB, 'f', 2) + " MB";
    } else if (size >= KB) {
        return QString::number(size / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(size) + " B";
    }
}

void FileTreeWidget::onItemExpanded(QTreeWidgetItem* item)
{
    // 移除占位子项
    if (item->childCount() == 1 && item->child(0)->text(0).isEmpty()) {
        delete item->takeChild(0);

        // 加载子目录内容
        QString path = item->data(0, Qt::UserRole).toString();
        emit directoryExpanded(path);
    }
}

void FileTreeWidget::onSelectionChanged()
{
    QList<Models::FileInfo> selected = getSelectedFiles();
    if (!selected.isEmpty()) {
        emit fileSelected(selected.first());
    }
}

} // namespace UI
} // namespace ResticGUI
