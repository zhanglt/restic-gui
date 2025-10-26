#ifndef PASSWORDMANAGERTEST_H
#define PASSWORDMANAGERTEST_H

#include "common/TestBase.h"
#include "data/PasswordManager.h"
#include "data/DatabaseManager.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief PasswordManager 测试类
 *
 * 测试密码管理器的功能：
 * - 存储模式（不存储、会话缓存、加密存储）
 * - 密码操作（获取、设置、删除、清除缓存）
 * - 缓存超时机制
 * - 主密码管理
 * - 信号机制
 */
class PasswordManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 存储模式测试
    void testSetStorageMode();
    void testNoStorageMode();
    void testSessionCacheMode();
    void testEncryptedStorageMode();

    // 密码操作测试
    void testSetPassword();
    void testGetPassword();
    void testRemovePassword();
    void testClearCache();
    void testHasPassword();

    // 缓存超时测试
    void testCacheTimeout();

    // 主密码测试
    void testSetMasterPassword();
    void testVerifyMasterPassword();
    void testChangeMasterPassword();

    // 信号测试
    void testPasswordRequiredSignal();

private:
    Data::PasswordManager* m_manager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
};

} // namespace Test
} // namespace ResticGUI

#endif // PASSWORDMANAGERTEST_H
