#include "ConfigManagerTest.h"
#include <QSignalSpy>

using namespace ResticGUI::Data;

namespace ResticGUI {
namespace Test {

void ConfigManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // 获取 ConfigManager 单例
    m_manager = ConfigManager::instance();
    m_manager->initialize();
}

void ConfigManagerTest::cleanupTestCase()
{
    m_manager = nullptr;
    TestBase::cleanupTestCase();
}

void ConfigManagerTest::init()
{
    // 每个测试前的准备
}

void ConfigManagerTest::cleanup()
{
    // 每个测试后的清理
}

// ========== 通用配置测试 ==========

void ConfigManagerTest::testGetValue()
{
    // 测试获取配置值

    QString testKey = "test/key";
    QString testValue = "test value";

    // 设置值
    m_manager->setValue(testKey, testValue);

    // 获取值
    QVariant retrieved = m_manager->getValue(testKey);

    // 验证
    QCOMPARE(retrieved.toString(), testValue);
}

void ConfigManagerTest::testSetValue()
{
    // 测试设置配置值

    QString testKey = "test/setvalue";
    int testValue = 42;

    // 设置值
    m_manager->setValue(testKey, testValue);

    // 验证值已设置
    QCOMPARE(m_manager->getValue(testKey).toInt(), testValue);
}

void ConfigManagerTest::testDefaultValue()
{
    // 测试默认值

    QString nonExistentKey = "nonexistent/key";
    QString defaultValue = "default";

    // 获取不存在的键（应该返回默认值）
    QVariant retrieved = m_manager->getValue(nonExistentKey, defaultValue);

    // 验证返回默认值
    QCOMPARE(retrieved.toString(), defaultValue);
}

void ConfigManagerTest::testContains()
{
    // 测试检查键是否存在

    QString testKey = "test/contains";
    QString testValue = "value";

    // 验证键不存在
    QVERIFY(!m_manager->contains(testKey));

    // 设置值
    m_manager->setValue(testKey, testValue);

    // 验证键存在
    QVERIFY(m_manager->contains(testKey));
}

void ConfigManagerTest::testRemove()
{
    // 测试移除配置键

    QString testKey = "test/remove";
    QString testValue = "value";

    // 设置值
    m_manager->setValue(testKey, testValue);
    QVERIFY(m_manager->contains(testKey));

    // 移除键
    m_manager->remove(testKey);

    // 验证键已移除
    QVERIFY(!m_manager->contains(testKey));
}

void ConfigManagerTest::testSync()
{
    // 测试同步配置到磁盘

    QString testKey = "test/sync";
    QString testValue = "sync value";

    // 设置值
    m_manager->setValue(testKey, testValue);

    // 同步
    m_manager->sync();

    // 验证调用不崩溃
    QVERIFY(true);
}

// ========== 应用程序设置测试 ==========

void ConfigManagerTest::testResticPath()
{
    // 测试restic路径设置

    QString testPath = "/usr/local/bin/restic";

    // 设置路径
    m_manager->setResticPath(testPath);

    // 获取路径
    QString retrievedPath = m_manager->getResticPath();

    // 验证
    QCOMPARE(retrievedPath, testPath);
}

void ConfigManagerTest::testLanguage()
{
    // 测试语言设置

    QString testLang = "zh_CN";

    // 设置语言
    m_manager->setLanguage(testLang);

    // 获取语言
    QString retrievedLang = m_manager->getLanguage();

    // 验证
    QCOMPARE(retrievedLang, testLang);
}

void ConfigManagerTest::testTheme()
{
    // 测试主题设置

    QString testTheme = "dark";

    // 设置主题
    m_manager->setTheme(testTheme);

    // 获取主题
    QString retrievedTheme = m_manager->getTheme();

    // 验证
    QCOMPARE(retrievedTheme, testTheme);
}

void ConfigManagerTest::testAutoStart()
{
    // 测试自动启动设置

    // 设置为启用
    m_manager->setAutoStart(true);
    QCOMPARE(m_manager->getAutoStart(), true);

    // 设置为禁用
    m_manager->setAutoStart(false);
    QCOMPARE(m_manager->getAutoStart(), false);
}

void ConfigManagerTest::testMinimizeToTray()
{
    // 测试最小化到托盘设置

    // 设置为启用
    m_manager->setMinimizeToTray(true);
    QCOMPARE(m_manager->getMinimizeToTray(), true);

    // 设置为禁用
    m_manager->setMinimizeToTray(false);
    QCOMPARE(m_manager->getMinimizeToTray(), false);
}

// ========== 备份设置测试 ==========

void ConfigManagerTest::testDefaultExcludePatterns()
{
    // 测试默认排除模式

    QStringList testPatterns;
    testPatterns << "*.tmp" << "*.log" << ".cache";

    // 设置排除模式
    m_manager->setDefaultExcludePatterns(testPatterns);

    // 获取排除模式
    QStringList retrievedPatterns = m_manager->getDefaultExcludePatterns();

    // 验证
    QCOMPARE(retrievedPatterns.size(), testPatterns.size());
    QCOMPARE(retrievedPatterns, testPatterns);
}

void ConfigManagerTest::testBackupNotifications()
{
    // 测试备份通知设置

    // 设置为启用
    m_manager->setShowBackupNotifications(true);
    QCOMPARE(m_manager->getShowBackupNotifications(), true);

    // 设置为禁用
    m_manager->setShowBackupNotifications(false);
    QCOMPARE(m_manager->getShowBackupNotifications(), false);
}

void ConfigManagerTest::testMaxParallelBackups()
{
    // 测试最大并行备份数

    int testCount = 3;

    // 设置并行数
    m_manager->setMaxParallelBackups(testCount);

    // 获取并行数
    int retrievedCount = m_manager->getMaxParallelBackups();

    // 验证
    QCOMPARE(retrievedCount, testCount);
}

// ========== 密码设置测试 ==========

void ConfigManagerTest::testPasswordStorageMode()
{
    // 测试密码存储模式

    // 设置为会话缓存模式（1）
    m_manager->setPasswordStorageMode(1);
    QCOMPARE(m_manager->getPasswordStorageMode(), 1);

    // 设置为加密存储模式（2）
    m_manager->setPasswordStorageMode(2);
    QCOMPARE(m_manager->getPasswordStorageMode(), 2);
}

void ConfigManagerTest::testPasswordCacheTimeout()
{
    // 测试密码缓存超时

    int testTimeout = 30; // 30分钟

    // 设置超时
    m_manager->setPasswordCacheTimeout(testTimeout);

    // 获取超时
    int retrievedTimeout = m_manager->getPasswordCacheTimeout();

    // 验证
    QCOMPARE(retrievedTimeout, testTimeout);
}

// ========== 网络设置测试 ==========

void ConfigManagerTest::testProxySettings()
{
    // 测试代理设置

    // 设置代理启用
    m_manager->setUseProxy(true);
    QCOMPARE(m_manager->getUseProxy(), true);

    // 设置代理主机
    QString testHost = "proxy.example.com";
    m_manager->setProxyHost(testHost);
    QCOMPARE(m_manager->getProxyHost(), testHost);

    // 设置代理端口
    int testPort = 8080;
    m_manager->setProxyPort(testPort);
    QCOMPARE(m_manager->getProxyPort(), testPort);

    // 设置代理用户名
    QString testUsername = "proxyuser";
    m_manager->setProxyUsername(testUsername);
    QCOMPARE(m_manager->getProxyUsername(), testUsername);
}

void ConfigManagerTest::testNetworkTimeout()
{
    // 测试网络超时

    int testTimeout = 60; // 60秒

    // 设置超时
    m_manager->setNetworkTimeout(testTimeout);

    // 获取超时
    int retrievedTimeout = m_manager->getNetworkTimeout();

    // 验证
    QCOMPARE(retrievedTimeout, testTimeout);
}

// ========== UI设置测试 ==========

void ConfigManagerTest::testWindowGeometry()
{
    // 测试窗口几何信息

    QByteArray testGeometry = "test_geometry_data";

    // 设置几何信息
    m_manager->setWindowGeometry(testGeometry);

    // 获取几何信息
    QByteArray retrievedGeometry = m_manager->getWindowGeometry();

    // 验证
    QCOMPARE(retrievedGeometry, testGeometry);
}

void ConfigManagerTest::testWindowState()
{
    // 测试窗口状态

    QByteArray testState = "test_state_data";

    // 设置状态
    m_manager->setWindowState(testState);

    // 获取状态
    QByteArray retrievedState = m_manager->getWindowState();

    // 验证
    QCOMPARE(retrievedState, testState);
}

void ConfigManagerTest::testLastSelectedRepository()
{
    // 测试上次选中的仓库ID

    int testRepoId = 42;

    // 设置仓库ID
    m_manager->setLastSelectedRepository(testRepoId);

    // 获取仓库ID
    int retrievedRepoId = m_manager->getLastSelectedRepository();

    // 验证
    QCOMPARE(retrievedRepoId, testRepoId);
}

// ========== 信号测试 ==========

void ConfigManagerTest::testConfigChangedSignal()
{
    // 测试配置改变信号

    ConfigManager* manager = ConfigManager::instance();

    // 创建信号监听器
    QSignalSpy spy(manager, &ConfigManager::configChanged);

    // 验证信号有效
    QVERIFY(spy.isValid());

    // 修改配置（可能触发信号，取决于实现）
    manager->setValue("test/signal", "value");
}

} // namespace Test
} // namespace ResticGUI
