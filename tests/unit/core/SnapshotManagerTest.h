#ifndef SNAPSHOTMANAGERTEST_H
#define SNAPSHOTMANAGERTEST_H

#include "common/TestBase.h"
#include "core/SnapshotManager.h"
#include "core/RepositoryManager.h"
#include "data/DatabaseManager.h"
#include "models/Snapshot.h"
#include "models/FileInfo.h"
#include "models/Repository.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief SnapshotManager 测试类
 *
 * 测试快照管理器的功能：
 * - 快照列表获取
 * - 单个快照获取
 * - 快照删除
 * - 文件浏览
 * - 快照比较
 * - 信号机制
 * - 缓存机制
 */
class SnapshotManagerTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 快照操作测试
    void testListSnapshots();
    void testListSnapshotsForceRefresh();
    void testGetSnapshot();
    void testDeleteSnapshots();

    // 文件浏览测试
    void testListFiles();
    void testListFilesInSubdirectory();

    // 快照比较测试
    void testCompareSnapshots();

    // 信号测试
    void testSnapshotsUpdatedSignal();
    void testSnapshotDeletedSignal();

private:
    Core::SnapshotManager* m_manager;
    Core::RepositoryManager* m_repoManager;
    Data::DatabaseManager* m_dbManager;
    QString m_testDbPath;
    QString m_testRepoPath;
    QString m_testPassword;

    // 辅助方法
    Models::Repository createTestRepository();
    Models::Snapshot createTestSnapshot(const QString& id);
};

} // namespace Test
} // namespace ResticGUI

#endif // SNAPSHOTMANAGERTEST_H
