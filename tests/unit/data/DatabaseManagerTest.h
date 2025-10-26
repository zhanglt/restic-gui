#ifndef DATABASEMANAGERTEST_H
#define DATABASEMANAGERTEST_H

#include "common/TestBase.h"
#include "data/DatabaseManager.h"
#include "models/Repository.h"
#include "models/BackupTask.h"
#include "models/BackupResult.h"
#include "models/Snapshot.h"

namespace ResticGUI {
namespace Test {

class DatabaseManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    void testDatabaseInitialization();
    void testDatabaseReinitialization();

    void testInsertRepository();
    void testUpdateRepository();
    void testDeleteRepository();
    void testGetRepository();
    void testGetAllRepositories();
    void testDefaultRepository();

    void testInsertBackupTask();
    void testUpdateBackupTask();
    void testDeleteBackupTask();
    void testGetBackupTask();
    void testGetAllBackupTasks();
    void testGetBackupTasksByRepository();
    void testGetEnabledBackupTasks();

    void testInsertBackupHistory();
    void testGetBackupHistory();
    void testGetRecentBackupHistory();

    void testCacheSnapshots();
    void testGetCachedSnapshots();
    void testClearSnapshotCache();

    void testGetSetSetting();
    void testSettingDefaultValue();

    void testStoreAndGetPassword();
    void testDeletePassword();

    void testTransactionCommit();
    void testTransactionRollback();

    void testForeignKeyConstraints();
    void testDataPersistence();

private:
    QString m_testDbPath;
    Data::DatabaseManager* m_dbManager;
};

}

}


#endif

