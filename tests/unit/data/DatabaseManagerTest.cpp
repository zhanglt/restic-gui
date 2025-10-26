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

    m_testDbPath = tempDir() + "/test_database.db";
    m_dbManager = DatabaseManager::instance();
}

void DatabaseManagerTest::cleanupTestCase()
{
    if (QFile::exists(m_testDbPath)) {
        QFile::remove(m_testDbPath);
    }

    TestBase::cleanupTestCase();
}

void DatabaseManagerTest::init()
{
    if (QFile::exists(m_testDbPath)) {
        QFile::remove(m_testDbPath);
    }

    bool success = m_dbManager->initialize(m_testDbPath);
    QVERIFY2(success, "Failed to initialize test database");
}

void DatabaseManagerTest::cleanup()
{
}


void DatabaseManagerTest::testDatabaseInitialization()
{
    verifyFileExists(m_testDbPath, "Database file should exist after initialization");

    DatabaseManager* instance = DatabaseManager::instance();
    QVERIFY(instance != nullptr);
}

void DatabaseManagerTest::testDatabaseReinitialization()
{
    bool success = m_dbManager->initialize(m_testDbPath);
    QVERIFY(success);
}


void DatabaseManagerTest::testInsertRepository()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/test/path";
    repo.isDefault = false;

    int id = m_dbManager->insertRepository(repo);

    QVERIFY(id > 0);

}

void DatabaseManagerTest::testUpdateRepository()
{
    Repository repo;
    repo.name = "Original Name";
    repo.type = RepositoryType::Local;
    repo.path = "/original/path";

    int id = m_dbManager->insertRepository(repo);
    QVERIFY(id > 0);

    repo.id = id;
    repo.name = "Updated Name";
    repo.path = "/updated/path";

    bool success = m_dbManager->updateRepository(repo);
    QVERIFY(success);

    Repository updated = m_dbManager->getRepository(id);
    QCOMPARE(updated.name, QString("Updated Name"));
    QCOMPARE(updated.path, QString("/updated/path"));
}

void DatabaseManagerTest::testDeleteRepository()
{
    Repository repo;
    repo.name = "To Delete";
    repo.type = RepositoryType::Local;
    repo.path = "/delete/me";

    int id = m_dbManager->insertRepository(repo);
    QVERIFY(id > 0);

    bool success = m_dbManager->deleteRepository(id);
    QVERIFY(success);

    Repository deleted = m_dbManager->getRepository(id);
    QVERIFY(deleted.id == -1 || deleted.name.isEmpty());
}

void DatabaseManagerTest::testGetRepository()
{
    Repository repo;
    repo.name = "Get Test";
    repo.type = RepositoryType::SFTP;
    repo.path = "user@host:/path";

    int id = m_dbManager->insertRepository(repo);

    Repository retrieved = m_dbManager->getRepository(id);

    QCOMPARE(retrieved.id, id);
    QCOMPARE(retrieved.name, repo.name);
    QCOMPARE(retrieved.type, repo.type);
    QCOMPARE(retrieved.path, repo.path);
}

void DatabaseManagerTest::testGetAllRepositories()
{
    for (int i = 0; i < 5; ++i) {
        Repository repo;
        repo.name = QString("Repo %1").arg(i);
        repo.type = RepositoryType::Local;
        repo.path = QString("/path/%1").arg(i);

        m_dbManager->insertRepository(repo);
    }

    QList<Repository> repos = m_dbManager->getAllRepositories();

    QCOMPARE(repos.size(), 5);
}

void DatabaseManagerTest::testDefaultRepository()
{
    Repository repo1;
    repo1.name = "Repo 1";
    repo1.type = RepositoryType::Local;
    repo1.path = "/path1";
    int id1 = m_dbManager->insertRepository(repo1);

    Repository repo2;
    repo2.name = "Repo 2";
    repo2.type = RepositoryType::Local;
    repo2.path = "/path2";
    int id2 = m_dbManager->insertRepository(repo2);

    bool success = m_dbManager->setDefaultRepository(id2);
    QVERIFY(success);

    Repository defaultRepo = m_dbManager->getDefaultRepository();
    QCOMPARE(defaultRepo.id, id2);
}


void DatabaseManagerTest::testInsertBackupTask()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "Test Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/source1" << "/source2";
    task.schedule.type = Schedule::Daily;

    int taskId = m_dbManager->insertBackupTask(task);
    QVERIFY(taskId > 0);
}

void DatabaseManagerTest::testUpdateBackupTask()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "Original Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/original";
    task.schedule.type = Schedule::Daily;

    int taskId = m_dbManager->insertBackupTask(task);

    task.id = taskId;
    task.name = "Updated Task";
    task.sourcePaths = QStringList() << "/updated1" << "/updated2";
    task.enabled = false;

    bool success = m_dbManager->updateBackupTask(task);
    QVERIFY(success);

    BackupTask updated = m_dbManager->getBackupTask(taskId);
    QCOMPARE(updated.name, QString("Updated Task"));
    QCOMPARE(updated.enabled, false);
    QCOMPARE(updated.sourcePaths.size(), 2);
}

void DatabaseManagerTest::testDeleteBackupTask()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "To Delete";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/source";

    int taskId = m_dbManager->insertBackupTask(task);

    bool success = m_dbManager->deleteBackupTask(taskId);
    QVERIFY(success);

    BackupTask deleted = m_dbManager->getBackupTask(taskId);
    QVERIFY(deleted.id == -1 || deleted.name.isEmpty());
}

void DatabaseManagerTest::testGetBackupTask()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "Get Test";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/source";
    task.excludePatterns = QStringList() << "*.tmp";
    task.tags = QStringList() << "test";

    int taskId = m_dbManager->insertBackupTask(task);

    BackupTask retrieved = m_dbManager->getBackupTask(taskId);

    QCOMPARE(retrieved.id, taskId);
    QCOMPARE(retrieved.name, task.name);
    QCOMPARE(retrieved.repositoryId, repoId);
}

void DatabaseManagerTest::testGetAllBackupTasks()
{

    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);


    for (int i = 0; i < 3; ++i) {
        BackupTask task;
        task.name = QString("Task %1").arg(i);
        task.repositoryId = repoId;
        task.sourcePaths = QStringList() << QString("/source%1").arg(i);

        m_dbManager->insertBackupTask(task);
    }


    QList<BackupTask> tasks = m_dbManager->getAllBackupTasks();
    QCOMPARE(tasks.size(), 3);
}

void DatabaseManagerTest::testGetBackupTasksByRepository()
{

    Repository repo1;
    repo1.name = "Repo 1";
    repo1.type = RepositoryType::Local;
    repo1.path = "/repo1";
    int repoId1 = m_dbManager->insertRepository(repo1);

    Repository repo2;
    repo2.name = "Repo 2";
    repo2.type = RepositoryType::Local;
    repo2.path = "/repo2";
    int repoId2 = m_dbManager->insertRepository(repo2);

    for (int i = 0; i < 2; ++i) {
        BackupTask task;
        task.name = QString("Repo1 Task %1").arg(i);
        task.repositoryId = repoId1;
        task.sourcePaths = QStringList() << "/source";
        m_dbManager->insertBackupTask(task);
    }

    BackupTask task;
    task.name = "Repo2 Task";
    task.repositoryId = repoId2;
    task.sourcePaths = QStringList() << "/source";
    m_dbManager->insertBackupTask(task);

    QList<BackupTask> repo1Tasks = m_dbManager->getBackupTasksByRepository(repoId1);
    QCOMPARE(repo1Tasks.size(), 2);

    QList<BackupTask> repo2Tasks = m_dbManager->getBackupTasksByRepository(repoId2);
    QCOMPARE(repo2Tasks.size(), 1);
}

void DatabaseManagerTest::testGetEnabledBackupTasks()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    for (int i = 0; i < 5; ++i) {
        BackupTask task;
        task.name = QString("Task %1").arg(i);
        task.repositoryId = repoId;
        task.sourcePaths = QStringList() << "/source";
        task.enabled = (i % 2 == 0);


        m_dbManager->insertBackupTask(task);
    }

    QList<BackupTask> enabledTasks = m_dbManager->getEnabledBackupTasks();


    QCOMPARE(enabledTasks.size(), 3);

    for (const BackupTask& task : enabledTasks) {
        QVERIFY(task.enabled);
    }
}


void DatabaseManagerTest::testInsertBackupHistory()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "Test Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/source";
    int taskId = m_dbManager->insertBackupTask(task);

    BackupResult result;
    result.taskId = taskId;
    result.success = true;
    result.snapshotId = "snapshot123";
    result.filesNew = 100;
    result.filesChanged = 50;
    result.dataAdded = 1024000;
    result.startTime = QDateTime::currentDateTime();
    result.endTime = QDateTime::currentDateTime();

    int historyId = m_dbManager->insertBackupHistory(result);
    QVERIFY(historyId > 0);
}

void DatabaseManagerTest::testGetBackupHistory()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "Test Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/source";
    int taskId = m_dbManager->insertBackupTask(task);


    for (int i = 0; i < 5; ++i) {
        BackupResult result;
        result.taskId = taskId;
        result.success = true;
        result.snapshotId = QString("snapshot%1").arg(i);
        result.startTime = QDateTime::currentDateTime();
        result.endTime = QDateTime::currentDateTime();

        m_dbManager->insertBackupHistory(result);
    }


    QList<BackupResult> history = m_dbManager->getBackupHistory(taskId);
    QCOMPARE(history.size(), 5);
}

void DatabaseManagerTest::testGetRecentBackupHistory()
{

    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);


    for (int i = 0; i < 3; ++i) {
        BackupTask task;
        task.name = QString("Task %1").arg(i);
        task.repositoryId = repoId;
        task.sourcePaths = QStringList() << "/source";
        int taskId = m_dbManager->insertBackupTask(task);

        for (int j = 0; j < 2; ++j) {
            BackupResult result;
            result.taskId = taskId;
            result.success = true;
            result.snapshotId = QString("snapshot_%1_%2").arg(i).arg(j);
            result.startTime = QDateTime::currentDateTime();
            result.endTime = QDateTime::currentDateTime();

            m_dbManager->insertBackupHistory(result);
        }
    }

    QList<BackupResult> recent = m_dbManager->getRecentBackupHistory(10);


    QVERIFY(recent.size() <= 10);
    QVERIFY(recent.size() >= 6);
}


void DatabaseManagerTest::testCacheSnapshots()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    QList<Snapshot> snapshots;
    for (int i = 0; i < 3; ++i) {
        Snapshot snapshot;
        snapshot.id = QString("snapshot%1").arg(i);
        snapshot.id = QString("snap%1").arg(i);
        snapshot.time = QDateTime::currentDateTime();
        snapshot.hostname = "testhost";

        snapshots.append(snapshot);
    }

    bool success = m_dbManager->cacheSnapshots(repoId, snapshots);
    QVERIFY(success);
}

void DatabaseManagerTest::testGetCachedSnapshots()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    QList<Snapshot> snapshots;
    for (int i = 0; i < 3; ++i) {
        Snapshot snapshot;
        snapshot.id = QString("snapshot%1").arg(i);
        snapshot.id = QString("snap%1").arg(i);
        snapshot.time = QDateTime::currentDateTime();

        snapshots.append(snapshot);
    }

    m_dbManager->cacheSnapshots(repoId, snapshots);

    QList<Snapshot> cached = m_dbManager->getCachedSnapshots(repoId);
    QCOMPARE(cached.size(), 3);
}

void DatabaseManagerTest::testClearSnapshotCache()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    QList<Snapshot> snapshots;
    Snapshot snapshot;
    snapshot.id = "snapshot1";
    snapshots.append(snapshot);

    m_dbManager->cacheSnapshots(repoId, snapshots);

    bool success = m_dbManager->clearSnapshotCache(repoId);
    QVERIFY(success);

    QList<Snapshot> cached = m_dbManager->getCachedSnapshots(repoId);
    QCOMPARE(cached.size(), 0);
}


void DatabaseManagerTest::testGetSetSetting()
{
    QString key = "test_setting";
    QString value = "test_value";

    bool success = m_dbManager->setSetting(key, value);
    QVERIFY(success);

    QString retrieved = m_dbManager->getSetting(key);
    QCOMPARE(retrieved, value);
}

void DatabaseManagerTest::testSettingDefaultValue()
{
    QString key = "nonexistent_key";
    QString defaultValue = "default";

    QString retrieved = m_dbManager->getSetting(key, defaultValue);
    QCOMPARE(retrieved, defaultValue);
}


void DatabaseManagerTest::testStoreAndGetPassword()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    QString encryptedPassword = "encrypted_password_string";

    bool success = m_dbManager->storePassword(repoId, encryptedPassword);
    QVERIFY(success);

    QString retrieved = m_dbManager->getStoredPassword(repoId);
    QCOMPARE(retrieved, encryptedPassword);
}

void DatabaseManagerTest::testDeletePassword()
{
    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    m_dbManager->storePassword(repoId, "password");

    bool success = m_dbManager->deleteStoredPassword(repoId);
    QVERIFY(success);

    QString retrieved = m_dbManager->getStoredPassword(repoId);
    QVERIFY(retrieved.isEmpty());
}


void DatabaseManagerTest::testTransactionCommit()
{
    bool beginSuccess = m_dbManager->beginTransaction();
    QVERIFY(beginSuccess);

    Repository repo;
    repo.name = "Transaction Test";
    repo.type = RepositoryType::Local;
    repo.path = "/transaction";

    int id = m_dbManager->insertRepository(repo);
    QVERIFY(id > 0);

    bool commitSuccess = m_dbManager->commit();
    QVERIFY(commitSuccess);

    Repository retrieved = m_dbManager->getRepository(id);
    QCOMPARE(retrieved.name, QString("Transaction Test"));
}

void DatabaseManagerTest::testTransactionRollback()
{
    m_dbManager->beginTransaction();

    Repository repo;
    repo.name = "Rollback Test";
    repo.type = RepositoryType::Local;
    repo.path = "/rollback";

    int id = m_dbManager->insertRepository(repo);
    QVERIFY(id > 0);

    bool rollbackSuccess = m_dbManager->rollback();
    QVERIFY(rollbackSuccess);

}


void DatabaseManagerTest::testForeignKeyConstraints()
{

    Repository repo;
    repo.name = "Test Repo";
    repo.type = RepositoryType::Local;
    repo.path = "/repo";
    int repoId = m_dbManager->insertRepository(repo);

    BackupTask task;
    task.name = "Test Task";
    task.repositoryId = repoId;
    task.sourcePaths = QStringList() << "/source";
    int taskId = m_dbManager->insertBackupTask(task);

    QVERIFY(taskId > 0);

    bool deleteSuccess = m_dbManager->deleteRepository(repoId);

    if (deleteSuccess) {
        BackupTask retrieved = m_dbManager->getBackupTask(taskId);
        QVERIFY(retrieved.id == -1 || retrieved.name.isEmpty());
    } else {
        BackupTask retrieved = m_dbManager->getBackupTask(taskId);
        QVERIFY(retrieved.id > 0);
    }
}

void DatabaseManagerTest::testDataPersistence()
{
    Repository repo;
    repo.name = "Persistence Test";
    repo.type = RepositoryType::Local;
    repo.path = "/persist";

    int id = m_dbManager->insertRepository(repo);
    QVERIFY(id > 0);

    m_dbManager->initialize(m_testDbPath);

    Repository retrieved = m_dbManager->getRepository(id);
    QCOMPARE(retrieved.name, QString("Persistence Test"));
}

}

}

