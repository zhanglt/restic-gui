#include "ResticWrapperTest.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QCoreApplication>

using namespace ResticGUI::Core;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void ResticWrapperTest::initTestCase()
{
    TestBase::initTestCase();

    m_wrapper = new ResticWrapper();

    // 设置 restic 路径为项目 bin 目录
    QString resticPath = QCoreApplication::applicationDirPath() + "/restic.exe";
    qDebug() << "Setting restic path to:" << resticPath;
    m_wrapper->setResticPath(resticPath);

    m_testRepoPath = tempDir() + "/test_restic_repo";
    m_testPassword = "TestPassword123!";
    m_resticAvailable = isResticAvailable();

    if (!m_resticAvailable) {
        qWarning() << "Restic is not available. Some tests will be skipped.";
        qWarning() << "To run all tests, please install restic: https://restic.net/";
    } else {
        qDebug() << "Restic is available at:" << resticPath;
    }
}

void ResticWrapperTest::cleanupTestCase()
{
    delete m_wrapper;
    m_wrapper = nullptr;

    TestBase::cleanupTestCase();
}

// ========== Helper Methods ==========

bool ResticWrapperTest::isResticAvailable()
{
    // 使用 wrapper 来检查 restic 是否可用
    QString version = m_wrapper->getVersion();
    return !version.isEmpty();
}

Repository ResticWrapperTest::createTestRepository()
{
    Repository repo;
    repo.name = "Test Repository";
    repo.type = RepositoryType::Local;
    repo.path = m_testRepoPath;
    return repo;
}

// ========== Basic Functionality Tests ==========

void ResticWrapperTest::testResticPathValidation()
{
    // Test setting restic path
    QString testPath = QCoreApplication::applicationDirPath() + "/restic.exe";
    m_wrapper->setResticPath(testPath);

    // Verify call doesn't crash
    QVERIFY(true);
}

void ResticWrapperTest::testGetVersion()
{
    if (!m_resticAvailable) {
        QSKIP("Restic not available");
    }

    // Get restic version
    QString version = m_wrapper->getVersion();

    // Verify version is not empty
    QVERIFY(!version.isEmpty());

    // Verify version contains "restic"
    QVERIFY(version.toLower().contains("restic"));

    qDebug() << "Restic version:" << version;
}

void ResticWrapperTest::testResticNotFound()
{
    // Test with nonexistent restic binary
    ResticWrapper wrapper;
    wrapper.setResticPath("/nonexistent/path/to/restic");

    // Try to get version (should fail)
    QString version = wrapper.getVersion();

    // Verify failure indicated by empty string or error message
    QVERIFY(version.isEmpty() || version.contains("error", Qt::CaseInsensitive));
}

// ========== Repository Operations Tests ==========

void ResticWrapperTest::testInitRepository()
{
    if (!m_resticAvailable) {
        QSKIP("Restic not available");
    }

    // Create test repository
    Repository repo = createTestRepository();

    // Initialize repository
    bool success = m_wrapper->initRepository(repo, m_testPassword);

    // Verify success
    QVERIFY(success);

    // Verify repository directory exists
    verifyDirExists(m_testRepoPath, "Repository directory should be created");
}

void ResticWrapperTest::testRepositoryPasswordHandling()
{
    if (!m_resticAvailable) {
        QSKIP("Restic not available");
    }

    // Initialize repository first
    Repository repo = createTestRepository();
    m_wrapper->initRepository(repo, m_testPassword);

    // Try checking repository with wrong password
    bool checkWithWrongPassword = m_wrapper->checkRepository(repo, "WrongPassword");
    QVERIFY(!checkWithWrongPassword);

    // Try checking repository with correct password
    bool checkWithCorrectPassword = m_wrapper->checkRepository(repo, m_testPassword);
    QVERIFY(checkWithCorrectPassword);
}

// ========== JSON Parsing Tests ==========

void ResticWrapperTest::testJsonOutputParsing()
{
    // Test parsing JSON output (simulate restic JSON output)
    QString jsonLine = R"({"message_type":"summary","files_new":10,"files_changed":5,"files_unmodified":100,"data_added":1024000})";

    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonLine.toUtf8());
    QVERIFY(!doc.isNull());

    QJsonObject obj = doc.object();

    // Verify parsed values
    QCOMPARE(obj["message_type"].toString(), QString("summary"));
    QCOMPARE(obj["files_new"].toInt(), 10);
    QCOMPARE(obj["files_changed"].toInt(), 5);
    QCOMPARE(obj["files_unmodified"].toInt(), 100);
    QCOMPARE(obj["data_added"].toInt(), 1024000);
}

void ResticWrapperTest::testInvalidJsonHandling()
{
    // Test handling invalid JSON
    QString invalidJson = "This is not valid JSON {]";

    QJsonDocument doc = QJsonDocument::fromJson(invalidJson.toUtf8());

    // Should return null document
    QVERIFY(doc.isNull());
}

// ========== Signal Tests ==========

void ResticWrapperTest::testProgressSignals()
{
    // Test progress signals
    // Note: Requires actual restic operations to emit signals
    // Here we just verify signals are properly defined

    ResticWrapper wrapper;

    // Create signal spies
    QSignalSpy progressSpy(&wrapper, &ResticWrapper::progressUpdated);
    QSignalSpy outputSpy(&wrapper, &ResticWrapper::standardOutput);

    // Verify signals are valid
    QVERIFY(progressSpy.isValid());
    QVERIFY(outputSpy.isValid());
}

// ========== Error Handling Tests ==========

void ResticWrapperTest::testErrorHandling()
{
    if (!m_resticAvailable) {
        QSKIP("Restic not available");
    }

    // Test with nonexistent repository
    Repository nonexistentRepo;
    nonexistentRepo.name = "Nonexistent";
    nonexistentRepo.type = RepositoryType::Local;
    nonexistentRepo.path = "/nonexistent/repo/path";

    // Try checking nonexistent repository
    bool result = m_wrapper->checkRepository(nonexistentRepo, m_testPassword);

    // Should fail
    QVERIFY(!result);
}

void ResticWrapperTest::testCancelOperation()
{
    // Test operation cancellation
    ResticWrapper wrapper;

    // Try canceling (should not crash)
    wrapper.cancel();

    // Verify call doesn't crash
    QVERIFY(true);
}

// ========== Security Tests ==========

void ResticWrapperTest::testPasswordNotInCommandLine()
{
    // Test that passwords are not exposed in command line
    // This is important for security

    // Note: This is a conceptual test
    // In actual implementation, verify that ResticWrapper uses:
    // - Environment variable RESTIC_PASSWORD
    // - Or --password-file option
    // - Never passes password directly in command line

    if (!m_resticAvailable) {
        QSKIP("Restic not available");
    }

    Repository repo = createTestRepository();

    // Initialize repository (password should be handled securely)
    bool success = m_wrapper->initRepository(repo, m_testPassword);

    // Verify success
    QVERIFY(success);

    // Note: Actual security verification would require:
    // - Monitoring process command line arguments
    // - Verifying environment variables are used
    // - Or verifying password file is used
}

} // namespace Test
} // namespace ResticGUI
