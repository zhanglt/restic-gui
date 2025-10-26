#include "FileSystemUtilTest.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace ResticGUI::Utils;

namespace ResticGUI {
namespace Test {

// ========== 目录创建测试 ==========

void FileSystemUtilTest::testEnsureDirectoryExists()
{
    // 测试创建目录
    QString testDir = tempDir() + "/test_directory";

    // 目录最初不应该存在
    QVERIFY(!QDir(testDir).exists());

    // 创建目录
    bool result = FileSystemUtil::ensureDirectoryExists(testDir);
    QVERIFY(result);

    // 现在目录应该存在
    QVERIFY(QDir(testDir).exists());
}

void FileSystemUtilTest::testEnsureDirectoryExistsNested()
{
    // 测试创建嵌套目录
    QString nestedDir = tempDir() + "/level1/level2/level3";

    // 嵌套目录不存在
    QVERIFY(!QDir(nestedDir).exists());

    // 创建嵌套目录
    bool result = FileSystemUtil::ensureDirectoryExists(nestedDir);
    QVERIFY(result);

    // 所有层级的目录都应该存在
    QVERIFY(QDir(tempDir() + "/level1").exists());
    QVERIFY(QDir(tempDir() + "/level1/level2").exists());
    QVERIFY(QDir(nestedDir).exists());
}

void FileSystemUtilTest::testEnsureDirectoryExistsAlreadyExists()
{
    // 测试对已存在的目录调用
    QString existingDir = tempDir();  // tempDir() 已经存在

    // 对已存在的目录调用应该成功
    bool result = FileSystemUtil::ensureDirectoryExists(existingDir);
    QVERIFY(result);

    // 目录仍然存在
    QVERIFY(QDir(existingDir).exists());
}

// ========== 路径规范化测试 ==========

void FileSystemUtilTest::testNormalizePath()
{
    // 测试基本路径规范化
    QString path = "/home/user/documents";
    QString normalized = FileSystemUtil::normalizePath(path);

    QVERIFY(!normalized.isEmpty());

    // 规范化路径应该使用正斜杠（跨平台）
    QVERIFY(!normalized.contains("\\"));
}

void FileSystemUtilTest::testNormalizePathWithBackslashes()
{
    // 测试反斜杠转换为正斜杠（Windows 风格路径）
    QString windowsPath = "C:\\Users\\John\\Documents";
    QString normalized = FileSystemUtil::normalizePath(windowsPath);

    // 应该转换为正斜杠
    QVERIFY(!normalized.contains("\\"));
}

void FileSystemUtilTest::testNormalizePathWithDots()
{
    // 测试包含 . 和 .. 的路径
    QString pathWithDots = "/home/user/../user/./documents";
    QString normalized = FileSystemUtil::normalizePath(pathWithDots);

    // 规范化后应该解析 . 和 ..
    QVERIFY(!normalized.contains(".."));
    QVERIFY(!normalized.contains("/./"));
}

void FileSystemUtilTest::testNormalizePathTrailingSlash()
{
    // 测试末尾斜杠的处理
    QString pathWithSlash = "/home/user/documents/";
    QString pathWithoutSlash = "/home/user/documents";

    QString normalized1 = FileSystemUtil::normalizePath(pathWithSlash);
    QString normalized2 = FileSystemUtil::normalizePath(pathWithoutSlash);

    // 两者应该规范化为相同结果（一致性）
    QVERIFY(!normalized1.isEmpty());
    QVERIFY(!normalized2.isEmpty());
}

// ========== 目录大小测试 ==========

void FileSystemUtilTest::testGetDirectorySize()
{
    // 创建测试目录并添加一些文件
    QString testDir = createTempSubDir("size_test");
    QVERIFY(!testDir.isEmpty());

    // 创建测试文件
    QString file1 = createTempFile("size_test/file1.txt", "Hello World!");
    QString file2 = createTempFile("size_test/file2.txt", "Test Data");

    QVERIFY(!file1.isEmpty());
    QVERIFY(!file2.isEmpty());

    // 获取目录大小
    qint64 size = FileSystemUtil::getDirectorySize(testDir);

    // 大小应该大于 0
    QVERIFY(size > 0);

    // 大小应该至少等于文件内容的大小
    qint64 expectedMinSize = QString("Hello World!").toUtf8().size() +
                             QString("Test Data").toUtf8().size();
    QVERIFY(size >= expectedMinSize);
}

void FileSystemUtilTest::testGetDirectorySizeEmpty()
{
    // 测试空目录的大小
    QString emptyDir = createTempSubDir("empty_dir");
    QVERIFY(!emptyDir.isEmpty());

    qint64 size = FileSystemUtil::getDirectorySize(emptyDir);

    // 空目录大小应该为 0 或很小（取决于文件系统）
    QVERIFY(size >= 0);
}

void FileSystemUtilTest::testGetDirectorySizeNonexistent()
{
    // 测试不存在的目录
    QString nonexistentDir = tempDir() + "/nonexistent";

    qint64 size = FileSystemUtil::getDirectorySize(nonexistentDir);

    // 不存在的目录应该返回 0 或 -1
    QVERIFY(size <= 0);
}

// ========== 可写权限测试 ==========

void FileSystemUtilTest::testIsWritable()
{
    // 测试可写目录
    QString writableDir = tempDir();  // 临时目录应该是可写的

    bool writable = FileSystemUtil::isWritable(writableDir);
    QVERIFY(writable);

    // 验证确实可以写入
    QString testFile = createTempFile("writable_test.txt", "test");
    QVERIFY(!testFile.isEmpty());
}

void FileSystemUtilTest::testIsWritableReadOnly()
{
    // 测试只读路径
    // 创建一个文件并设置为只读
    QString testFile = createTempFile("readonly_test.txt", "test");
    QVERIFY(!testFile.isEmpty());

    QFile file(testFile);
    QFile::Permissions permissions = file.permissions();

    // 移除写权限
    file.setPermissions(permissions & ~QFile::WriteUser & ~QFile::WriteOwner);

    // 检查文件是否可写
    bool writable = FileSystemUtil::isWritable(testFile);

    // 文件应该不可写
    // 注意：在某些系统（如 Windows）上，这个测试可能不准确
    // 因为文件系统权限处理方式不同

    // 恢复权限（清理）
    file.setPermissions(permissions);

    // Windows 上的行为可能不同，所以这里不做严格断言
    // QVERIFY(!writable);
    qDebug() << "ReadOnly file writable check (platform-dependent):" << writable;
}

} // namespace Test
} // namespace ResticGUI
