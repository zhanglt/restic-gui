#ifndef REPOSITORYMANAGERTEST_H
#define REPOSITORYMANAGERTEST_H

#include "common/TestBase.h"
#include "core/RepositoryManager.h"
#include "data/DatabaseManager.h"
#include "models/Repository.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief RepositoryManager 测试类
 *
 * 测试仓库管理器的核心功能：
 * - 仓库 CRUD 操作
 * - 仓库初始化和检查
 * - 连接测试
 * - 默认仓库管理
 * - 密码管理
 * - 信号机制
 * - 错误处理
 */
class RepositoryManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 仓库 CRUD 测试
    void testCreateRepository();
    void testUpdateRepository();
    void testDeleteRepository();
    void testGetRepository();
    void testGetAllRepositories();

    // 默认仓库测试
    void testSetDefaultRepository();
    void testGetDefaultRepository();

    // 仓库操作测试
    void testTestConnection();
    void testCheckRepository();

    // 密码管理测试
    void testPasswordManagement();

    // 信号测试
    void testRepositoryCreatedSignal();
    void testRepositoryUpdatedSignal();
    void testRepositoryDeletedSignal();

    // 错误处理测试
    void testInvalidRepositoryId();

private:
    Core::RepositoryManager* m_manager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
    QString m_testRepoPath;
    QString m_testPassword;

    // 辅助方法
    Models::Repository createTestRepository();
};

} // namespace Test
} // namespace ResticGUI

#endif // REPOSITORYMANAGERTEST_H
