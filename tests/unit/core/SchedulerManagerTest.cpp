#include "SchedulerManagerTest.h"
#include <QSignalSpy>
#include <QDateTime>

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void SchedulerManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/scheduler_manager_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取管理器单例
    m_backupManager = BackupManager::instance();
    m_backupManager->initialize();

    m_manager = SchedulerManager::instance();
    m_manager->initialize();
}

void SchedulerManagerTest::cleanupTestCase()
{
    // 确保停止调度器
    if (m_manager->isRunning()) {
        m_manager->stop();
    }

    m_manager = nullptr;
    m_backupManager = nullptr;

    TestBase::cleanupTestCase();
}

void SchedulerManagerTest::init()
{
    // 每个测试前的准备
}

void SchedulerManagerTest::cleanup()
{
    // 每个测试后的清理 - 停止调度器
    if (m_manager->isRunning()) {
        m_manager->stop();
    }
}

// ========== 辅助方法 ==========

Repository SchedulerManagerTest::createTestRepository()
{
    Repository repo;
    repo.name = "Test Scheduler Repo";
    repo.type = RepositoryType::Local;
    repo.path = tempDir() + "/test_repo";
    repo.id = m_dbManager->insertRepository(repo);
    return repo;
}

BackupTask SchedulerManagerTest::createTestBackupTask(int repoId)
{
    BackupTask task;
    task.name = "Test Scheduled Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << tempDir() + "/source";

    // 创建调度设置（每天执行）
    Schedule schedule;
    schedule.type = Schedule::Type::Daily;
    schedule.time = QTime(2, 0);  // 凌晨2点
    task.schedule = schedule;

    return task;
}

// ========== 调度控制测试 ==========

void SchedulerManagerTest::testStartScheduler()
{
    // 确保调度器未运行
    if (m_manager->isRunning()) {
        m_manager->stop();
    }

    // 启动调度器
    m_manager->start();

    // 验证调度器正在运行
    QVERIFY(m_manager->isRunning());
}

void SchedulerManagerTest::testStopScheduler()
{
    // 启动调度器
    m_manager->start();
    QVERIFY(m_manager->isRunning());

    // 停止调度器
    m_manager->stop();

    // 验证调度器已停止
    QVERIFY(!m_manager->isRunning());
}

void SchedulerManagerTest::testIsRunning()
{
    // 测试 isRunning 状态

    // 初始状态
    bool initialState = m_manager->isRunning();

    // 启动
    m_manager->start();
    QVERIFY(m_manager->isRunning());
    QCOMPARE(m_manager->isRunning(), true);

    // 停止
    m_manager->stop();
    QVERIFY(!m_manager->isRunning());
    QCOMPARE(m_manager->isRunning(), false);
}

// ========== 任务调度测试 ==========

void SchedulerManagerTest::testUpdateTaskNextRun()
{
    // 创建测试任务
    Repository repo = createTestRepository();
    BackupTask task = createTestBackupTask(repo.id);
    int taskId = m_backupManager->createBackupTask(task);

    // 更新任务的下次运行时间
    m_manager->updateTaskNextRun(taskId);

    // 验证调用不崩溃
    QVERIFY(true);
}

void SchedulerManagerTest::testRemoveTask()
{
    // 创建测试任务
    Repository repo = createTestRepository();
    BackupTask task = createTestBackupTask(repo.id);
    int taskId = m_backupManager->createBackupTask(task);

    // 先添加到调度器
    m_manager->updateTaskNextRun(taskId);

    // 移除任务
    m_manager->removeTask(taskId);

    // 验证调用不崩溃
    QVERIFY(true);
}

void SchedulerManagerTest::testCheckScheduledTasks()
{
    // 测试手动检查调度任务

    // 创建测试任务
    Repository repo = createTestRepository();
    BackupTask task = createTestBackupTask(repo.id);
    int taskId = m_backupManager->createBackupTask(task);

    // 添加到调度器
    m_manager->updateTaskNextRun(taskId);

    // 手动触发检查
    m_manager->checkScheduledTasks();

    // 验证调用不崩溃
    QVERIFY(true);
}

// ========== 信号测试 ==========

void SchedulerManagerTest::testTaskScheduledSignal()
{
    // 测试任务调度信号

    SchedulerManager* manager = SchedulerManager::instance();

    // 创建信号监听器
    QSignalSpy spy(manager, &SchedulerManager::taskScheduled);

    // 验证信号有效
    QVERIFY(spy.isValid());

    // 创建任务并更新调度
    Repository repo = createTestRepository();
    BackupTask task = createTestBackupTask(repo.id);
    int taskId = m_backupManager->createBackupTask(task);

    // 更新下次运行时间（可能触发信号）
    manager->updateTaskNextRun(taskId);

    // 注意：信号可能不会立即发出（取决于实现）
    // 这里只验证信号有效
}

void SchedulerManagerTest::testTaskTriggeredSignal()
{
    // 测试任务触发信号

    SchedulerManager* manager = SchedulerManager::instance();

    // 创建信号监听器
    QSignalSpy spy(manager, &SchedulerManager::taskTriggered);

    // 验证信号有效
    QVERIFY(spy.isValid());
}

} // namespace Test
} // namespace ResticGUI
