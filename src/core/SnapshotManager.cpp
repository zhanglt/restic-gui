#include "SnapshotManager.h"
#include "ResticWrapper.h"
#include "RepositoryManager.h"
#include "../data/CacheManager.h"
#include "../data/PasswordManager.h"
#include "../utils/Logger.h"
#include <QMutexLocker>

namespace ResticGUI {
namespace Core {

SnapshotManager* SnapshotManager::s_instance = nullptr;
QMutex SnapshotManager::s_instanceMutex;

SnapshotManager* SnapshotManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new SnapshotManager();
        }
    }
    return s_instance;
}

SnapshotManager::SnapshotManager(QObject* parent) : QObject(parent) {}
SnapshotManager::~SnapshotManager() {}

void SnapshotManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "快照管理器初始化完成");
}

QList<Models::Snapshot> SnapshotManager::listSnapshots(int repoId, bool forceRefresh)
{
    QMutexLocker locker(&m_mutex);

    QList<Models::Snapshot> snapshots;
    Data::CacheManager* cache = Data::CacheManager::instance();

    // 检查缓存
    if (!forceRefresh && cache->isSnapshotCacheValid(repoId, 5)) {
        if (cache->getCachedSnapshots(repoId, snapshots)) {
            return snapshots;
        }
    }

    // 从restic获取
    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    QString password;
    if (!Data::PasswordManager::instance()->getPassword(repoId, password)) {
        return snapshots;
    }

    ResticWrapper wrapper;
    if (wrapper.listSnapshots(repo, password, snapshots)) {
        cache->cacheSnapshots(repoId, snapshots);
        emit snapshotsUpdated(repoId);
    }

    return snapshots;
}

Models::Snapshot SnapshotManager::getSnapshot(int repoId, const QString& snapshotId)
{
    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    QString password;
    Models::Snapshot snapshot;

    if (Data::PasswordManager::instance()->getPassword(repoId, password)) {
        ResticWrapper wrapper;
        wrapper.getSnapshotInfo(repo, password, snapshotId, snapshot);
    }

    return snapshot;
}

bool SnapshotManager::deleteSnapshots(int repoId, const QStringList& snapshotIds)
{
    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    QString password;

    if (!Data::PasswordManager::instance()->getPassword(repoId, password)) {
        return false;
    }

    ResticWrapper wrapper;
    bool success = wrapper.deleteSnapshots(repo, password, snapshotIds);

    if (success) {
        Data::CacheManager::instance()->clearSnapshotCache(repoId);
        for (const QString& id : snapshotIds) {
            emit snapshotDeleted(id);
        }
        emit snapshotsUpdated(repoId);
    }

    return success;
}

QList<Models::FileInfo> SnapshotManager::listFiles(int repoId, const QString& snapshotId, const QString& path)
{
    QList<Models::FileInfo> files;

    // 检查缓存
    Data::CacheManager* cache = Data::CacheManager::instance();
    if (cache->getCachedFileTree(snapshotId, path, files)) {
        return files;
    }

    // 从restic获取
    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    QString password;

    if (Data::PasswordManager::instance()->getPassword(repoId, password)) {
        ResticWrapper wrapper;
        if (wrapper.listFiles(repo, password, snapshotId, path, files)) {
            cache->cacheFileTree(snapshotId, path, files);
        }
    }

    return files;
}

QList<Models::FileInfo> SnapshotManager::compareSnapshots(int repoId, const QString& snapshot1, const QString& snapshot2)
{
    // TODO: 实现快照比较功能
    Q_UNUSED(repoId);
    Q_UNUSED(snapshot1);
    Q_UNUSED(snapshot2);
    return QList<Models::FileInfo>();
}

} // namespace Core
} // namespace ResticGUI
