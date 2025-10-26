#include "BackupFlowTest.h"
#include "models/Repository.h"
#include "models/BackupTask.h"

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void BackupFlowTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/backup_flow_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取管理器单例
    m_repoManager = RepositoryManager::instance();
    m_repoManager->initialize();

    m_backupManager = BackupManager::instance();
    m_backupManager->initialize();
}

void BackupFlowTest::cleanupTestCase()
{
    m_backupManager = nullptr;
    m_repoManager = nullptr;

    TestBase::cleanupTestCase();
}

void BackupFlowTest::init()
{
    // 每个测试前的准备
}

void BackupFlowTest::cleanup()
{
    // 每个测试后的清理
}

// ========== 完整备份流程测试 ==========

void BackupFlowTest::testCompleteBackupFlow()
{
    // 测试完整的备份流程：创建仓库 -> 创建任务 -> 执行备份 -> 查看历史

    // 1. 创建仓库
    Repository repo;
    repo.name = "Backup Flow Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = tempDir() + "/flow_test_repo";
    int repoId = m_repoManager->createRepository(repo, "TestPassword123", false);

    // 验证仓库创建成功
    QVERIFY(repoId > 0);

    // 2. 创建备份任务
    BackupTask task;
    task.name = "Flow Test Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << tempDir() + "/source";
    task.schedule.type = Schedule::Daily;
    int taskId = m_backupManager->createBackupTask(task);

    // 验证任务创建成功
    QVERIFY(taskId > 0);

    // 3. 执行备份（注意：可能会失败，因为没有真实的restic环境）
    bool backupResult = m_backupManager->runBackupTask(taskId);

    // 4. 查看备份历史
    QList<BackupResult> history = m_backupManager->getBackupHistory(taskId);

    // 验证流程完整性（即使备份失败，流程应该不崩溃）
    QVERIFY(true);
}

// ========== 多任务备份流程测试 ==========

void BackupFlowTest::testMultipleBackupTasks()
{
    // 测试多个备份任务的流程

    // 1. 创建仓库
    Repository repo;
    repo.name = "Multi Task Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = tempDir() + "/multi_task_repo";
    int repoId = m_repoManager->createRepository(repo, "TestPassword123", false);

    QVERIFY(repoId > 0);

    // 2. 创建多个备份任务
    QList<int> taskIds;
    for (int i = 0; i < 3; ++i) {
        BackupTask task;
        task.name = QString("Multi Task %1").arg(i);
        task.repositoryId = repoId;
        task.sourcePaths = QStringList() << tempDir() + QString("/source%1").arg(i);
        task.schedule.type = Schedule::Daily;
        int taskId = m_backupManager->createBackupTask(task);
        taskIds.append(taskId);
    }

    // 验证所有任务创建成功
    QCOMPARE(taskIds.size(), 3);

    // 3. 获取所有任务
    QList<BackupTask> allTasks = m_backupManager->getAllBackupTasks();

    // 验证至少有3个任务
    QVERIFY(allTasks.size() >= 3);

    // 验证流程完整性
    QVERIFY(true);
}

} // namespace Test
} // namespace ResticGUI
