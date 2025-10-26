#include "LoggerTest.h"
#include <QSignalSpy>

using namespace ResticGUI::Utils;

namespace ResticGUI {
namespace Test {

void LoggerTest::testSingleton()
{
    // 测试单例模式
    Logger* logger1 = Logger::instance();
    Logger* logger2 = Logger::instance();

    QVERIFY(logger1 != nullptr);
    QCOMPARE(logger1, logger2);  // 应该是同一个实例
}

void LoggerTest::testLogLevels()
{
    // 测试不同日志级别
    Logger* logger = Logger::instance();

    // 设置为 Warning 级别
    logger->setLevel(Logger::Warning);

    QSignalSpy spy(logger, &Logger::logMessage);

    // Debug 和 Info 不应该触发信号
    logger->debug("Debug message");
    logger->info("Info message");

    // Warning, Error, Critical 应该触发信号
    logger->warning("Warning message");
    logger->error("Error message");
    logger->critical("Critical message");

    // 应该收到 3 个信号（Warning, Error, Critical）
    QCOMPARE(spy.count(), 3);
}

void LoggerTest::testLogToFile()
{
    // 测试日志写入文件
    Logger* logger = Logger::instance();

    QString logFile = tempDir() + "/test.log";
    logger->setLogFile(logFile);

    logger->info("Test log message");

    // 验证日志文件已创建
    verifyFileExists(logFile);
}

void LoggerTest::testLogSignals()
{
    // 测试日志信号
    Logger* logger = Logger::instance();

    QSignalSpy spy(logger, &Logger::logMessage);

    logger->info("Test message");

    // 应该收到信号
    QVERIFY(spy.count() > 0);

    // 验证信号参数
    QList<QVariant> arguments = spy.takeFirst();
    int level = arguments.at(0).toInt();
    QString message = arguments.at(1).toString();

    QCOMPARE(level, static_cast<int>(Logger::Info));
    QVERIFY(message.contains("Test message"));
}

} // namespace Test
} // namespace ResticGUI
