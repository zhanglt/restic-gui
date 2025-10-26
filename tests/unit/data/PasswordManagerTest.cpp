#include "PasswordManagerTest.h"
#include <QSignalSpy>
#include <QCoreApplication>

using namespace ResticGUI::Data;

namespace ResticGUI {
namespace Test {

void PasswordManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 创建测试数据库
    m_testDbPath = tempDir() + "/password_manager_test.db";
    m_dbManager = DatabaseManager::instance();
    m_dbManager->initialize(m_testDbPath);

    // 获取 PasswordManager 单例
    m_manager = PasswordManager::instance();
    m_manager->initialize();
}

void PasswordManagerTest::cleanupTestCase()
{
    // 清理
    m_manager->clearCache();
    m_manager = nullptr;

    TestBase::cleanupTestCase();
}

void PasswordManagerTest::init()
{
    // 每个测试前的准备 - 清除缓存
    m_manager->clearCache();
    QCoreApplication::processEvents(); // 处理定时器删除
    m_manager->setStorageMode(PasswordManager::NoStorage);
}

void PasswordManagerTest::cleanup()
{
    // 每个测试后的清理
    m_manager->clearCache();
    QCoreApplication::processEvents(); // 处理定时器删除
}

// ========== 存储模式测试 ==========

void PasswordManagerTest::testSetStorageMode()
{
    // 测试设置存储模式

    // 测试不存储模式
    m_manager->setStorageMode(PasswordManager::NoStorage);
    QCOMPARE(m_manager->getStorageMode(), PasswordManager::NoStorage);

    // 测试会话缓存模式
    m_manager->setStorageMode(PasswordManager::SessionCache);
    QCOMPARE(m_manager->getStorageMode(), PasswordManager::SessionCache);

    // 测试加密存储模式
    m_manager->setStorageMode(PasswordManager::EncryptedStorage);
    QCOMPARE(m_manager->getStorageMode(), PasswordManager::EncryptedStorage);
}

void PasswordManagerTest::testNoStorageMode()
{
    // 测试不存储模式 - 密码不会被保存

    m_manager->setStorageMode(PasswordManager::NoStorage);

    int testRepoId = 1;
    QString testPassword = "TestPassword123";

    // 设置密码
    m_manager->setPassword(testRepoId, testPassword);

    // 尝试获取密码（不存储模式下应该失败）
    QString retrievedPassword;
    bool hasPassword = m_manager->getPassword(testRepoId, retrievedPassword);

    // 验证密码未被存储
    QVERIFY(!hasPassword);
}

void PasswordManagerTest::testSessionCacheMode()
{
    // 测试会话缓存模式 - 密码保存在内存中

    m_manager->setStorageMode(PasswordManager::SessionCache);

    int testRepoId = 1;
    QString testPassword = "TestPassword123";

    // 设置密码
    m_manager->setPassword(testRepoId, testPassword);

    // 获取密码
    QString retrievedPassword;
    bool hasPassword = m_manager->getPassword(testRepoId, retrievedPassword);

    // 验证密码已缓存
    QVERIFY(hasPassword);
    QCOMPARE(retrievedPassword, testPassword);
}

void PasswordManagerTest::testEncryptedStorageMode()
{
    // 测试加密存储模式 - 密码加密后存储在数据库

    m_manager->setStorageMode(PasswordManager::EncryptedStorage);

    // 先设置主密码
    m_manager->setMasterPassword("MasterPassword123");

    int testRepoId = 1;
    QString testPassword = "TestPassword123";

    // 设置密码
    m_manager->setPassword(testRepoId, testPassword);

    // 处理事件队列（定时器创建）
    QCoreApplication::processEvents();

    // 清除缓存（模拟应用重启）
    m_manager->clearCache();

    // 处理事件队列（定时器删除）
    QCoreApplication::processEvents();

    // 获取密码（应该从数据库加载并解密）
    QString retrievedPassword;
    bool hasPassword = m_manager->getPassword(testRepoId, retrievedPassword);

    // 处理事件队列（新定时器创建）
    QCoreApplication::processEvents();

    // 验证密码已从数据库恢复
    QVERIFY(hasPassword);
    QCOMPARE(retrievedPassword, testPassword);
}

// ========== 密码操作测试 ==========

void PasswordManagerTest::testSetPassword()
{
    // 测试设置密码

    m_manager->setStorageMode(PasswordManager::SessionCache);

    int testRepoId = 1;
    QString testPassword = "MySecretPassword";

    // 设置密码
    bool success = m_manager->setPassword(testRepoId, testPassword);

    // 验证设置成功
    QVERIFY(success);

    // 验证密码已缓存
    QVERIFY(m_manager->hasPassword(testRepoId));
}

void PasswordManagerTest::testGetPassword()
{
    // 测试获取密码

    m_manager->setStorageMode(PasswordManager::SessionCache);

    int testRepoId = 1;
    QString testPassword = "MySecretPassword";

    // 设置密码
    m_manager->setPassword(testRepoId, testPassword);

    // 获取密码
    QString retrievedPassword;
    bool hasPassword = m_manager->getPassword(testRepoId, retrievedPassword);

    // 验证
    QVERIFY(hasPassword);
    QCOMPARE(retrievedPassword, testPassword);
}

void PasswordManagerTest::testRemovePassword()
{
    // 测试删除密码

    m_manager->setStorageMode(PasswordManager::SessionCache);

    int testRepoId = 1;
    QString testPassword = "MySecretPassword";

    // 设置密码
    m_manager->setPassword(testRepoId, testPassword);
    QVERIFY(m_manager->hasPassword(testRepoId));

    // 删除密码
    m_manager->removePassword(testRepoId);

    // 验证密码已删除
    QVERIFY(!m_manager->hasPassword(testRepoId));
}

void PasswordManagerTest::testClearCache()
{
    // 测试清除所有密码缓存

    m_manager->setStorageMode(PasswordManager::SessionCache);

    // 设置多个密码
    m_manager->setPassword(1, "Password1");
    m_manager->setPassword(2, "Password2");
    m_manager->setPassword(3, "Password3");

    // 处理定时器创建
    QCoreApplication::processEvents();

    // 验证所有密码已缓存
    QVERIFY(m_manager->hasPassword(1));
    QVERIFY(m_manager->hasPassword(2));
    QVERIFY(m_manager->hasPassword(3));

    // 清除缓存
    m_manager->clearCache();

    // 处理定时器删除
    QCoreApplication::processEvents();

    // 验证所有密码已清除
    QVERIFY(!m_manager->hasPassword(1));
    QVERIFY(!m_manager->hasPassword(2));
    QVERIFY(!m_manager->hasPassword(3));
}

void PasswordManagerTest::testHasPassword()
{
    // 测试检查密码是否已缓存

    m_manager->setStorageMode(PasswordManager::SessionCache);

    int testRepoId = 1;

    // 初始状态 - 没有密码
    QVERIFY(!m_manager->hasPassword(testRepoId));

    // 设置密码
    m_manager->setPassword(testRepoId, "TestPassword");

    // 验证已有密码
    QVERIFY(m_manager->hasPassword(testRepoId));
}

// ========== 缓存超时测试 ==========

void PasswordManagerTest::testCacheTimeout()
{
    // 测试缓存超时设置

    // 设置超时时间为 5 分钟
    m_manager->setCacheTimeout(5);

    // 验证超时时间
    QCOMPARE(m_manager->getCacheTimeout(), 5);

    // 设置超时时间为 30 分钟
    m_manager->setCacheTimeout(30);

    // 验证超时时间
    QCOMPARE(m_manager->getCacheTimeout(), 30);
}

// ========== 主密码测试 ==========

void PasswordManagerTest::testSetMasterPassword()
{
    // 测试设置主密码

    QString masterPassword = "MyMasterPassword123";

    // 设置主密码
    m_manager->setMasterPassword(masterPassword);

    // 验证主密码已设置
    QVERIFY(m_manager->hasMasterPassword());
}

void PasswordManagerTest::testVerifyMasterPassword()
{
    // 测试验证主密码

    QString correctPassword = "CorrectMasterPassword";
    QString wrongPassword = "WrongPassword";

    // 设置主密码
    m_manager->setMasterPassword(correctPassword);

    // 验证正确的主密码
    QVERIFY(m_manager->verifyMasterPassword(correctPassword));

    // 验证错误的主密码
    QVERIFY(!m_manager->verifyMasterPassword(wrongPassword));
}

void PasswordManagerTest::testChangeMasterPassword()
{
    // 测试更改主密码

    QString oldPassword = "OldMasterPassword";
    QString newPassword = "NewMasterPassword";
    QString wrongPassword = "WrongPassword";

    // 设置主密码
    m_manager->setMasterPassword(oldPassword);

    // 使用错误的旧密码更改（应该失败）
    bool failedChange = m_manager->changeMasterPassword(wrongPassword, newPassword);
    QVERIFY(!failedChange);

    // 使用正确的旧密码更改（应该成功）
    bool successChange = m_manager->changeMasterPassword(oldPassword, newPassword);
    QVERIFY(successChange);

    // 验证新密码有效
    QVERIFY(m_manager->verifyMasterPassword(newPassword));

    // 验证旧密码无效
    QVERIFY(!m_manager->verifyMasterPassword(oldPassword));
}

// ========== 信号测试 ==========

void PasswordManagerTest::testPasswordRequiredSignal()
{
    // 测试密码需要信号

    PasswordManager* manager = PasswordManager::instance();

    // 创建信号监听器
    QSignalSpy spy(manager, &PasswordManager::passwordRequired);

    // 验证信号有效
    QVERIFY(spy.isValid());
}

} // namespace Test
} // namespace ResticGUI
