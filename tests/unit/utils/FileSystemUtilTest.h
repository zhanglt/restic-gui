#ifndef FILESYSTEMUTILTEST_H
#define FILESYSTEMUTILTEST_H

#include "common/TestBase.h"
#include "utils/FileSystemUtil.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief FileSystemUtil 测试类
 *
 * 测试文件系统工具类的所有功能：
 * - 目录创建
 * - 路径规范化
 * - 目录大小计算
 * - 可写权限检查
 */
class FileSystemUtilTest : public TestBase
{
    Q_OBJECT

private slots:
    // 目录创建测试
    void testEnsureDirectoryExists();
    void testEnsureDirectoryExistsNested();
    void testEnsureDirectoryExistsAlreadyExists();

    // 路径规范化测试
    void testNormalizePath();
    void testNormalizePathWithBackslashes();
    void testNormalizePathWithDots();
    void testNormalizePathTrailingSlash();

    // 目录大小测试
    void testGetDirectorySize();
    void testGetDirectorySizeEmpty();
    void testGetDirectorySizeNonexistent();

    // 可写权限测试
    void testIsWritable();
    void testIsWritableReadOnly();
};

} // namespace Test
} // namespace ResticGUI

#endif // FILESYSTEMUTILTEST_H
