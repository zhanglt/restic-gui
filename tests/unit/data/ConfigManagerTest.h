#ifndef CONFIGMANAGERTEST_H
#define CONFIGMANAGERTEST_H

#include "common/TestBase.h"
#include "data/ConfigManager.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief ConfigManager 测试类
 *
 * 测试配置管理器的功能：
 * - 通用配置读写
 * - 应用程序设置
 * - 备份设置
 * - 密码设置
 * - 网络设置
 * - UI设置
 * - 默认值处理
 * - 配置持久化
 * - 信号机制
 */
class ConfigManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 通用配置测试
    void testGetValue();
    void testSetValue();
    void testDefaultValue();
    void testContains();
    void testRemove();
    void testSync();

    // 应用程序设置测试
    void testResticPath();
    void testLanguage();
    void testTheme();
    void testAutoStart();
    void testMinimizeToTray();

    // 备份设置测试
    void testDefaultExcludePatterns();
    void testBackupNotifications();
    void testMaxParallelBackups();

    // 密码设置测试
    void testPasswordStorageMode();
    void testPasswordCacheTimeout();

    // 网络设置测试
    void testProxySettings();
    void testNetworkTimeout();

    // UI设置测试
    void testWindowGeometry();
    void testWindowState();
    void testLastSelectedRepository();

    // 信号测试
    void testConfigChangedSignal();

private:
    Data::ConfigManager* m_manager;
};

} // namespace Test
} // namespace ResticGUI

#endif // CONFIGMANAGERTEST_H
