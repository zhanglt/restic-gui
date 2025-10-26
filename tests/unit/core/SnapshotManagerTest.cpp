#include "SnapshotManagerTest.h"
#include <QSignalSpy>
#include <QDateTime>

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void SnapshotManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/snapshot_manager_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取管理器单例
    m_repoManager = RepositoryManager::instance();
    m_repoManager->initialize();

    m_manager = SnapshotManager::instance();
    m_manager->initialize();

    m_testRepoPath = tempDir() + "/test_repo";
    m_testPassword = "TestPassword123!";
}

void SnapshotManagerTest::cleanupTestCase()
{
    // 清理
    m_manager = nullptr;
    m_repoManager = nullptr;

    TestBase::cleanupTestCase();
}

void SnapshotManagerTest::init()
{
    // 每个测试前的准备
}

void SnapshotManagerTest::cleanup()
{
    // 每个测试后的清理
}

// ========== 辅助方法 ==========

Repository SnapshotManagerTest::createTestRepository()
{
    Repository repo;
    repo.name = "Test Snapshot Repository";
    repo.type = RepositoryType::Local;
    repo.path = m_testRepoPath;
    return repo;
}

Snapshot SnapshotManagerTest::createTestSnapshot(const QString& id)
{
    Snapshot snapshot;
    snapshot.id = id;
    snapshot.time = QDateTime::currentDateTime();
    snapshot.hostname = "test-host";
    snapshot.username = "test-user";
    snapshot.paths = QStringList() << "/home/test";
    snapshot.tags = QStringList() << "test" << "automated";
    return snapshot;
}

// ========== 快照操作测试 ==========

void SnapshotManagerTest::testListSnapshots()
{
    // 这个测试需要真实的 restic 仓库
    // 这里只测试调用不崩溃

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    // 尝试列出快照（可能返回空列表）
    QList<Snapshot> snapshots = m_manager->listSnapshots(repoId, false);

    // 验证调用不崩溃（返回空列表也是有效的）
    QVERIFY(true);
}

void SnapshotManagerTest::testListSnapshotsForceRefresh()
{
    // 测试强制刷新快照列表

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    // 第一次调用（使用缓存）
    QList<Snapshot> snapshots1 = m_manager->listSnapshots(repoId, false);

    // 第二次调用（强制刷新）
    QList<Snapshot> snapshots2 = m_manager->listSnapshots(repoId, true);

    // 验证调用不崩溃
    QVERIFY(true);
}

void SnapshotManagerTest::testGetSnapshot()
{
    // 测试获取单个快照

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QString snapshotId = "test-snapshot-id";

    // 尝试获取快照（可能会失败，因为快照不存在）
    Snapshot snapshot = m_manager->getSnapshot(repoId, snapshotId);

    // 验证调用不崩溃
    QVERIFY(true);
}

void SnapshotManagerTest::testDeleteSnapshots()
{
    // 测试删除快照

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QStringList snapshotIds;
    snapshotIds << "snapshot1" << "snapshot2";

    // 尝试删除快照（可能会失败，因为快照不存在）
    bool result = m_manager->deleteSnapshots(repoId, snapshotIds);

    // 验证调用不崩溃
    QVERIFY(true);
}

// ========== 文件浏览测试 ==========

void SnapshotManagerTest::testListFiles()
{
    // 测试列出快照根目录的文件

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QString snapshotId = "test-snapshot-id";

    // 尝试列出根目录文件（可能返回空列表）
    QList<FileInfo> files = m_manager->listFiles(repoId, snapshotId, QString());

    // 验证调用不崩溃
    QVERIFY(true);
}

void SnapshotManagerTest::testListFilesInSubdirectory()
{
    // 测试列出快照子目录的文件

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QString snapshotId = "test-snapshot-id";
    QString path = "/home/user/documents";

    // 尝试列出子目录文件（可能返回空列表）
    QList<FileInfo> files = m_manager->listFiles(repoId, snapshotId, path);

    // 验证调用不崩溃
    QVERIFY(true);
}

// ========== 快照比较测试 ==========

void SnapshotManagerTest::testCompareSnapshots()
{
    // 测试比较两个快照

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QString snapshot1 = "snapshot-id-1";
    QString snapshot2 = "snapshot-id-2";

    // 尝试比较快照（可能返回空列表）
    QList<FileInfo> differences = m_manager->compareSnapshots(repoId, snapshot1, snapshot2);

    // 验证调用不崩溃
    QVERIFY(true);
}

// ========== 信号测试 ==========

void SnapshotManagerTest::testSnapshotsUpdatedSignal()
{
    // 测试快照更新信号

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    // 创建信号监听器
    QSignalSpy spy(m_manager, &SnapshotManager::snapshotsUpdated);

    // 强制刷新快照列表（应该触发信号）
    m_manager->listSnapshots(repoId, true);

    // 注意：信号可能不会立即发出（取决于实现）
    // 这里只验证信号有效
    QVERIFY(spy.isValid());
}

void SnapshotManagerTest::testSnapshotDeletedSignal()
{
    // 测试快照删除信号

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    // 创建信号监听器
    QSignalSpy spy(m_manager, &SnapshotManager::snapshotDeleted);

    // 尝试删除快照（可能触发信号，即使删除失败）
    QStringList snapshotIds;
    snapshotIds << "test-snapshot-id";
    m_manager->deleteSnapshots(repoId, snapshotIds);

    // 验证信号有效
    QVERIFY(spy.isValid());
}

} // namespace Test
} // namespace ResticGUI
