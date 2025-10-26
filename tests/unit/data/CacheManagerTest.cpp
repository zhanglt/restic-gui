#include "CacheManagerTest.h"
#include <QSignalSpy>
#include <QDateTime>

using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void CacheManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 获取 CacheManager 单例
    m_manager = CacheManager::instance();
    m_manager->initialize();
}

void CacheManagerTest::cleanupTestCase()
{
    // 清理所有缓存
    m_manager->clearAllCache();
    m_manager = nullptr;

    TestBase::cleanupTestCase();
}

void CacheManagerTest::init()
{
    // 每个测试前的准备 - 清除所有缓存
    m_manager->clearAllCache();
}

void CacheManagerTest::cleanup()
{
    // 每个测试后的清理
    m_manager->clearAllCache();
}

// ========== 辅助方法 ==========

Snapshot CacheManagerTest::createTestSnapshot(const QString& id)
{
    Snapshot snapshot;
    snapshot.id = id;
    snapshot.time = QDateTime::currentDateTime();
    snapshot.hostname = "test-host";
    snapshot.username = "test-user";
    snapshot.paths = QStringList() << "/home/test";
    snapshot.tags = QStringList() << "test";
    return snapshot;
}

FileInfo CacheManagerTest::createTestFileInfo(const QString& name)
{
    FileInfo fileInfo;
    fileInfo.name = name;
    fileInfo.path = "/test/" + name;
    fileInfo.size = 1024;
    fileInfo.type = FileType::File;
    fileInfo.mtime = QDateTime::currentDateTime();
    return fileInfo;
}

RepoStats CacheManagerTest::createTestRepoStats()
{
    RepoStats stats;
    stats.totalSize = 1024 * 1024 * 100; // 100MB
    stats.totalFileCount = 1000;
    stats.snapshotCount = 10;
    stats.uniqueSize = 1024 * 1024 * 50; // 50MB
    return stats;
}

// ========== 快照缓存测试 ==========

void CacheManagerTest::testCacheSnapshots()
{
    // 测试缓存快照列表

    int testRepoId = 1;
    QList<Snapshot> snapshots;
    snapshots << createTestSnapshot("snapshot1");
    snapshots << createTestSnapshot("snapshot2");
    snapshots << createTestSnapshot("snapshot3");

    // 缓存快照（不持久化到数据库）
    m_manager->cacheSnapshots(testRepoId, snapshots, false);

    // 验证缓存成功
    QList<Snapshot> cachedSnapshots;
    bool success = m_manager->getCachedSnapshots(testRepoId, cachedSnapshots);

    QVERIFY(success);
    QCOMPARE(cachedSnapshots.size(), snapshots.size());
    QCOMPARE(cachedSnapshots[0].id, snapshots[0].id);
}

void CacheManagerTest::testGetCachedSnapshots()
{
    // 测试获取缓存的快照

    int testRepoId = 1;
    QList<Snapshot> snapshots;
    snapshots << createTestSnapshot("snapshot1");

    // 先缓存
    m_manager->cacheSnapshots(testRepoId, snapshots, false);

    // 获取缓存
    QList<Snapshot> cachedSnapshots;
    bool success = m_manager->getCachedSnapshots(testRepoId, cachedSnapshots);

    // 验证
    QVERIFY(success);
    QCOMPARE(cachedSnapshots.size(), 1);
    QCOMPARE(cachedSnapshots[0].id, QString("snapshot1"));
}

void CacheManagerTest::testClearSnapshotCache()
{
    // 测试清除快照缓存

    int testRepoId = 1;
    QList<Snapshot> snapshots;
    snapshots << createTestSnapshot("snapshot1");

    // 缓存快照
    m_manager->cacheSnapshots(testRepoId, snapshots, false);

    // 验证缓存存在
    QList<Snapshot> cachedSnapshots;
    QVERIFY(m_manager->getCachedSnapshots(testRepoId, cachedSnapshots));

    // 清除缓存
    m_manager->clearSnapshotCache(testRepoId);

    // 验证缓存已清除
    QList<Snapshot> emptySnapshots;
    bool hasCache = m_manager->getCachedSnapshots(testRepoId, emptySnapshots);
    QVERIFY(!hasCache);
}

void CacheManagerTest::testIsSnapshotCacheValid()
{
    // 测试快照缓存有效性检查

    int testRepoId = 1;
    QList<Snapshot> snapshots;
    snapshots << createTestSnapshot("snapshot1");

    // 缓存快照
    m_manager->cacheSnapshots(testRepoId, snapshots, false);

    // 立即检查（应该有效）
    bool isValid = m_manager->isSnapshotCacheValid(testRepoId, 5);
    QVERIFY(isValid);

    // 检查一个不存在的仓库（应该无效）
    bool isInvalid = m_manager->isSnapshotCacheValid(999, 5);
    QVERIFY(!isInvalid);
}

// ========== 文件树缓存测试 ==========

void CacheManagerTest::testCacheFileTree()
{
    // 测试缓存文件树

    QString snapshotId = "snapshot1";
    QString path = "/home/user";
    QList<FileInfo> files;
    files << createTestFileInfo("file1.txt");
    files << createTestFileInfo("file2.txt");

    // 缓存文件树
    m_manager->cacheFileTree(snapshotId, path, files);

    // 验证缓存成功
    QList<FileInfo> cachedFiles;
    bool success = m_manager->getCachedFileTree(snapshotId, path, cachedFiles);

    QVERIFY(success);
    QCOMPARE(cachedFiles.size(), files.size());
    QCOMPARE(cachedFiles[0].name, files[0].name);
}

void CacheManagerTest::testGetCachedFileTree()
{
    // 测试获取缓存的文件树

    QString snapshotId = "snapshot1";
    QString path = "/home/user";
    QList<FileInfo> files;
    files << createTestFileInfo("file1.txt");

    // 先缓存
    m_manager->cacheFileTree(snapshotId, path, files);

    // 获取缓存
    QList<FileInfo> cachedFiles;
    bool success = m_manager->getCachedFileTree(snapshotId, path, cachedFiles);

    // 验证
    QVERIFY(success);
    QCOMPARE(cachedFiles.size(), 1);
    QCOMPARE(cachedFiles[0].name, QString("file1.txt"));
}

void CacheManagerTest::testClearFileTreeCache()
{
    // 测试清除文件树缓存

    QString snapshotId = "snapshot1";
    QString path = "/home/user";
    QList<FileInfo> files;
    files << createTestFileInfo("file1.txt");

    // 缓存文件树
    m_manager->cacheFileTree(snapshotId, path, files);

    // 验证缓存存在
    QList<FileInfo> cachedFiles;
    QVERIFY(m_manager->getCachedFileTree(snapshotId, path, cachedFiles));

    // 清除缓存
    m_manager->clearFileTreeCache(snapshotId);

    // 验证缓存已清除
    QList<FileInfo> emptyFiles;
    bool hasCache = m_manager->getCachedFileTree(snapshotId, path, emptyFiles);
    QVERIFY(!hasCache);
}

// ========== 仓库统计缓存测试 ==========

void CacheManagerTest::testCacheRepoStats()
{
    // 测试缓存仓库统计信息

    int testRepoId = 1;
    RepoStats stats = createTestRepoStats();

    // 缓存统计信息
    m_manager->cacheRepoStats(testRepoId, stats);

    // 验证缓存成功
    RepoStats cachedStats;
    bool success = m_manager->getCachedRepoStats(testRepoId, cachedStats);

    QVERIFY(success);
    QCOMPARE(cachedStats.totalSize, stats.totalSize);
    QCOMPARE(cachedStats.snapshotCount, stats.snapshotCount);
}

void CacheManagerTest::testGetCachedRepoStats()
{
    // 测试获取缓存的仓库统计信息

    int testRepoId = 1;
    RepoStats stats = createTestRepoStats();

    // 先缓存
    m_manager->cacheRepoStats(testRepoId, stats);

    // 获取缓存
    RepoStats cachedStats;
    bool success = m_manager->getCachedRepoStats(testRepoId, cachedStats);

    // 验证
    QVERIFY(success);
    QCOMPARE(cachedStats.totalSize, stats.totalSize);
}

void CacheManagerTest::testClearRepoStatsCache()
{
    // 测试清除仓库统计缓存

    int testRepoId = 1;
    RepoStats stats = createTestRepoStats();

    // 缓存统计信息
    m_manager->cacheRepoStats(testRepoId, stats);

    // 验证缓存存在
    RepoStats cachedStats;
    QVERIFY(m_manager->getCachedRepoStats(testRepoId, cachedStats));

    // 清除缓存
    m_manager->clearRepoStatsCache(testRepoId);

    // 验证缓存已清除
    RepoStats emptyStats;
    bool hasCache = m_manager->getCachedRepoStats(testRepoId, emptyStats);
    QVERIFY(!hasCache);
}

void CacheManagerTest::testIsRepoStatsCacheValid()
{
    // 测试仓库统计缓存有效性检查

    int testRepoId = 1;
    RepoStats stats = createTestRepoStats();

    // 缓存统计信息
    m_manager->cacheRepoStats(testRepoId, stats);

    // 立即检查（应该有效）
    bool isValid = m_manager->isRepoStatsCacheValid(testRepoId, 10);
    QVERIFY(isValid);

    // 检查一个不存在的仓库（应该无效）
    bool isInvalid = m_manager->isRepoStatsCacheValid(999, 10);
    QVERIFY(!isInvalid);
}

// ========== 通用缓存管理测试 ==========

void CacheManagerTest::testClearAllCache()
{
    // 测试清除所有缓存

    // 缓存多个类型的数据
    int testRepoId = 1;
    QList<Snapshot> snapshots;
    snapshots << createTestSnapshot("snapshot1");
    m_manager->cacheSnapshots(testRepoId, snapshots, false);

    QString snapshotId = "snapshot1";
    QString path = "/home/user";
    QList<FileInfo> files;
    files << createTestFileInfo("file1.txt");
    m_manager->cacheFileTree(snapshotId, path, files);

    RepoStats stats = createTestRepoStats();
    m_manager->cacheRepoStats(testRepoId, stats);

    // 清除所有缓存
    m_manager->clearAllCache();

    // 验证所有缓存已清除
    QList<Snapshot> emptySnapshots;
    QVERIFY(!m_manager->getCachedSnapshots(testRepoId, emptySnapshots));

    QList<FileInfo> emptyFiles;
    QVERIFY(!m_manager->getCachedFileTree(snapshotId, path, emptyFiles));

    RepoStats emptyStats;
    QVERIFY(!m_manager->getCachedRepoStats(testRepoId, emptyStats));
}

void CacheManagerTest::testClearRepositoryCache()
{
    // 测试清除指定仓库的所有缓存

    int testRepoId = 1;

    // 缓存快照和统计信息
    QList<Snapshot> snapshots;
    snapshots << createTestSnapshot("snapshot1");
    m_manager->cacheSnapshots(testRepoId, snapshots, false);

    RepoStats stats = createTestRepoStats();
    m_manager->cacheRepoStats(testRepoId, stats);

    // 清除仓库缓存
    m_manager->clearRepositoryCache(testRepoId);

    // 验证缓存已清除
    QList<Snapshot> emptySnapshots;
    QVERIFY(!m_manager->getCachedSnapshots(testRepoId, emptySnapshots));

    RepoStats emptyStats;
    QVERIFY(!m_manager->getCachedRepoStats(testRepoId, emptyStats));
}

void CacheManagerTest::testMaxCacheSize()
{
    // 测试最大缓存大小设置

    int testSize = 100; // 100MB

    // 设置最大缓存大小
    m_manager->setMaxCacheSize(testSize);

    // 获取最大缓存大小
    int retrievedSize = m_manager->getMaxCacheSize();

    // 验证
    QCOMPARE(retrievedSize, testSize);
}

// ========== 信号测试 ==========

void CacheManagerTest::testCacheUpdatedSignal()
{
    // 测试缓存更新信号

    CacheManager* manager = CacheManager::instance();

    // 创建信号监听器
    QSignalSpy spy(manager, &CacheManager::cacheUpdated);

    // 验证信号有效
    QVERIFY(spy.isValid());
}

void CacheManagerTest::testCacheClearedSignal()
{
    // 测试缓存清除信号

    CacheManager* manager = CacheManager::instance();

    // 创建信号监听器
    QSignalSpy spy(manager, &CacheManager::cacheCleared);

    // 验证信号有效
    QVERIFY(spy.isValid());
}

} // namespace Test
} // namespace ResticGUI
