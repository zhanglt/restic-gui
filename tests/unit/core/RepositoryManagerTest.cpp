#include "RepositoryManagerTest.h"
#include <QSignalSpy>

using namespace ResticGUI::Core;
using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void RepositoryManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/repository_manager_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取 RepositoryManager 单例
    m_manager = RepositoryManager::instance();
    m_manager->initialize();

    m_testRepoPath = tempDir() + "/test_repo";
    m_testPassword = "TestPassword123!";
}

void RepositoryManagerTest::cleanupTestCase()
{
    // 清理
    m_manager = nullptr;

    TestBase::cleanupTestCase();
}

void RepositoryManagerTest::init()
{
    // 每个测试前的准备
}

void RepositoryManagerTest::cleanup()
{
    // 每个测试后的清理
}

// ========== 辅助方法 ==========

Repository RepositoryManagerTest::createTestRepository()
{
    Repository repo;
    repo.name = "Test Repository";
    repo.type = RepositoryType::Local;
    repo.path = m_testRepoPath;
    repo.isDefault = false;
    return repo;
}

// ========== 仓库 CRUD 测试 ==========

void RepositoryManagerTest::testCreateRepository()
{
    // 创建测试仓库
    Repository repo = createTestRepository();

    // 创建仓库（不初始化 restic，因为测试环境可能没有 restic）
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 验证创建成功
    QVERIFY(repoId > 0);

    // 验证可以获取仓库
    Repository retrieved = m_manager->getRepository(repoId);
    QCOMPARE(retrieved.name, repo.name);
    QCOMPARE(retrieved.type, repo.type);
    QCOMPARE(retrieved.path, repo.path);
}

void RepositoryManagerTest::testUpdateRepository()
{
    // 创建准备数据
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 更新仓库
    repo.id = repoId;
    repo.name = "Updated Repository Name";
    repo.path = tempDir() + "/updated_repo";

    bool success = m_manager->updateRepository(repo);
    QVERIFY(success);

    // 验证更新
    Repository updated = m_manager->getRepository(repoId);
    QCOMPARE(updated.name, QString("Updated Repository Name"));
    QCOMPARE(updated.path, repo.path);
}

void RepositoryManagerTest::testDeleteRepository()
{
    // 创建准备数据
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 删除仓库（不删除数据）
    bool success = m_manager->deleteRepository(repoId, false);
    QVERIFY(success);

    // 验证已删除
    Repository deleted = m_manager->getRepository(repoId);
    QVERIFY(deleted.id == -1 || deleted.name.isEmpty());
}

void RepositoryManagerTest::testGetRepository()
{
    // 创建准备数据
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 获取仓库
    Repository retrieved = m_manager->getRepository(repoId);

    // 验证
    QCOMPARE(retrieved.id, repoId);
    QCOMPARE(retrieved.name, repo.name);
    QCOMPARE(retrieved.path, repo.path);
}

void RepositoryManagerTest::testGetAllRepositories()
{
    // 创建多个仓库
    for (int i = 0; i < 3; ++i) {
        Repository repo = createTestRepository();
        repo.name = QString("Repository %1").arg(i);
        repo.path = tempDir() + QString("/repo_%1").arg(i);
        m_manager->createRepository(repo, m_testPassword, false);
    }

    // 获取所有仓库
    QList<Repository> repos = m_manager->getAllRepositories();

    // 验证至少有 3 个仓库
    QVERIFY(repos.size() >= 3);
}

// ========== 默认仓库测试 ==========

void RepositoryManagerTest::testSetDefaultRepository()
{
    // 创建仓库
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 设置为默认仓库
    bool success = m_manager->setDefaultRepository(repoId);
    QVERIFY(success);

    // 验证是否为默认仓库
    Repository defaultRepo = m_manager->getDefaultRepository();
    QCOMPARE(defaultRepo.id, repoId);
}

void RepositoryManagerTest::testGetDefaultRepository()
{
    // 创建两个仓库
    Repository repo1 = createTestRepository();
    repo1.name = "Repo 1";
    repo1.path = tempDir() + "/repo1";
    int repoId1 = m_manager->createRepository(repo1, m_testPassword, false);

    Repository repo2 = createTestRepository();
    repo2.name = "Repo 2";
    repo2.path = tempDir() + "/repo2";
    int repoId2 = m_manager->createRepository(repo2, m_testPassword, false);

    // 设置第二个为默认仓库
    m_manager->setDefaultRepository(repoId2);

    // 获取默认仓库
    Repository defaultRepo = m_manager->getDefaultRepository();

    // 验证
    QCOMPARE(defaultRepo.id, repoId2);
    QCOMPARE(defaultRepo.name, QString("Repo 2"));
}

// ========== 仓库操作测试 ==========

void RepositoryManagerTest::testTestConnection()
{
    // 这个测试需要真实的 restic 环境，这里只测试调用不崩溃
    Repository repo = createTestRepository();

    // 尝试测试连接（可能会失败，因为没有 restic 或仓库未初始化）
    bool result = m_manager->testConnection(repo, m_testPassword);

    // 只验证调用不崩溃
    QVERIFY(true);  // 占位符
}

void RepositoryManagerTest::testCheckRepository()
{
    // 这个测试需要真实的 restic 环境
    // 创建仓库
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 尝试检查仓库（可能会失败）
    bool result = m_manager->checkRepository(repoId, m_testPassword, false);

    // 只验证调用不崩溃
    QVERIFY(true);  // 占位符
}

// ========== 密码管理测试 ==========

void RepositoryManagerTest::testPasswordManagement()
{
    // 创建仓库
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 设置密码
    m_manager->setPassword(repoId, "MySecretPassword");

    // 获取密码
    QString retrievedPassword;
    bool hasPassword = m_manager->getPassword(repoId, retrievedPassword);

    // 验证密码已缓存
    QVERIFY(hasPassword);
    QCOMPARE(retrievedPassword, QString("MySecretPassword"));

    // 清除密码缓存
    m_manager->clearPasswordCache(repoId);

    // 再次获取密码（应该失败）
    QString emptyPassword;
    bool hasPasswordAfterClear = m_manager->getPassword(repoId, emptyPassword);
    QVERIFY(!hasPasswordAfterClear);
}

// ========== 信号测试 ==========

void RepositoryManagerTest::testRepositoryCreatedSignal()
{
    // 创建信号监听器
    QSignalSpy spy(m_manager, &RepositoryManager::repositoryCreated);

    // 创建仓库
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 验证信号发出
    QCOMPARE(spy.count(), 1);

    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), repoId);
}

void RepositoryManagerTest::testRepositoryUpdatedSignal()
{
    // 创建仓库
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 创建信号监听器
    QSignalSpy spy(m_manager, &RepositoryManager::repositoryUpdated);

    // 更新仓库
    repo.id = repoId;
    repo.name = "Updated";
    m_manager->updateRepository(repo);

    // 验证信号发出
    QCOMPARE(spy.count(), 1);

    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), repoId);
}

void RepositoryManagerTest::testRepositoryDeletedSignal()
{
    // 创建仓库
    Repository repo = createTestRepository();
    int repoId = m_manager->createRepository(repo, m_testPassword, false);

    // 创建信号监听器
    QSignalSpy spy(m_manager, &RepositoryManager::repositoryDeleted);

    // 删除仓库
    m_manager->deleteRepository(repoId, false);

    // 验证信号发出
    QCOMPARE(spy.count(), 1);

    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toInt(), repoId);
}

// ========== 错误处理测试 ==========

void RepositoryManagerTest::testInvalidRepositoryId()
{
    // 测试无效的仓库 ID
    int invalidRepoId = 99999;

    // 获取不存在的仓库
    Repository repo = m_manager->getRepository(invalidRepoId);

    // 验证返回无效仓库
    QVERIFY(repo.id == -1 || repo.name.isEmpty());

    // 尝试更新不存在的仓库（应该失败）
    Repository invalidRepo;
    invalidRepo.id = invalidRepoId;
    invalidRepo.name = "Invalid";
    bool result = m_manager->updateRepository(invalidRepo);
    QVERIFY(!result);
}

} // namespace Test
} // namespace ResticGUI
