#ifndef SCHEDULERMANAGERTEST_H
#define SCHEDULERMANAGERTEST_H

#include "common/TestBase.h"
#include "core/SchedulerManager.h"
#include "core/BackupManager.h"
#include "data/DatabaseManager.h"
#include "models/BackupTask.h"
#include "models/Repository.h"
#include "models/Schedule.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief SchedulerManager 测试类
 *
 * 测试调度管理器的功能：
 * - 调度启动和停止
 * - 任务调度
 * - 下次运行时间计算
 * - 手动触发检查
 * - 任务移除
 * - 信号机制
 */
class SchedulerManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 调度控制测试
    void testStartScheduler();
    void testStopScheduler();
    void testIsRunning();

    // 任务调度测试
    void testUpdateTaskNextRun();
    void testRemoveTask();
    void testCheckScheduledTasks();

    // 信号测试
    void testTaskScheduledSignal();
    void testTaskTriggeredSignal();

private:
    Core::SchedulerManager* m_manager;
    Core::BackupManager* m_backupManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;

    // 辅助方法
    Models::Repository createTestRepository();
    Models::BackupTask createTestBackupTask(int repoId);
};

} // namespace Test
} // namespace ResticGUI

#endif // SCHEDULERMANAGERTEST_H
