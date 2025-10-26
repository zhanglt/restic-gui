#ifndef RESTOREFLOWTEST_H
#define RESTOREFLOWTEST_H

#include "common/TestBase.h"
#include "core/RestoreManager.h"
#include "core/RepositoryManager.h"
#include "core/SnapshotManager.h"
#include "data/DatabaseManager.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief RestoreFlow 集成测试类
 *
 * 测试完整的恢复流程：
 * - 创建仓库 -> 列出快照 -> 浏览文件 -> 执行恢复
 * - 特定文件恢复流程
 */
class RestoreFlowTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 完整恢复流程测试
    void testCompleteRestoreFlow();

    // 特定文件恢复流程测试
    void testSelectiveRestoreFlow();

private:
    Core::RestoreManager* m_restoreManager;
    Core::RepositoryManager* m_repoManager;
    Core::SnapshotManager* m_snapshotManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
};

} // namespace Test
} // namespace ResticGUI

#endif // RESTOREFLOWTEST_H
