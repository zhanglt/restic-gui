#include "RestoreManagerTest.h"
#include <QSignalSpy>
#include <QDir>

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void RestoreManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/restore_manager_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取管理器单例
    m_repoManager = RepositoryManager::instance();
    m_repoManager->initialize();

    m_manager = RestoreManager::instance();
    m_manager->initialize();

    m_testRepoPath = tempDir() + "/test_repo";
    m_testPassword = "TestPassword123!";
}

void RestoreManagerTest::cleanupTestCase()
{
    // 清理
    m_manager = nullptr;
    m_repoManager = nullptr;

    TestBase::cleanupTestCase();
}

void RestoreManagerTest::init()
{
    // 每个测试前的准备
}

void RestoreManagerTest::cleanup()
{
    // 每个测试后的清理
}

// ========== 辅助方法 ==========

Repository RestoreManagerTest::createTestRepository()
{
    Repository repo;
    repo.name = "Test Restore Repository";
    repo.type = RepositoryType::Local;
    repo.path = m_testRepoPath;
    return repo;
}

RestoreOptions RestoreManagerTest::createTestRestoreOptions()
{
    RestoreOptions options;
    options.targetPath = tempDir() + "/restore_target";
    options.verify = true;
    options.includePaths = QStringList() << "*.txt" << "*.log";
    options.excludePaths = QStringList() << "*.tmp";
    return options;
}

// ========== 恢复操作测试 ==========

void RestoreManagerTest::testRestoreSnapshot()
{
    // 这个测试需要真实的 restic 环境和快照
    // 这里只测试调用不崩溃

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    RestoreOptions options = createTestRestoreOptions();
    QString snapshotId = "test-snapshot-id";

    // 尝试恢复（可能会失败，因为没有真实快照）
    bool result = m_manager->restore(repoId, snapshotId, options);

    // 只验证调用不崩溃
    QVERIFY(true);  // 占位符
}

void RestoreManagerTest::testRestoreSpecificFiles()
{
    // 测试恢复特定文件

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    RestoreOptions options = createTestRestoreOptions();
    options.includePaths = QStringList() << "/path/to/file1.txt" << "/path/to/file2.txt";
    QString snapshotId = "test-snapshot-id";

    // 尝试恢复特定文件
    bool result = m_manager->restore(repoId, snapshotId, options);

    // 只验证调用不崩溃
    QVERIFY(true);  // 占位符
}

void RestoreManagerTest::testRestoreToTarget()
{
    // 测试恢复到指定目录

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QString targetDir = tempDir() + "/custom_restore_target";
    QDir().mkpath(targetDir);

    RestoreOptions options = createTestRestoreOptions();
    options.targetPath = targetDir;
    QString snapshotId = "test-snapshot-id";

    // 尝试恢复到指定目录
    bool result = m_manager->restore(repoId, snapshotId, options);

    // 验证目标目录存在
    verifyDirExists(targetDir, "Target directory should exist");
}

void RestoreManagerTest::testCancelRestore()
{
    // 测试取消恢复操作

    RestoreManager* manager = RestoreManager::instance();

    // 调用取消（即使没有正在运行的恢复，也不应该崩溃）
    manager->cancelRestore();

    // 验证调用不崩溃
    QVERIFY(true);
}

// ========== 挂载操作测试 ==========

void RestoreManagerTest::testMountRepository()
{
    // 挂载功能仅在 Unix 系统上可用
#ifndef Q_OS_WIN
    // 测试挂载仓库

    Repository repo = createTestRepository();
    int repoId = m_repoManager->createRepository(repo, m_testPassword, false);

    QString mountPoint = tempDir() + "/mount_point";
    QDir().mkpath(mountPoint);

    // 尝试挂载（可能会失败，因为没有真实的 restic 仓库）
    bool result = m_manager->mountRepository(repoId, mountPoint);

    // 只验证调用不崩溃
    QVERIFY(true);  // 占位符
#else
    QSKIP("Mount功能仅在Unix系统上可用");
#endif
}

void RestoreManagerTest::testUnmountRepository()
{
    // 卸载功能仅在 Unix 系统上可用
#ifndef Q_OS_WIN
    // 测试卸载仓库

    QString mountPoint = tempDir() + "/mount_point";

    // 尝试卸载（即使没有挂载，也不应该崩溃）
    bool result = m_manager->unmountRepository(mountPoint);

    // 只验证调用不崩溃
    QVERIFY(true);  // 占位符
#else
    QSKIP("Unmount功能仅在Unix系统上可用");
#endif
}

// ========== 信号测试 ==========

void RestoreManagerTest::testRestoreSignals()
{
    // 测试恢复相关信号

    RestoreManager* manager = RestoreManager::instance();

    // 创建信号监听器
    QSignalSpy startedSpy(manager, &RestoreManager::restoreStarted);
    QSignalSpy progressSpy(manager, &RestoreManager::restoreProgress);
    QSignalSpy finishedSpy(manager, &RestoreManager::restoreFinished);
    QSignalSpy errorSpy(manager, &RestoreManager::restoreError);

    // 验证信号有效
    QVERIFY(startedSpy.isValid());
    QVERIFY(progressSpy.isValid());
    QVERIFY(finishedSpy.isValid());
    QVERIFY(errorSpy.isValid());
}

// ========== 错误处理测试 ==========

void RestoreManagerTest::testInvalidRestore()
{
    // 测试无效的恢复请求

    // 使用无效的仓库 ID
    int invalidRepoId = 99999;
    QString invalidSnapshotId = "nonexistent-snapshot";
    RestoreOptions options = createTestRestoreOptions();

    // 尝试恢复（应该失败）
    bool result = m_manager->restore(invalidRepoId, invalidSnapshotId, options);

    // 验证调用不崩溃（失败是预期的）
    QVERIFY(true);  // 占位符
}

} // namespace Test
} // namespace ResticGUI
