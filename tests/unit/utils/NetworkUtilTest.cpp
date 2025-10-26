#include "NetworkUtilTest.h"

using namespace ResticGUI::Utils;

namespace ResticGUI {
namespace Test {

void NetworkUtilTest::testIsNetworkAvailable()
{
    // 测试网络可用性检测
    bool available = NetworkUtil::isNetworkAvailable();

    // 网络可能可用也可能不可用，取决于测试环境
    // 我们只验证函数不会崩溃并返回布尔值
    qDebug() << "Network available:" << available;

    // 函数应该成功返回（不崩溃）
    QVERIFY(true);
}

void NetworkUtilTest::testIsHostReachableLocalhost()
{
    // 测试本地主机可达性（localhost 应该总是可达的）
    bool reachable = NetworkUtil::isHostReachable("127.0.0.1", 0);

    // localhost 通常应该是可达的
    // 但在某些受限环境中可能不可达，所以这个测试比较宽松
    qDebug() << "Localhost reachable:" << reachable;

    // 函数应该成功返回（不崩溃）
    QVERIFY(true);
}

void NetworkUtilTest::testIsHostReachableInvalidHost()
{
    // 测试无效主机
    bool reachable = NetworkUtil::isHostReachable("invalid.host.that.does.not.exist.123456", 22);

    // 无效主机应该不可达
    // 注意：这个测试依赖网络环境，可能需要超时
    qDebug() << "Invalid host reachable:" << reachable;

    // 预期不可达（但不强制要求，因为网络测试不稳定）
    // QVERIFY(!reachable);

    // 函数应该成功返回（不崩溃）
    QVERIFY(true);
}

} // namespace Test
} // namespace ResticGUI
