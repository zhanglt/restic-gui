/**
 * @file main.cpp
 * @brief 测试主函数 - 运行所有 Qt Test 测试用例
 *
 * 这个文件使用 Qt Test 框架的 QTEST_MAIN 或自定义 main 函数
 * 来运行所有注册的测试用例。
 */

#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

// 包含所有测试类头文件
#include "common/TestBase.h"

// 单元测试 - 核心管理器
#include "unit/core/ResticWrapperTest.h"
#include "unit/core/RepositoryManagerTest.h"
#include "unit/core/BackupManagerTest.h"
#include "unit/core/RestoreManagerTest.h"
#include "unit/core/SnapshotManagerTest.h"
#include "unit/core/SchedulerManagerTest.h"

// 单元测试 - 数据访问层
#include "unit/data/DatabaseManagerTest.h"
#include "unit/data/ConfigManagerTest.h"
#include "unit/data/PasswordManagerTest.h"
#include "unit/data/CacheManagerTest.h"

// 单元测试 - 工具类
#include "unit/utils/LoggerTest.h"
#include "unit/utils/CryptoUtilTest.h"
#include "unit/utils/FileSystemUtilTest.h"
#include "unit/utils/NetworkUtilTest.h"

// 单元测试 - 数据模型
#include "unit/models/ModelsTest.h"

// UI 测试
#include "ui/MainWindowTest.h"
#include "ui/PagesTest.h"
#include "ui/DialogsTest.h"

// 集成测试
#include "integration/BackupFlowTest.h"
#include "integration/RestoreFlowTest.h"
#include "integration/RepositoryManagementTest.h"
#include "integration/SchedulerTest.h"

/**
 * @brief 运行单个测试类
 * @param testObject 测试对象指针
 * @param testName 测试名称（用于日志输出）
 * @return 测试是否通过（0 表示成功）
 */
int runTest(QObject* testObject, const QString& testName)
{
    qDebug() << "\n========================================";
    qDebug() << "Running:" << testName;
    qDebug() << "========================================";

    int result = QTest::qExec(testObject);

    if (result == 0) {
        qDebug() << "✓" << testName << "PASSED";
    } else {
        qDebug() << "✗" << testName << "FAILED";
    }

    delete testObject;
    return result;
}

/**
 * @brief 主函数 - 运行所有测试
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    qDebug() << "========================================";
    qDebug() << "Restic GUI - Test Suite";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "========================================";

    int failedTests = 0;
    int totalTests = 0;

    // ========== 单元测试 - 核心管理器 ==========
    qDebug() << "\n### UNIT TESTS - Core Managers ###\n";

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::ResticWrapperTest(), "ResticWrapperTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::RepositoryManagerTest(), "RepositoryManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::BackupManagerTest(), "BackupManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::RestoreManagerTest(), "RestoreManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::SnapshotManagerTest(), "SnapshotManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::SchedulerManagerTest(), "SchedulerManagerTest");

    // ========== 单元测试 - 数据访问层 ==========
    qDebug() << "\n### UNIT TESTS - Data Access Layer ###\n";

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::DatabaseManagerTest(), "DatabaseManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::ConfigManagerTest(), "ConfigManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::PasswordManagerTest(), "PasswordManagerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::CacheManagerTest(), "CacheManagerTest");

    // ========== 单元测试 - 工具类 ==========
    qDebug() << "\n### UNIT TESTS - Utilities ###\n";

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::LoggerTest(), "LoggerTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::CryptoUtilTest(), "CryptoUtilTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::FileSystemUtilTest(), "FileSystemUtilTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::NetworkUtilTest(), "NetworkUtilTest");

    // ========== 单元测试 - 数据模型 ==========
    qDebug() << "\n### UNIT TESTS - Data Models ###\n";

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::ModelsTest(), "ModelsTest");

    // ========== UI 测试 ==========
    qDebug() << "\n### UI TESTS ###\n";

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::MainWindowTest(), "MainWindowTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::PagesTest(), "PagesTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::DialogsTest(), "DialogsTest");

    // ========== 集成测试 ==========
    qDebug() << "\n### INTEGRATION TESTS ###\n";

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::BackupFlowTest(), "BackupFlowTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::RestoreFlowTest(), "RestoreFlowTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::RepositoryManagementTest(), "RepositoryManagementTest");

    totalTests++;
    failedTests += runTest(new ResticGUI::Test::SchedulerTest(), "SchedulerTest");

    // ========== 测试总结 ==========
    qDebug() << "\n========================================";
    qDebug() << "TEST SUMMARY";
    qDebug() << "========================================";
    qDebug() << "Total test suites:" << totalTests;
    qDebug() << "Passed:" << (totalTests - failedTests);
    qDebug() << "Failed:" << failedTests;

    if (failedTests == 0) {
        qDebug() << "\n✓ ALL TESTS PASSED!";
        qDebug() << "========================================\n";
        return 0;
    } else {
        qDebug() << "\n✗ SOME TESTS FAILED!";
        qDebug() << "========================================\n";
        return 1;
    }
}
