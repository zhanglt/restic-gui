#ifndef LOGGERTEST_H
#define LOGGERTEST_H

#include "common/TestBase.h"
#include "utils/Logger.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief Logger 测试类
 */
class LoggerTest : public TestBase
{
    Q_OBJECT

private slots:
    void testSingleton();
    void testLogLevels();
    void testLogToFile();
    void testLogSignals();
};

} // namespace Test
} // namespace ResticGUI

#endif // LOGGERTEST_H
