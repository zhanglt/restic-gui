#ifndef PAGESTEST_H
#define PAGESTEST_H

#include "common/TestBase.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief Pages 测试类
 *
 * 测试各个页面的功能：
 * - HomePage - 首页显示
 * - RepositoryPage - 仓库管理页
 * - BackupPage - 备份任务页
 * - SnapshotPage - 快照浏览页
 * - RestorePage - 恢复页
 * - StatsPage - 统计页
 */
class PagesTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 首页测试
    void testHomePageCreation();

    // 仓库页测试
    void testRepositoryPageCreation();

    // 备份页测试
    void testBackupPageCreation();

    // 快照页测试
    void testSnapshotPageCreation();

    // 恢复页测试
    void testRestorePageCreation();

    // 统计页测试
    void testStatsPageCreation();
};

} // namespace Test
} // namespace ResticGUI

#endif // PAGESTEST_H
