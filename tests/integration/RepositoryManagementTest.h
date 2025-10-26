#ifndef REPOSITORYMANAGEMENTTEST_H
#define REPOSITORYMANAGEMENTTEST_H

#include "common/TestBase.h"
#include "core/RepositoryManager.h"
#include "core/SnapshotManager.h"
#include "data/DatabaseManager.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief RepositoryManagement 集成测试类
 *
 * 测试仓库管理的完整流程：
 * - 创建仓库 -> 初始化 -> 检查 -> 管理快照 -> 删除仓库
 * - 多仓库管理流程
 */
class RepositoryManagementTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 完整仓库管理流程测试
    void testCompleteRepositoryManagement();

    // 多仓库管理流程测试
    void testMultipleRepositoryManagement();

private:
    Core::RepositoryManager* m_repoManager;
    Core::SnapshotManager* m_snapshotManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
};

} // namespace Test
} // namespace ResticGUI

#endif // REPOSITORYMANAGEMENTTEST_H
