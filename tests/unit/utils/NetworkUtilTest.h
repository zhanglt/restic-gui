#ifndef NETWORKUTILTEST_H
#define NETWORKUTILTEST_H

#include "common/TestBase.h"
#include "utils/NetworkUtil.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief NetworkUtil 测试类
 */
class NetworkUtilTest : public TestBase
{
    Q_OBJECT

private slots:
    void testIsNetworkAvailable();
    void testIsHostReachableLocalhost();
    void testIsHostReachableInvalidHost();
};

} // namespace Test
} // namespace ResticGUI

#endif // NETWORKUTILTEST_H
