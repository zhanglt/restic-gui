#ifndef RESTOREWIZARD_H
#define RESTOREWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QTableWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QFutureWatcher>
#include "../../models/Snapshot.h"
#include "../../models/FileInfo.h"
#include "../../models/RestoreOptions.h"

namespace ResticGUI {
namespace UI {

/**
 * @brief 数据恢复向导
 */
class RestoreWizard : public QWizard
{
    Q_OBJECT

public:
    explicit RestoreWizard(QWidget* parent = nullptr);
    ~RestoreWizard();

    // 设置预设的恢复路径（在打开向导前设置）
    void setPresetTargetPath(const QString& path);
    QString getPresetTargetPath() const { return m_presetTargetPath; }

    // 获取恢复配置
    int getRepositoryId() const;
    QString getSnapshotId() const;
    QStringList getSelectedPaths() const;
    Models::RestoreOptions getRestoreOptions() const;

    // 页面枚举（public以便子类访问）
    enum {
        Page_SnapshotSelection,   // 步骤1：选择快照
        Page_FileSelection,       // 步骤2：选择文件
        Page_RestoreOptions,      // 步骤3：恢复选项
        Page_Confirm              // 步骤4：确认
    };

protected:
    void accept() override;

private:

    // 保存的配置
    mutable int m_repositoryId;
    mutable QString m_snapshotId;
    mutable QStringList m_selectedPaths;
    mutable Models::RestoreOptions m_options;

    // 预设的恢复路径
    QString m_presetTargetPath;
};

/**
 * @brief 步骤1：快照选择页面
 */
class SnapshotSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SnapshotSelectionPage(QWidget* parent = nullptr);
    ~SnapshotSelectionPage();

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;

private slots:
    void onRepositoryChanged(int index);
    void onRefresh();
    void onSnapshotsLoaded();
    void onSnapshotSelected(int currentRow, int previousRow = -1);

private:
    void loadRepositories();
    void loadSnapshots();
    void displaySnapshots(const QList<Models::Snapshot>& snapshots);

    QComboBox* m_repositoryComboBox;
    QPushButton* m_refreshButton;
    QTableWidget* m_snapshotTable;
    QLabel* m_infoLabel;

    int m_currentRepositoryId;
    bool m_isLoading;
    QFutureWatcher<QList<Models::Snapshot>>* m_snapshotWatcher;
};

/**
 * @brief 步骤2：文件选择页面
 */
class FileSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit FileSelectionPage(QWidget* parent = nullptr);
    ~FileSelectionPage();

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;

private slots:
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onSearchTextChanged(const QString& text);
    void onFilesLoaded();
    void onSelectAll();
    void onSelectNone();
    void onExpandAll();
    void onCollapseAll();

private:
    void loadRootFiles();
    void loadDirectoryFiles(QTreeWidgetItem* item, const QString& path);
    void addFileItems(QTreeWidgetItem* parent, const QList<Models::FileInfo>& files);
    void updateSelectionStats();
    void setItemChecked(QTreeWidgetItem* item, bool checked, bool updateChildren = true);
    QIcon getFileIcon(const Models::FileInfo& fileInfo);
    QString formatFileSize(qint64 size);

    QLineEdit* m_searchEdit;
    QTreeWidget* m_treeWidget;
    QLabel* m_statusLabel;
    QLabel* m_selectionLabel;
    QPushButton* m_selectAllButton;
    QPushButton* m_selectNoneButton;
    QPushButton* m_expandAllButton;
    QPushButton* m_collapseAllButton;

    int m_repoId;
    QString m_snapshotId;
    bool m_isLoading;
    QFutureWatcher<QList<Models::FileInfo>>* m_fileWatcher;
    QTreeWidgetItem* m_currentLoadingItem;
};

/**
 * @brief 步骤3：恢复选项页面
 */
class RestoreOptionsPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit RestoreOptionsPage(QWidget* parent = nullptr);

    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private slots:
    void onBrowse();
    void onIncludeCheckBoxToggled(bool checked);

private:
    QLineEdit* m_targetPathEdit;
    QPushButton* m_browseButton;
    QCheckBox* m_includeCheckBox;
    QLineEdit* m_includeEdit;
    QCheckBox* m_restorePermissionsCheckBox;
    QCheckBox* m_restoreTimestampsCheckBox;
    QCheckBox* m_verifyCheckBox;
};

/**
 * @brief 步骤4：确认页面
 */
class RestoreConfirmPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit RestoreConfirmPage(QWidget* parent = nullptr);

    void initializePage() override;
    int nextId() const override;

private:
    QTextEdit* m_summaryText;
};

} // namespace UI
} // namespace ResticGUI

#endif // RESTOREWIZARD_H
