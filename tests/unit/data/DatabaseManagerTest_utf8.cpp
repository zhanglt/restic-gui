#include "DatabaseManagerTest.h"
#include <QFile>
#include <QDir>

using namespace ResticGUI::Data;
using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

void DatabaseManagerTest::initTestCase()
{
    TestBase::initTestCase();

    // Kpn
    m_testDbPath = tempDir() + "/test_database.db";
    m_dbManager = DatabaseManager::instance();
}

void DatabaseManagerTest::cleanupTestCase()
{
    // pn
    if (QFile::exists(m_testDbPath)) {
        QFile::remove(m_testDbPath);
    }

    TestBase::cleanupTestCase();
}

void DatabaseManagerTest::init()
{
    // *KM桶pn
    if (QFile::exists(m_testDbPath)) {
        QFile::remove(m_testDbPath);
    }

    bool success = m_dbManager->initialize(m_testDbPath);
    QVERIFY2(success, "Failed to initialize test database");
}

void DatabaseManagerTest::cleanup()
{
    // *K
}

// ========== pnK ==========

void DatabaseManagerTest::testDatabaseInitialization()
{
    // pn
    verifyFileExists(m_testDbPath, "Database file should exist after initialization");

    //  DatabaseManager 
    DatabaseManager* instance = DatabaseManager::instance();
    QVERIFY(instance != nullptr);
}

void DatabaseManagerTest::testDatabaseReinitialization()
{
    // ,!
)	
    bool success = m_dbManager->initialize(m_testDbPath);
    QVERIFY(success);
}

// ========== 
iconv: DatabaseManagerTest.cpp:66:14: cannot convert
