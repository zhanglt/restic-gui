#ifndef BACKUPFLOWTEST_H
#define BACKUPFLOWTEST_H

#include "common/TestBase.h"
#include "core/BackupManager.h"
#include "core/RepositoryManager.h"
#include "data/DatabaseManager.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief BackupFlow 集成测试类
 *
 * 测试完整的备份流程：
 * - 创建仓库 -> 创建备份任务 -> 执行备份 -> 查看历史
 * - 多任务备份流程
 */
class BackupFlowTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 完整备份流程测试
    void testCompleteBackupFlow();

    // 多任务备份流程测试
    void testMultipleBackupTasks();

private:
    Core::BackupManager* m_backupManager;
    Core::RepositoryManager* m_repoManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
};

} // namespace Test
} // namespace ResticGUI

#endif // BACKUPFLOWTEST_H
