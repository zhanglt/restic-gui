#ifndef RESTOREMANAGERTEST_H
#define RESTOREMANAGERTEST_H

#include "common/TestBase.h"
#include "core/RestoreManager.h"
#include "core/RepositoryManager.h"
#include "data/DatabaseManager.h"
#include "models/RestoreOptions.h"
#include "models/Repository.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief RestoreManager 测试类
 *
 * 测试恢复管理器的功能：
 * - 快照恢复操作
 * - 特定文件恢复
 * - 恢复到指定目录
 * - 取消恢复
 * - 挂载操作（Unix）
 * - 信号机制
 * - 错误处理
 */
class RestoreManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 恢复操作测试
    void testRestoreSnapshot();
    void testRestoreSpecificFiles();
    void testRestoreToTarget();
    void testCancelRestore();

    // 挂载操作测试（仅Unix）
    void testMountRepository();
    void testUnmountRepository();

    // 信号测试
    void testRestoreSignals();

    // 错误处理测试
    void testInvalidRestore();

private:
    Core::RestoreManager* m_manager;
    Core::RepositoryManager* m_repoManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
    QString m_testRepoPath;
    QString m_testPassword;

    // 辅助方法
    Models::Repository createTestRepository();
    Models::RestoreOptions createTestRestoreOptions();
};

} // namespace Test
} // namespace ResticGUI

#endif // RESTOREMANAGERTEST_H
