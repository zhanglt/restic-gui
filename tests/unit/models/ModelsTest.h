#ifndef MODELSTEST_H
#define MODELSTEST_H

#include "common/TestBase.h"
#include "models/Repository.h"
#include "models/BackupTask.h"
#include "models/Schedule.h"
#include "models/Snapshot.h"
#include "models/FileInfo.h"
#include "models/BackupResult.h"
#include "models/RestoreOptions.h"
#include "models/RepoStats.h"

namespace ResticGUI {
namespace Test {

class ModelsTest : public TestBase
{
    Q_OBJECT

private slots:
    void testRepositorySerializeDeserialize();
    void testRepositoryTypeConversion();
    void testRepositoryValidation();

    void testBackupTaskSerializeDeserialize();
    void testBackupTaskSourcesHandling();

    void testScheduleSerializeDeserialize();
    void testScheduleTypeHandling();

    void testSnapshotSerializeDeserialize();
    void testSnapshotTagsHandling();

    void testFileInfoSerializeDeserialize();

    void testBackupResultSerializeDeserialize();
    void testBackupResultStatistics();

    void testRestoreOptionsSerializeDeserialize();

    void testRepoStatsSerializeDeserialize();
};

}

}


#endif

