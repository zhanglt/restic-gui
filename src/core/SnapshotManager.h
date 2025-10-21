#ifndef SNAPSHOTMANAGER_H
#define SNAPSHOTMANAGER_H

#include <QObject>
#include <QMutex>
#include "../models/Snapshot.h"
#include "../models/FileInfo.h"

namespace ResticGUI {
namespace Core {

class SnapshotManager : public QObject
{
    Q_OBJECT

public:
    static SnapshotManager* instance();
    void initialize();

    // 快照操作
    QList<Models::Snapshot> listSnapshots(int repoId, bool forceRefresh = false);
    Models::Snapshot getSnapshot(int repoId, const QString& snapshotId);
    bool deleteSnapshots(int repoId, const QStringList& snapshotIds);

    // 文件浏览
    QList<Models::FileInfo> listFiles(int repoId, const QString& snapshotId, const QString& path = QString());

    // 快照比较
    QList<Models::FileInfo> compareSnapshots(int repoId, const QString& snapshot1, const QString& snapshot2);

signals:
    void snapshotsUpdated(int repoId);
    void snapshotDeleted(const QString& snapshotId);

private:
    explicit SnapshotManager(QObject* parent = nullptr);
    ~SnapshotManager();
    SnapshotManager(const SnapshotManager&) = delete;
    SnapshotManager& operator=(const SnapshotManager&) = delete;

    static SnapshotManager* s_instance;
    static QMutex s_instanceMutex;
    mutable QMutex m_mutex;
};

} // namespace Core
} // namespace ResticGUI

#endif // SNAPSHOTMANAGER_H
