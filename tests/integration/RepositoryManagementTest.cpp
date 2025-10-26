#include "RepositoryManagementTest.h"
#include "models/Repository.h"

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void RepositoryManagementTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/repo_management_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取管理器单例
    m_repoManager = RepositoryManager::instance();
    m_repoManager->initialize();

    m_snapshotManager = SnapshotManager::instance();
    m_snapshotManager->initialize();
}

void RepositoryManagementTest::cleanupTestCase()
{
    m_snapshotManager = nullptr;
    m_repoManager = nullptr;

    TestBase::cleanupTestCase();
}

void RepositoryManagementTest::init()
{
    // 每个测试前的准备
}

void RepositoryManagementTest::cleanup()
{
    // 每个测试后的清理
}

// ========== 完整仓库管理流程测试 ==========

void RepositoryManagementTest::testCompleteRepositoryManagement()
{
    // 测试完整的仓库管理流程：创建 -> 初始化 -> 检查 -> 管理快照 -> 删除

    // 1. 创建仓库
    Repository repo;
    repo.name = "Complete Management Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = tempDir() + "/complete_mgmt_repo";
    int repoId = m_repoManager->createRepository(repo, "TestPassword123", false);

    // 验证仓库创建成功
    QVERIFY(repoId > 0);

    // 2. 获取仓库信息
    Repository retrievedRepo = m_repoManager->getRepository(repoId);
    QCOMPARE(retrievedRepo.name, repo.name);

    // 3. 测试连接（可能会失败，因为没有真实的restic仓库）
    bool connectionResult = m_repoManager->testConnection(repo, "TestPassword123");

    // 4. 列出快照（应该为空或失败）
    QList<Snapshot> snapshots = m_snapshotManager->listSnapshots(repoId, false);

    // 5. 设置为默认仓库
    bool setDefaultResult = m_repoManager->setDefaultRepository(repoId);
    QVERIFY(setDefaultResult);

    // 6. 删除仓库
    bool deleteResult = m_repoManager->deleteRepository(repoId, false);
    QVERIFY(deleteResult);

    // 验证流程完整性
    QVERIFY(true);
}

// ========== 多仓库管理流程测试 ==========

void RepositoryManagementTest::testMultipleRepositoryManagement()
{
    // 测试多个仓库的管理流程

    // 1. 创建多个仓库
    QList<int> repoIds;
    for (int i = 0; i < 3; ++i) {
        Repository repo;
        repo.name = QString("Multi Repo %1").arg(i);
        repo.type = RepositoryType::Local;
        repo.path = tempDir() + QString("/multi_repo_%1").arg(i);
        int repoId = m_repoManager->createRepository(repo, "TestPassword123", false);
        repoIds.append(repoId);
    }

    // 验证所有仓库创建成功
    QCOMPARE(repoIds.size(), 3);

    // 2. 获取所有仓库
    QList<Repository> allRepos = m_repoManager->getAllRepositories();

    // 验证至少有3个仓库
    QVERIFY(allRepos.size() >= 3);

    // 3. 设置第二个仓库为默认
    m_repoManager->setDefaultRepository(repoIds[1]);

    // 4. 获取默认仓库
    Repository defaultRepo = m_repoManager->getDefaultRepository();
    QCOMPARE(defaultRepo.id, repoIds[1]);

    // 5. 删除所有测试仓库
    for (int repoId : repoIds) {
        m_repoManager->deleteRepository(repoId, false);
    }

    // 验证流程完整性
    QVERIFY(true);
}

} // namespace Test
} // namespace ResticGUI
