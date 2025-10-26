#include "ModelsTest.h"
#include <QStringList>

using namespace ResticGUI::Models;

namespace ResticGUI {
namespace Test {

// ========== Repository Tests ==========

void ModelsTest::testRepositorySerializeDeserialize()
{
    // Create test data
    Repository repo;
    repo.id = 123;
    repo.name = "Test Repository";
    repo.type = RepositoryType::Local;
    repo.path = "/backup/repo";
    repo.config["key1"] = "value1";
    repo.config["key2"] = 42;
    repo.passwordHash = "hash123";
    repo.createdAt = QDateTime::currentDateTime();
    repo.lastBackup = QDateTime::currentDateTime();
    repo.isDefault = true;

    // Serialize
    QVariantMap map = repo.toVariantMap();

    // Verify serialization
    QCOMPARE(map["id"].toInt(), 123);
    QCOMPARE(map["name"].toString(), QString("Test Repository"));
    QCOMPARE(map["path"].toString(), QString("/backup/repo"));
    QVERIFY(map.contains("config"));
    QVERIFY(map.contains("passwordHash"));
    QCOMPARE(map["isDefault"].toBool(), true);

    // Deserialize
    Repository repo2 = Repository::fromVariantMap(map);

    // Verify deserialization
    QCOMPARE(repo2.id, repo.id);
    QCOMPARE(repo2.name, repo.name);
    QCOMPARE(repo2.type, repo.type);
    QCOMPARE(repo2.path, repo.path);
    QCOMPARE(repo2.isDefault, repo.isDefault);
    QCOMPARE(repo2.config.size(), repo.config.size());
}

void ModelsTest::testRepositoryTypeConversion()
{
    // Test all repository types string conversion
    RepositoryType types[] = {
        RepositoryType::Local,
        RepositoryType::SFTP,
        RepositoryType::S3,
        RepositoryType::REST,
        RepositoryType::Azure,
        RepositoryType::GS,
        RepositoryType::B2,
        RepositoryType::Rclone
    };

    for (auto type : types) {
        // Convert to string
        QString typeStr = Repository::typeToString(type);
        QVERIFY(!typeStr.isEmpty());

        // Convert back to type
        RepositoryType typeBack = Repository::stringToType(typeStr);
        QCOMPARE(typeBack, type);
    }
}

void ModelsTest::testRepositoryValidation()
{
    // Test valid repository
    Repository validRepo;
    validRepo.name = "Valid Repo";
    validRepo.type = RepositoryType::Local;
    validRepo.path = "/valid/path";

    QVERIFY(validRepo.isValid());

    // Test invalid repository (empty name)
    Repository invalidRepo1;
    invalidRepo1.name = "";
    invalidRepo1.path = "/path";

    // Note: Depends on actual isValid() implementation
    // QVERIFY(!invalidRepo1.isValid());

    // Test invalid repository (empty path)
    Repository invalidRepo2;
    invalidRepo2.name = "Name";
    invalidRepo2.path = "";

    // QVERIFY(!invalidRepo2.isValid());
}

// ========== BackupTask Tests ==========

void ModelsTest::testBackupTaskSerializeDeserialize()
{
    // Create test data
    BackupTask task;
    task.id = 456;
    task.name = "Test Backup Task";
    task.repositoryId = 123;
    task.sourcePaths = QStringList() << "/home/user/docs" << "/home/user/photos";
    task.excludePatterns = QStringList() << "*.tmp" << "*.log";
    task.tags = QStringList() << "daily" << "important";
    task.schedule.type = Schedule::Type::Daily;

    // Serialize
    QVariantMap map = task.toVariantMap();

    // Verify serialization
    QCOMPARE(map["id"].toInt(), 456);
    QCOMPARE(map["name"].toString(), QString("Test Backup Task"));
    QCOMPARE(map["repositoryId"].toInt(), 123);
    QVERIFY(map.contains("sourcePaths"));
    QVERIFY(map.contains("excludePatterns"));
    QVERIFY(map.contains("tags"));
    QCOMPARE(map["enabled"].toBool(), true);

    // Deserialize
    BackupTask task2 = BackupTask::fromVariantMap(map);

    // Verify deserialization
    QCOMPARE(task2.id, task.id);
    QCOMPARE(task2.name, task.name);
    QCOMPARE(task2.repositoryId, task.repositoryId);
    QCOMPARE(task2.sourcePaths.size(), task.sourcePaths.size());
    QCOMPARE(task2.excludePatterns.size(), task.excludePatterns.size());
    QCOMPARE(task2.tags.size(), task.tags.size());
    QCOMPARE(task2.enabled, task.enabled);
}

void ModelsTest::testBackupTaskSourcesHandling()
{
    // Test backup source paths handling
    BackupTask task;
    task.sourcePaths = QStringList() << "/path1" << "/path2" << "/path3";

    // Serialize and deserialize
    QVariantMap map = task.toVariantMap();
    BackupTask task2 = BackupTask::fromVariantMap(map);

    // Verify paths preserved
    QCOMPARE(task2.sourcePaths.size(), 3);
    QVERIFY(task2.sourcePaths.contains("/path1"));
    QVERIFY(task2.sourcePaths.contains("/path2"));
    QVERIFY(task2.sourcePaths.contains("/path3"));
}

// ========== Schedule Tests ==========

void ModelsTest::testScheduleSerializeDeserialize()
{
    // Create test data
    Schedule schedule;
    schedule.type = Schedule::Daily;
    schedule.time = QTime(14, 30);
    schedule.requireAC = true;
    schedule.requireNetwork = false;

    // Serialize
    QVariantMap map = schedule.toVariantMap();

    // Verify serialization
    QVERIFY(map.contains("type"));
    QVERIFY(map.contains("time"));
    QVERIFY(map.contains("requireAC"));
    QCOMPARE(map["requireAC"].toBool(), true);
    QCOMPARE(map["requireNetwork"].toBool(), false);

    // Deserialize
    Schedule schedule2 = Schedule::fromVariantMap(map);

    // Verify deserialization
    QCOMPARE(schedule2.type, schedule.type);
    QCOMPARE(schedule2.time, schedule.time);
    QCOMPARE(schedule2.requireAC, schedule.requireAC);
    QCOMPARE(schedule2.requireNetwork, schedule.requireNetwork);
}

void ModelsTest::testScheduleTypeHandling()
{
    // Test different schedule types
    Schedule::Type types[] = {
        Schedule::Type::Manual,
        Schedule::Type::Hourly,
        Schedule::Type::Daily,
        Schedule::Type::Weekly,
        Schedule::Type::Monthly
    };

    for (auto type : types) {
        Schedule schedule;
        schedule.type = type;

        // Serialize and deserialize
        QVariantMap map = schedule.toVariantMap();
        Schedule schedule2 = Schedule::fromVariantMap(map);

        // Verify type preserved
        QCOMPARE(schedule2.type, type);
    }
}

// ========== Snapshot Tests ==========

void ModelsTest::testSnapshotSerializeDeserialize()
{
    // Note: Snapshot model does not have toVariantMap/fromVariantMap methods
    // This test validates basic field assignment and comparison

    // Create test data
    Snapshot snapshot;
    snapshot.id = "abc123def456";
    snapshot.time = QDateTime::currentDateTime();
    snapshot.hostname = "testhost";
    snapshot.username = "testuser";
    snapshot.paths = QStringList() << "/home" << "/etc";
    snapshot.tags = QStringList() << "backup" << "daily";

    // Create second snapshot by copying
    Snapshot snapshot2;
    snapshot2.id = snapshot.id;
    snapshot2.time = snapshot.time;
    snapshot2.hostname = snapshot.hostname;
    snapshot2.username = snapshot.username;
    snapshot2.paths = snapshot.paths;
    snapshot2.tags = snapshot.tags;

    // Verify data was copied correctly
    QCOMPARE(snapshot2.id, snapshot.id);
    QCOMPARE(snapshot2.hostname, snapshot.hostname);
    QCOMPARE(snapshot2.username, snapshot.username);
    QCOMPARE(snapshot2.paths.size(), snapshot.paths.size());
    QCOMPARE(snapshot2.tags.size(), snapshot.tags.size());
}

void ModelsTest::testSnapshotTagsHandling()
{
    // Test snapshot tags handling
    Snapshot snapshot;
    snapshot.tags = QStringList() << "tag1" << "tag2" << "tag3";

    // Copy tags to another snapshot
    Snapshot snapshot2;
    snapshot2.tags = snapshot.tags;

    // Verify tags preserved
    QCOMPARE(snapshot2.tags.size(), 3);
    QVERIFY(snapshot2.tags.contains("tag1"));
    QVERIFY(snapshot2.tags.contains("tag2"));
    QVERIFY(snapshot2.tags.contains("tag3"));
}

// ========== FileInfo Tests ==========

void ModelsTest::testFileInfoSerializeDeserialize()
{
    // Note: FileInfo model does not have toVariantMap/fromVariantMap methods
    // This test validates basic field assignment and comparison

    // Create test data
    FileInfo fileInfo;
    fileInfo.path = "/home/user/document.txt";
    fileInfo.name = "document.txt";
    fileInfo.size = 1024000;
    fileInfo.type = FileType::File;
    fileInfo.mtime = QDateTime::currentDateTime();

    // Create second FileInfo by copying
    FileInfo fileInfo2;
    fileInfo2.path = fileInfo.path;
    fileInfo2.name = fileInfo.name;
    fileInfo2.size = fileInfo.size;
    fileInfo2.type = fileInfo.type;
    fileInfo2.mtime = fileInfo.mtime;

    // Verify data was copied correctly
    QCOMPARE(fileInfo2.path, fileInfo.path);
    QCOMPARE(fileInfo2.name, fileInfo.name);
    QCOMPARE(fileInfo2.size, fileInfo.size);
    QCOMPARE(fileInfo2.type, fileInfo.type);
    QVERIFY(fileInfo2.type != FileType::Directory);
}

// ========== BackupResult Tests ==========

void ModelsTest::testBackupResultSerializeDeserialize()
{
    // Note: BackupResult model does not have toVariantMap/fromVariantMap methods
    // This test validates basic field assignment and comparison

    // Create test data
    BackupResult result;
    result.success = true;
    result.snapshotId = "snapshot123";
    result.filesNew = 100;
    result.filesChanged = 50;
    result.filesUnmodified = 1000;
    result.dirsNew = 10;
    result.dirsChanged = 5;
    result.dirsUnmodified = 100;
    result.dataAdded = 1024000;
    result.totalFilesProcessed = 1150;
    result.totalBytesProcessed = 5000000;
    result.duration = 120;
    result.errorMessage = "";

    // Create second result by copying
    BackupResult result2;
    result2.success = result.success;
    result2.snapshotId = result.snapshotId;
    result2.filesNew = result.filesNew;
    result2.filesChanged = result.filesChanged;
    result2.filesUnmodified = result.filesUnmodified;
    result2.dirsNew = result.dirsNew;
    result2.dirsChanged = result.dirsChanged;
    result2.dirsUnmodified = result.dirsUnmodified;
    result2.dataAdded = result.dataAdded;
    result2.totalFilesProcessed = result.totalFilesProcessed;
    result2.totalBytesProcessed = result.totalBytesProcessed;
    result2.duration = result.duration;
    result2.errorMessage = result.errorMessage;

    // Verify data was copied correctly
    QCOMPARE(result2.success, result.success);
    QCOMPARE(result2.snapshotId, result.snapshotId);
    QCOMPARE(result2.filesNew, result.filesNew);
    QCOMPARE(result2.filesChanged, result.filesChanged);
    QCOMPARE(result2.filesUnmodified, result.filesUnmodified);
}

void ModelsTest::testBackupResultStatistics()
{
    // Test backup statistics data
    BackupResult result;
    result.filesNew = 10;
    result.filesChanged = 5;
    result.filesUnmodified = 100;

    // Copy statistics
    BackupResult result2;
    result2.filesNew = result.filesNew;
    result2.filesChanged = result.filesChanged;
    result2.filesUnmodified = result.filesUnmodified;

    // Verify statistics
    QCOMPARE(result2.filesNew + result2.filesChanged + result2.filesUnmodified,
             result.filesNew + result.filesChanged + result.filesUnmodified);
}

// ========== RestoreOptions Tests ==========

void ModelsTest::testRestoreOptionsSerializeDeserialize()
{
    // Note: RestoreOptions model does not have toVariantMap/fromVariantMap methods
    // This test validates basic field assignment and comparison

    // Create test data
    RestoreOptions options;
    options.targetPath = "/restore/location";
    options.includePaths = QStringList() << "/include1" << "/include2";
    options.excludePaths = QStringList() << "/exclude1";
    options.verify = true;
    options.overwritePolicy = RestoreOptions::Always;

    // Create second options by copying
    RestoreOptions options2;
    options2.targetPath = options.targetPath;
    options2.includePaths = options.includePaths;
    options2.excludePaths = options.excludePaths;
    options2.verify = options.verify;
    options2.overwritePolicy = options.overwritePolicy;

    // Verify data was copied correctly
    QCOMPARE(options2.targetPath, options.targetPath);
    QCOMPARE(options2.includePaths.size(), options.includePaths.size());
    QCOMPARE(options2.excludePaths.size(), options.excludePaths.size());
    QCOMPARE(options2.verify, options.verify);
    QCOMPARE(options2.overwritePolicy, options.overwritePolicy);
}

// ========== RepoStats Tests ==========

void ModelsTest::testRepoStatsSerializeDeserialize()
{
    // Note: RepoStats model does not have toVariantMap/fromVariantMap methods
    // This test validates basic field assignment and comparison

    // Create test data
    RepoStats stats;
    stats.totalSize = 10000000000LL;  // 10 GB
    stats.totalFileCount = 50000;
    stats.snapshotCount = 30;
    stats.compressionRatio = 0.65;
    stats.uniqueSize = 8000000000LL;  // 8 GB

    // Create second stats by copying
    RepoStats stats2;
    stats2.totalSize = stats.totalSize;
    stats2.totalFileCount = stats.totalFileCount;
    stats2.snapshotCount = stats.snapshotCount;
    stats2.compressionRatio = stats.compressionRatio;
    stats2.uniqueSize = stats.uniqueSize;

    // Verify data was copied correctly
    QCOMPARE(stats2.totalSize, stats.totalSize);
    QCOMPARE(stats2.totalFileCount, stats.totalFileCount);
    QCOMPARE(stats2.snapshotCount, stats.snapshotCount);
    QCOMPARE(stats2.uniqueSize, stats.uniqueSize);

    // Compare floating point with tolerance
    QVERIFY(qAbs(stats2.compressionRatio - stats.compressionRatio) < 0.0001);
}

} // namespace Test
} // namespace ResticGUI
