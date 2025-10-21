/**
 * @file CacheManager.cpp
 * @brief 缓存管理器实现
 */

#include "CacheManager.h"
#include "DatabaseManager.h"
#include "../utils/Logger.h"
#include <QMutexLocker>

namespace ResticGUI {
namespace Data {

CacheManager* CacheManager::s_instance = nullptr;
QMutex CacheManager::s_instanceMutex;

CacheManager* CacheManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new CacheManager();
        }
    }
    return s_instance;
}

CacheManager::CacheManager(QObject* parent)
    : QObject(parent)
    , m_maxCacheSizeMB(100)
{
}

CacheManager::~CacheManager()
{
    clearAllCache();
}

void CacheManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "缓存管理器初始化完成");
}

// ========== 快照缓存 ==========

void CacheManager::cacheSnapshots(int repoId, const QList<Models::Snapshot>& snapshots, bool persistToDb)
{
    QMutexLocker locker(&m_mutex);

    SnapshotCache cache;
    cache.snapshots = snapshots;
    cache.timestamp = QDateTime::currentDateTime();

    m_snapshotCache[repoId] = cache;

    // 持久化到数据库
    if (persistToDb) {
        DatabaseManager::instance()->cacheSnapshots(repoId, snapshots);
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("快照列表已缓存，仓库ID: %1, 数量: %2").arg(repoId).arg(snapshots.size()));

    emit cacheUpdated(repoId);
}

bool CacheManager::getCachedSnapshots(int repoId, QList<Models::Snapshot>& snapshots)
{
    QMutexLocker locker(&m_mutex);

    // 先检查内存缓存
    if (m_snapshotCache.contains(repoId)) {
        snapshots = m_snapshotCache[repoId].snapshots;
        return true;
    }

    // 尝试从数据库加载
    snapshots = DatabaseManager::instance()->getCachedSnapshots(repoId);
    if (!snapshots.isEmpty()) {
        // 加载到内存缓存
        SnapshotCache cache;
        cache.snapshots = snapshots;
        cache.timestamp = QDateTime::currentDateTime();
        m_snapshotCache[repoId] = cache;
        return true;
    }

    return false;
}

void CacheManager::clearSnapshotCache(int repoId)
{
    QMutexLocker locker(&m_mutex);

    m_snapshotCache.remove(repoId);
    DatabaseManager::instance()->clearSnapshotCache(repoId);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("快照缓存已清除，仓库ID: %1").arg(repoId));

    emit cacheCleared(repoId);
}

bool CacheManager::isSnapshotCacheValid(int repoId, int maxAgeMinutes)
{
    QMutexLocker locker(&m_mutex);

    if (!m_snapshotCache.contains(repoId)) {
        return false;
    }

    QDateTime now = QDateTime::currentDateTime();
    QDateTime cacheTime = m_snapshotCache[repoId].timestamp;

    qint64 ageSeconds = cacheTime.secsTo(now);
    return ageSeconds < (maxAgeMinutes * 60);
}

// ========== 文件树缓存 ==========

void CacheManager::cacheFileTree(const QString& snapshotId, const QString& path, const QList<Models::FileInfo>& files)
{
    QMutexLocker locker(&m_mutex);

    QString cacheKey = snapshotId + "_" + path;

    FileTreeCache cache;
    cache.files = files;
    cache.timestamp = QDateTime::currentDateTime();

    m_fileTreeCache[cacheKey] = cache;

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("文件树已缓存，快照: %1, 路径: %2, 文件数: %3")
        .arg(snapshotId).arg(path).arg(files.size()));

    checkCacheSizeLimit();
}

bool CacheManager::getCachedFileTree(const QString& snapshotId, const QString& path, QList<Models::FileInfo>& files)
{
    QMutexLocker locker(&m_mutex);

    QString cacheKey = snapshotId + "_" + path;

    if (m_fileTreeCache.contains(cacheKey)) {
        files = m_fileTreeCache[cacheKey].files;
        return true;
    }

    return false;
}

void CacheManager::clearFileTreeCache(const QString& snapshotId)
{
    QMutexLocker locker(&m_mutex);

    if (snapshotId.isEmpty()) {
        // 清除所有文件树缓存
        m_fileTreeCache.clear();
        Utils::Logger::instance()->log(Utils::Logger::Debug, "所有文件树缓存已清除");
    } else {
        // 清除指定快照的文件树缓存
        QStringList keysToRemove;
        for (const QString& key : m_fileTreeCache.keys()) {
            if (key.startsWith(snapshotId + "_")) {
                keysToRemove.append(key);
            }
        }

        for (const QString& key : keysToRemove) {
            m_fileTreeCache.remove(key);
        }

        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("文件树缓存已清除，快照ID: %1").arg(snapshotId));
    }
}

// ========== 仓库统计缓存 ==========

void CacheManager::cacheRepoStats(int repoId, const Models::RepoStats& stats)
{
    QMutexLocker locker(&m_mutex);

    RepoStatsCache cache;
    cache.stats = stats;
    cache.timestamp = QDateTime::currentDateTime();

    m_repoStatsCache[repoId] = cache;

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("仓库统计信息已缓存，仓库ID: %1").arg(repoId));
}

bool CacheManager::getCachedRepoStats(int repoId, Models::RepoStats& stats)
{
    QMutexLocker locker(&m_mutex);

    if (m_repoStatsCache.contains(repoId)) {
        stats = m_repoStatsCache[repoId].stats;
        return true;
    }

    return false;
}

void CacheManager::clearRepoStatsCache(int repoId)
{
    QMutexLocker locker(&m_mutex);

    m_repoStatsCache.remove(repoId);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("仓库统计缓存已清除，仓库ID: %1").arg(repoId));
}

bool CacheManager::isRepoStatsCacheValid(int repoId, int maxAgeMinutes)
{
    QMutexLocker locker(&m_mutex);

    if (!m_repoStatsCache.contains(repoId)) {
        return false;
    }

    QDateTime now = QDateTime::currentDateTime();
    QDateTime cacheTime = m_repoStatsCache[repoId].timestamp;

    qint64 ageSeconds = cacheTime.secsTo(now);
    return ageSeconds < (maxAgeMinutes * 60);
}

// ========== 通用缓存管理 ==========

void CacheManager::clearAllCache()
{
    QMutexLocker locker(&m_mutex);

    m_snapshotCache.clear();
    m_fileTreeCache.clear();
    m_repoStatsCache.clear();

    Utils::Logger::instance()->log(Utils::Logger::Info, "所有缓存已清除");
}

void CacheManager::clearRepositoryCache(int repoId)
{
    QMutexLocker locker(&m_mutex);

    // 清除快照缓存
    m_snapshotCache.remove(repoId);

    // 清除仓库统计缓存
    m_repoStatsCache.remove(repoId);

    // 文件树缓存不与仓库ID直接关联，不在此清除

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("仓库缓存已清除，仓库ID: %1").arg(repoId));

    emit cacheCleared(repoId);
}

qint64 CacheManager::getCacheSize() const
{
    QMutexLocker locker(&m_mutex);

    // 简化估算：每个快照约1KB，每个文件信息约512字节
    qint64 size = 0;

    // 快照缓存
    for (const SnapshotCache& cache : m_snapshotCache.values()) {
        size += cache.snapshots.size() * 1024;
    }

    // 文件树缓存
    for (const FileTreeCache& cache : m_fileTreeCache.values()) {
        size += cache.files.size() * 512;
    }

    // 仓库统计缓存（每个约4KB）
    size += m_repoStatsCache.size() * 4096;

    return size;
}

void CacheManager::setMaxCacheSize(int sizeMB)
{
    QMutexLocker locker(&m_mutex);
    m_maxCacheSizeMB = sizeMB;

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("最大缓存大小已设置为: %1 MB").arg(sizeMB));

    checkCacheSizeLimit();
}

// ========== 私有辅助函数 ==========

void CacheManager::cleanupExpiredCache()
{
    // 注意：调用此函数前应已锁定mutex

    QDateTime now = QDateTime::currentDateTime();

    // 清理超过1小时的文件树缓存
    QStringList expiredFileTreeKeys;
    for (auto it = m_fileTreeCache.begin(); it != m_fileTreeCache.end(); ++it) {
        qint64 ageSeconds = it.value().timestamp.secsTo(now);
        if (ageSeconds > 3600) { // 1小时
            expiredFileTreeKeys.append(it.key());
        }
    }

    for (const QString& key : expiredFileTreeKeys) {
        m_fileTreeCache.remove(key);
    }

    if (!expiredFileTreeKeys.isEmpty()) {
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("已清理 %1 个过期文件树缓存").arg(expiredFileTreeKeys.size()));
    }
}

void CacheManager::checkCacheSizeLimit()
{
    // 注意：调用此函数前应已锁定mutex

    qint64 currentSize = getCacheSize();
    qint64 maxSize = static_cast<qint64>(m_maxCacheSizeMB) * 1024 * 1024;

    if (currentSize > maxSize) {
        // 先清理过期缓存
        cleanupExpiredCache();

        // 如果还是超过限制，清理最老的文件树缓存
        currentSize = getCacheSize();
        if (currentSize > maxSize) {
            // 按时间戳排序，移除最老的缓存
            QList<QString> keys = m_fileTreeCache.keys();
            QList<QPair<QString, QDateTime>> keyTimes;

            for (const QString& key : keys) {
                keyTimes.append(qMakePair(key, m_fileTreeCache[key].timestamp));
            }

            // 按时间戳升序排序
            std::sort(keyTimes.begin(), keyTimes.end(),
                [](const QPair<QString, QDateTime>& a, const QPair<QString, QDateTime>& b) {
                    return a.second < b.second;
                });

            // 移除最老的缓存，直到大小符合限制
            int removed = 0;
            for (const auto& pair : keyTimes) {
                m_fileTreeCache.remove(pair.first);
                removed++;

                if (getCacheSize() <= maxSize) {
                    break;
                }
            }

            if (removed > 0) {
                Utils::Logger::instance()->log(Utils::Logger::Debug,
                    QString("已清理 %1 个文件树缓存以满足大小限制").arg(removed));
            }
        }
    }
}

} // namespace Data
} // namespace ResticGUI
