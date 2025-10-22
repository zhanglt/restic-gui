#include "SnapshotBrowserDialog.h"
#include "../../core/SnapshotManager.h"
#include "../../utils/Logger.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QtConcurrent>
#include <QApplication>
#include <QStyle>
#include <QSet>

namespace ResticGUI {
namespace UI {

SnapshotBrowserDialog::SnapshotBrowserDialog(int repoId, const QString& snapshotId,
                                             const QString& snapshotName, QWidget* parent)
    : QDialog(parent)
    , m_repoId(repoId)
    , m_snapshotId(snapshotId)
    , m_snapshotName(snapshotName)
    , m_treeWidget(nullptr)
    , m_searchEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_closeButton(nullptr)
    , m_isLoading(false)
    , m_fileWatcher(nullptr)
    , m_currentLoadingItem(nullptr)
{
    setupUI();

    // 使用 QTimer 延迟加载，确保对话框完全显示后再开始加载
    QTimer::singleShot(100, this, &SnapshotBrowserDialog::loadRootFiles);
}

SnapshotBrowserDialog::~SnapshotBrowserDialog()
{
}

void SnapshotBrowserDialog::setupUI()
{
    setWindowTitle(tr("浏览快照: %1").arg(m_snapshotName));
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 顶部信息栏
    QLabel* infoLabel = new QLabel(
        tr("快照ID: %1\n双击目录展开，双击文件查看详情").arg(m_snapshotId.left(8)),
        this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // 搜索框
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel(tr("搜索:"), this);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("输入文件名或路径..."));
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchEdit);
    mainLayout->addLayout(searchLayout);

    // 文件树
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels(QStringList() << tr("名称") << tr("大小") << tr("类型") << tr("修改时间"));
    m_treeWidget->setColumnWidth(0, 300);
    m_treeWidget->setColumnWidth(1, 100);
    m_treeWidget->setColumnWidth(2, 80);
    m_treeWidget->setColumnWidth(3, 150);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSortingEnabled(false);
    mainLayout->addWidget(m_treeWidget);

    // 状态栏
    m_statusLabel = new QLabel(tr("准备加载..."), this);
    mainLayout->addWidget(m_statusLabel);

    // 底部按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_closeButton = new QPushButton(tr("关闭"), this);
    buttonLayout->addWidget(m_closeButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, &SnapshotBrowserDialog::onItemExpanded);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &SnapshotBrowserDialog::onItemDoubleClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SnapshotBrowserDialog::onSearchTextChanged);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // 初始化异步加载器
    m_fileWatcher = new QFutureWatcher<QList<Models::FileInfo>>(this);

    // 使用新式信号槽语法确保类型安全
    bool connected = connect(m_fileWatcher, &QFutureWatcher<QList<Models::FileInfo>>::finished,
            this, &SnapshotBrowserDialog::onFilesLoaded, Qt::QueuedConnection);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("setupUI: QFutureWatcher finished 信号连接%1 (连接类型: Qt::QueuedConnection)")
            .arg(connected ? "成功" : "失败"));

    // 添加备用的进度信号连接用于诊断
    connect(m_fileWatcher, &QFutureWatcher<QList<Models::FileInfo>>::started,
            this, [this]() {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "QFutureWatcher: started 信号触发");
    });

    connect(m_fileWatcher, &QFutureWatcher<QList<Models::FileInfo>>::canceled,
            this, [this]() {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "QFutureWatcher: canceled 信号触发");
    });
}

void SnapshotBrowserDialog::loadRootFiles()
{
    Utils::Logger::instance()->log(Utils::Logger::Debug, "loadRootFiles: 开始加载根目录");

    if (m_isLoading) {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "loadRootFiles: 已经在加载中，忽略");
        return;
    }

    m_isLoading = true;
    m_currentLoadingItem = nullptr;
    m_statusLabel->setText(tr("正在加载快照根目录..."));
    m_treeWidget->clear();

    int repoId = m_repoId;
    QString snapshotId = m_snapshotId;

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("loadRootFiles: 启动异步任务, repoId=%1, snapshotId=%2")
            .arg(repoId).arg(snapshotId.left(8)));

    QFuture<QList<Models::FileInfo>> future = QtConcurrent::run([repoId, snapshotId]() -> QList<Models::FileInfo> {
        try {
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                "loadRootFiles Lambda: 开始执行异步任务");
            Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
            QList<Models::FileInfo> result = snapshotMgr->listFiles(repoId, snapshotId, QString());
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("loadRootFiles Lambda: 异步任务完成，返回 %1 个文件").arg(result.size()));
            return result;
        } catch (const std::exception& e) {
            Utils::Logger::instance()->log(Utils::Logger::Error,
                QString("loadRootFiles Lambda: 捕获异常: %1").arg(e.what()));
            return QList<Models::FileInfo>();
        } catch (...) {
            Utils::Logger::instance()->log(Utils::Logger::Error,
                "loadRootFiles Lambda: 捕获未知异常");
            return QList<Models::FileInfo>();
        }
    });

    m_fileWatcher->setFuture(future);
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("loadRootFiles: QFuture已设置到watcher, isFinished=%1, isRunning=%2")
            .arg(future.isFinished()).arg(future.isRunning()));
}

void SnapshotBrowserDialog::loadDirectoryFiles(QTreeWidgetItem* item, const QString& path)
{
    if (m_isLoading) {
        return;
    }

    m_isLoading = true;
    m_currentLoadingItem = item;
    m_statusLabel->setText(tr("正在加载目录: %1").arg(path));

    int repoId = m_repoId;
    QString snapshotId = m_snapshotId;

    QFuture<QList<Models::FileInfo>> future = QtConcurrent::run([repoId, snapshotId, path]() -> QList<Models::FileInfo> {
        try {
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("loadDirectoryFiles Lambda: 开始加载目录: %1").arg(path));
            Core::SnapshotManager* snapshotMgr = Core::SnapshotManager::instance();
            QList<Models::FileInfo> result = snapshotMgr->listFiles(repoId, snapshotId, path);
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("loadDirectoryFiles Lambda: 完成，返回 %1 个文件").arg(result.size()));
            return result;
        } catch (const std::exception& e) {
            Utils::Logger::instance()->log(Utils::Logger::Error,
                QString("loadDirectoryFiles Lambda: 捕获异常: %1").arg(e.what()));
            return QList<Models::FileInfo>();
        } catch (...) {
            Utils::Logger::instance()->log(Utils::Logger::Error,
                "loadDirectoryFiles Lambda: 捕获未知异常");
            return QList<Models::FileInfo>();
        }
    });

    m_fileWatcher->setFuture(future);
}

void SnapshotBrowserDialog::onFilesLoaded()
{
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        "onFilesLoaded: ===== 回调函数被触发 =====");

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("onFilesLoaded: QFuture状态 - isFinished=%1, isRunning=%2")
            .arg(m_fileWatcher->isFinished()).arg(m_fileWatcher->isRunning()));

    m_isLoading = false;

    QList<Models::FileInfo> files;
    try {
        files = m_fileWatcher->result();
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("onFilesLoaded: 成功获取结果，收到 %1 个文件").arg(files.size()));
    } catch (const std::exception& e) {
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("onFilesLoaded: 获取结果时捕获异常: %1").arg(e.what()));
        m_statusLabel->setText(tr("加载失败: %1").arg(e.what()));
        return;
    } catch (...) {
        Utils::Logger::instance()->log(Utils::Logger::Error,
            "onFilesLoaded: 获取结果时捕获未知异常");
        m_statusLabel->setText(tr("加载失败: 未知错误"));
        return;
    }

    if (m_currentLoadingItem == nullptr) {
        // 加载根目录
        Utils::Logger::instance()->log(Utils::Logger::Debug, "onFilesLoaded: 准备加载根目录文件");
        addFileItems(nullptr, files);
        m_statusLabel->setText(tr("已加载 %1 个文件/目录").arg(files.size()));
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("onFilesLoaded: 根目录加载完成，树控件有 %1 个顶级项").arg(m_treeWidget->topLevelItemCount()));
    } else {
        // 加载子目录
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("onFilesLoaded: 准备加载子目录: %1").arg(m_currentLoadingItem->text(0)));
        // 清除占位符
        m_currentLoadingItem->takeChildren();
        addFileItems(m_currentLoadingItem, files);
        m_statusLabel->setText(tr("已加载 %1 个文件/目录").arg(files.size()));
    }
}

void SnapshotBrowserDialog::addFileItems(QTreeWidgetItem* parent, const QList<Models::FileInfo>& files)
{
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("addFileItems: 开始添加 %1 个文件项").arg(files.size()));

    // 确定当前目录的路径
    QString currentPath;
    if (parent != nullptr) {
        currentPath = parent->data(0, Qt::UserRole).toString();
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("addFileItems: 当前路径=%1").arg(currentPath));
    }

    // 用于去重的集合（因为 restic ls 可能返回重复项）
    QSet<QString> addedPaths;

    int addedCount = 0;
    for (const Models::FileInfo& fileInfo : files) {
        // 跳过空名称的项
        if (fileInfo.name.isEmpty()) {
            Utils::Logger::instance()->log(Utils::Logger::Debug, "跳过空名称的文件项");
            continue;
        }

        // 过滤：只添加当前目录的直接子项
        if (parent == nullptr) {
            // 根目录：只添加第一级项（路径不包含多个斜杠）
            // 例如：接受 "/C"，拒绝 "/C/tmp"
            QString path = fileInfo.path;
            if (path.startsWith("/")) {
                path = path.mid(1); // 去掉开头的 /
            }
            if (path.contains('/')) {
                // 包含斜杠，说明不是第一级
                continue;
            }
        } else {
            // 子目录：只添加该目录的直接子项
            // 例如：当前路径=/C，接受 /C/tmp，拒绝 /C/tmp/4
            if (!fileInfo.path.startsWith(currentPath + "/")) {
                // 不是当前目录的子项
                continue;
            }

            // 检查是否是直接子项
            QString relativePath = fileInfo.path.mid(currentPath.length() + 1); // +1 去掉 /
            if (relativePath.contains('/')) {
                // 包含斜杠，说明不是直接子项
                continue;
            }
        }

        // 去重检查
        if (addedPaths.contains(fileInfo.path)) {
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("跳过重复项: %1").arg(fileInfo.path));
            continue;
        }
        addedPaths.insert(fileInfo.path);

        QTreeWidgetItem* item = new QTreeWidgetItem();

        // 名称
        item->setText(0, fileInfo.name);
        item->setIcon(0, getFileIcon(fileInfo));

        // 大小
        if (fileInfo.type == Models::FileType::File) {
            item->setText(1, formatFileSize(fileInfo.size));
        } else {
            item->setText(1, "-");
        }

        // 类型
        if (fileInfo.type == Models::FileType::Directory) {
            item->setText(2, tr("文件夹"));
            // 为目录添加占位符子项，使其可展开
            QTreeWidgetItem* placeholder = new QTreeWidgetItem();
            placeholder->setText(0, tr("加载中..."));
            item->addChild(placeholder);
        } else if (fileInfo.type == Models::FileType::Symlink) {
            item->setText(2, tr("符号链接"));
        } else {
            item->setText(2, tr("文件"));
        }

        // 修改时间
        if (fileInfo.mtime.isValid()) {
            item->setText(3, fileInfo.mtime.toString("yyyy-MM-dd HH:mm:ss"));
        }

        // 存储完整路径和类型（转换为int存储）
        item->setData(0, Qt::UserRole, fileInfo.path);
        item->setData(0, Qt::UserRole + 1, static_cast<int>(fileInfo.type));

        if (parent == nullptr) {
            m_treeWidget->addTopLevelItem(item);
        } else {
            parent->addChild(item);
        }

        addedCount++;
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("添加项: name=%1, path=%2").arg(fileInfo.name).arg(fileInfo.path));
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("addFileItems: 实际添加了 %1 个项（从 %2 个候选项中过滤）")
            .arg(addedCount).arg(files.size()));
}

void SnapshotBrowserDialog::onItemExpanded(QTreeWidgetItem* item)
{
    // 检查是否需要加载子项
    if (item->childCount() == 1 && item->child(0)->text(0) == tr("加载中...")) {
        QString path = item->data(0, Qt::UserRole).toString();
        int typeInt = item->data(0, Qt::UserRole + 1).toInt();
        Models::FileType type = static_cast<Models::FileType>(typeInt);

        if (type == Models::FileType::Directory) {
            loadDirectoryFiles(item, path);
        }
    }
}

void SnapshotBrowserDialog::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);

    QString path = item->data(0, Qt::UserRole).toString();
    int typeInt = item->data(0, Qt::UserRole + 1).toInt();
    Models::FileType type = static_cast<Models::FileType>(typeInt);
    QString name = item->text(0);
    QString size = item->text(1);
    QString mtime = item->text(3);

    if (type == Models::FileType::File) {
        // 显示文件详情
        QString details = tr("文件信息\n\n")
            + tr("名称: %1\n").arg(name)
            + tr("路径: %1\n").arg(path)
            + tr("大小: %1\n").arg(size)
            + tr("修改时间: %1").arg(mtime);

        QMessageBox::information(this, tr("文件详情"), details);
    }
}

void SnapshotBrowserDialog::onSearchTextChanged(const QString& text)
{
    // 简单的搜索过滤实现
    if (text.isEmpty()) {
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            m_treeWidget->topLevelItem(i)->setHidden(false);
        }
        return;
    }

    // 隐藏不匹配的项
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        QString itemText = item->text(0);
        bool match = itemText.contains(text, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}

QIcon SnapshotBrowserDialog::getFileIcon(const Models::FileInfo& fileInfo)
{
    // 使用系统标准图标
    QStyle* style = QApplication::style();

    if (fileInfo.type == Models::FileType::Directory) {
        return style->standardIcon(QStyle::SP_DirIcon);
    } else if (fileInfo.type == Models::FileType::Symlink) {
        return style->standardIcon(QStyle::SP_FileLinkIcon);
    } else {
        return style->standardIcon(QStyle::SP_FileIcon);
    }
}

QString SnapshotBrowserDialog::formatFileSize(qint64 size)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;

    if (size < KB) {
        return QString::number(size) + " B";
    } else if (size < MB) {
        return QString::number(size / KB) + " KB";
    } else if (size < GB) {
        return QString::number(size / MB) + " MB";
    } else if (size < TB) {
        return QString::number(size / GB) + " GB";
    } else {
        return QString::number(size / TB) + " TB";
    }
}

} // namespace UI
} // namespace ResticGUI
