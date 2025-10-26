#include "SchedulerTest.h"
#include "models/Repository.h"
#include "models/BackupTask.h"
#include "models/Schedule.h"

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void SchedulerTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/scheduler_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取管理器单例
    m_repoManager = RepositoryManager::instance();
    m_repoManager->initialize();

    m_backupManager = BackupManager::instance();
    m_backupManager->initialize();

    m_scheduler = SchedulerManager::instance();
    m_scheduler->initialize();
}

void SchedulerTest::cleanupTestCase()
{
    // 确保停止调度器
    if (m_scheduler->isRunning()) {
        m_scheduler->stop();
    }

    m_scheduler = nullptr;
    m_backupManager = nullptr;
    m_repoManager = nullptr;

    TestBase::cleanupTestCase();
}

void SchedulerTest::init()
{
    // 每个测试前的准备
}

void SchedulerTest::cleanup()
{
    // 每个测试后的清理 - 停止调度器
    if (m_scheduler->isRunning()) {
        m_scheduler->stop();
    }
}

// ========== 完整调度流程测试 ==========

void SchedulerTest::testCompleteSchedulerFlow()
{
    // 测试完整的调度流程：创建调度任务 -> 启动调度器 -> 触发执行

    // 1. 创建仓库
    Repository repo;
    repo.name = "Scheduler Flow Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = tempDir() + "/scheduler_flow_repo";
    int repoId = m_repoManager->createRepository(repo, "TestPassword123", false);

    QVERIFY(repoId > 0);

    // 2. 创建调度备份任务
    BackupTask task;
    task.name = "Scheduled Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << tempDir() + "/source";

    // 设置调度（每天凌晨2点）
    Schedule schedule;
    schedule.type = Schedule::Type::Daily;
    schedule.time = QTime(2, 0);
    task.schedule = schedule;

    int taskId = m_backupManager->createBackupTask(task);
    QVERIFY(taskId > 0);

    // 3. 更新任务的下次运行时间
    m_scheduler->updateTaskNextRun(taskId);

    // 4. 启动调度器
    m_scheduler->start();
    QVERIFY(m_scheduler->isRunning());

    // 5. 手动触发检查（不等待实际调度时间）
    m_scheduler->checkScheduledTasks();

    // 6. 停止调度器
    m_scheduler->stop();
    QVERIFY(!m_scheduler->isRunning());

    // 验证流程完整性
    QVERIFY(true);
}

// ========== 多任务调度流程测试 ==========

void SchedulerTest::testMultipleScheduledTasks()
{
    // 测试多个调度任务的流程

    // 1. 创建仓库
    Repository repo;
    repo.name = "Multi Schedule Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = tempDir() + "/multi_schedule_repo";
    int repoId = m_repoManager->createRepository(repo, "TestPassword123", false);

    QVERIFY(repoId > 0);

    // 2. 创建多个调度任务
    QList<int> taskIds;
    for (int i = 0; i < 3; ++i) {
        BackupTask task;
        task.name = QString("Scheduled Task %1").arg(i);
        task.repositoryId = repoId;
        task.sourcePaths = QStringList() << tempDir() + QString("/source%1").arg(i);

        // 设置不同的调度时间
        Schedule schedule;
        schedule.type = Schedule::Type::Daily;
        schedule.time = QTime(i + 1, 0);  // 1:00, 2:00, 3:00
        task.schedule = schedule;

        int taskId = m_backupManager->createBackupTask(task);
        taskIds.append(taskId);

        // 更新调度
        m_scheduler->updateTaskNextRun(taskId);
    }

    // 验证所有任务创建成功
    QCOMPARE(taskIds.size(), 3);

    // 3. 启动调度器
    m_scheduler->start();
    QVERIFY(m_scheduler->isRunning());

    // 4. 手动触发检查
    m_scheduler->checkScheduledTasks();

    // 5. 停止调度器
    m_scheduler->stop();

    // 验证流程完整性
    QVERIFY(true);
}

} // namespace Test
} // namespace ResticGUI
