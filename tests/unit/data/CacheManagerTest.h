#ifndef CACHEMANAGERTEST_H
#define CACHEMANAGERTEST_H

#include "common/TestBase.h"
#include "data/CacheManager.h"
#include "models/Snapshot.h"
#include "models/FileInfo.h"
#include "models/RepoStats.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief CacheManager 测试类
 *
 * 测试缓存管理器的功能：
 * - 快照缓存（存储、获取、清除、有效性检查）
 * - 文件树缓存（存储、获取、清除）
 * - 仓库统计缓存（存储、获取、清除、有效性检查）
 * - 缓存大小管理
 * - 缓存过期机制
 * - 信号机制
 */
class CacheManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 快照缓存测试
    void testCacheSnapshots();
    void testGetCachedSnapshots();
    void testClearSnapshotCache();
    void testIsSnapshotCacheValid();

    // 文件树缓存测试
    void testCacheFileTree();
    void testGetCachedFileTree();
    void testClearFileTreeCache();

    // 仓库统计缓存测试
    void testCacheRepoStats();
    void testGetCachedRepoStats();
    void testClearRepoStatsCache();
    void testIsRepoStatsCacheValid();

    // 通用缓存管理测试
    void testClearAllCache();
    void testClearRepositoryCache();
    void testMaxCacheSize();

    // 信号测试
    void testCacheUpdatedSignal();
    void testCacheClearedSignal();

private:
    Data::CacheManager* m_manager;

    // 辅助方法
    Models::Snapshot createTestSnapshot(const QString& id);
    Models::FileInfo createTestFileInfo(const QString& name);
    Models::RepoStats createTestRepoStats();
};

} // namespace Test
} // namespace ResticGUI

#endif // CACHEMANAGERTEST_H
