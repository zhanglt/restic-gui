#ifndef BACKUPMANAGERTEST_H
#define BACKUPMANAGERTEST_H

#include "common/TestBase.h"
#include "core/BackupManager.h"
#include "data/DatabaseManager.h"
#include "models/BackupTask.h"
#include "models/Repository.h"

namespace ResticGUI {
namespace Test {

class BackupManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    void testCreateBackupTask();
    void testUpdateBackupTask();
    void testDeleteBackupTask();
    void testGetBackupTask();
    void testGetAllBackupTasks();
    void testGetTasksByRepository();

    void testRunBackupTask();
    void testCancelBackup();
    void testConcurrentBackupPrevention();

    void testGetBackupHistory();
    void testGetLastBackupResult();

    void testTaskCreatedSignal();
    void testTaskUpdatedSignal();
    void testTaskDeletedSignal();
    void testBackupSignals();

    void testInvalidTaskId();
    void testPasswordError();

private:
    Core::BackupManager* m_manager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;

    Models::Repository createTestRepository();
    Models::BackupTask createTestBackupTask(int repoId);
};

}

}


#endif

