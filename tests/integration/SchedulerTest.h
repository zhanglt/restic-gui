#ifndef SCHEDULERTEST_H
#define SCHEDULERTEST_H

#include "common/TestBase.h"
#include "core/SchedulerManager.h"
#include "core/BackupManager.h"
#include "core/RepositoryManager.h"
#include "data/DatabaseManager.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief Scheduler 集成测试类
 *
 * 测试调度器的完整流程：
 * - 创建调度任务 -> 启动调度器 -> 触发执行 -> 查看结果
 * - 多任务调度流程
 */
class SchedulerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 完整调度流程测试
    void testCompleteSchedulerFlow();

    // 多任务调度流程测试
    void testMultipleScheduledTasks();

private:
    Core::SchedulerManager* m_scheduler;
    Core::BackupManager* m_backupManager;
    Core::RepositoryManager* m_repoManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
};

} // namespace Test
} // namespace ResticGUI

#endif // SCHEDULERTEST_H
