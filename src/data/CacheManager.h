#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QDateTime>
#include "../models/Snapshot.h"
#include "../models/FileInfo.h"
#include "../models/RepoStats.h"

namespace ResticGUI {
namespace Data {

/**
 * @brief 缓存管理器（单例模式）
 *
 * 提供快照列表、文件树、仓库统计信息等数据的缓存
 * 减少对restic命令的调用，提高响应速度
 */
class CacheManager : public QObject
{
    Q_OBJECT

public:
    // 文件树缓存键（快照ID + 路径）
    struct FileTreeKey {
        QString snapshotId;
        QString path;

        bool operator==(const FileTreeKey& other) const {
            return snapshotId == other.snapshotId && path == other.path;
        }

        friend uint qHash(const FileTreeKey& key, uint seed);
    };

    /**
     * @brief 获取单例实例
     */
    static CacheManager* instance();

    /**
     * @brief 初始化缓存管理器
     */
    void initialize();

    // ========== 快照缓存 ==========

    /**
     * @brief 缓存快照列表
     * @param repoId 仓库ID
     * @param snapshots 快照列表
     * @param persistToDb 是否持久化到数据库
     */
    void cacheSnapshots(int repoId, const QList<Models::Snapshot>& snapshots, bool persistToDb = true);

    /**
     * @brief 获取缓存的快照列表
     * @param repoId 仓库ID
     * @param snapshots 输出参数，快照列表
     * @return 成功返回true
     */
    bool getCachedSnapshots(int repoId, QList<Models::Snapshot>& snapshots);

    /**
     * @brief 清除快照缓存
     * @param repoId 仓库ID
     */
    void clearSnapshotCache(int repoId);

    /**
     * @brief 检查快照缓存是否有效
     * @param repoId 仓库ID
     * @param maxAgeMinutes 最大缓存时间（分钟）
     */
    bool isSnapshotCacheValid(int repoId, int maxAgeMinutes = 5);

    // ========== 文件树缓存 ==========

    /**
     * @brief 缓存快照的文件树
     * @param snapshotId 快照ID
     * @param path 路径
     * @param files 文件列表
     */
    void cacheFileTree(const QString& snapshotId, const QString& path, const QList<Models::FileInfo>& files);

    /**
     * @brief 获取缓存的文件树
     * @param snapshotId 快照ID
     * @param path 路径
     * @param files 输出参数，文件列表
     * @return 成功返回true
     */
    bool getCachedFileTree(const QString& snapshotId, const QString& path, QList<Models::FileInfo>& files);

    /**
     * @brief 清除文件树缓存
     * @param snapshotId 快照ID（为空则清除所有）
     */
    void clearFileTreeCache(const QString& snapshotId = QString());

    // ========== 仓库统计缓存 ==========

    /**
     * @brief 缓存仓库统计信息
     * @param repoId 仓库ID
     * @param stats 统计信息
     */
    void cacheRepoStats(int repoId, const Models::RepoStats& stats);

    /**
     * @brief 获取缓存的仓库统计信息
     * @param repoId 仓库ID
     * @param stats 输出参数，统计信息
     * @return 成功返回true
     */
    bool getCachedRepoStats(int repoId, Models::RepoStats& stats);

    /**
     * @brief 清除仓库统计缓存
     * @param repoId 仓库ID
     */
    void clearRepoStatsCache(int repoId);

    /**
     * @brief 检查仓库统计缓存是否有效
     * @param repoId 仓库ID
     * @param maxAgeMinutes 最大缓存时间（分钟）
     */
    bool isRepoStatsCacheValid(int repoId, int maxAgeMinutes = 10);

    // ========== 通用缓存管理 ==========

    /**
     * @brief 清除所有缓存
     */
    void clearAllCache();

    /**
     * @brief 清除指定仓库的所有缓存
     */
    void clearRepositoryCache(int repoId);

    /**
     * @brief 获取缓存大小（字节）
     */
    qint64 getCacheSize() const;

    /**
     * @brief 设置最大缓存大小（MB）
     */
    void setMaxCacheSize(int sizeMB);

    /**
     * @brief 获取最大缓存大小（MB）
     */
    int getMaxCacheSize() const { return m_maxCacheSizeMB; }

signals:
    /**
     * @brief 缓存更新信号
     */
    void cacheUpdated(int repoId);

    /**
     * @brief 缓存清除信号
     */
    void cacheCleared(int repoId);

private:
    explicit CacheManager(QObject* parent = nullptr);
    ~CacheManager();
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;

    /**
     * @brief 检查并清理过期缓存
     */
    void cleanupExpiredCache();

    /**
     * @brief 检查缓存大小并在必要时清理
     */
    void checkCacheSizeLimit();

    /**
     * @brief 获取缓存大小（不加锁版本，调用前需已持有锁）
     */
    qint64 getCacheSizeUnlocked() const;

private:
    static CacheManager* s_instance;
    static QMutex s_instanceMutex;

    // 快照缓存结构
    struct SnapshotCache {
        QList<Models::Snapshot> snapshots;
        QDateTime timestamp;
    };

    // 文件树缓存结构
    struct FileTreeCache {
        QList<Models::FileInfo> files;
        QDateTime timestamp;
    };

    // 仓库统计缓存结构
    struct RepoStatsCache {
        Models::RepoStats stats;
        QDateTime timestamp;
    };

    // 缓存数据
    QMap<int, SnapshotCache> m_snapshotCache;         // 仓库ID -> 快照缓存
    QMap<QString, FileTreeCache> m_fileTreeCache;     // 快照ID_路径 -> 文件树缓存
    QMap<int, RepoStatsCache> m_repoStatsCache;       // 仓库ID -> 统计缓存

    int m_maxCacheSizeMB;
    mutable QMutex m_mutex;
};

} // namespace Data
} // namespace ResticGUI

// 为FileTreeKey提供hash函数（用于QHash）
inline uint qHash(const ResticGUI::Data::CacheManager::FileTreeKey& key, uint seed = 0)
{
    return qHash(key.snapshotId + key.path, seed);
}

#endif // CACHEMANAGER_H
