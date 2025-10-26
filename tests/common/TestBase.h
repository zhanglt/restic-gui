#ifndef TESTBASE_H
#define TESTBASE_H

#include <QtTest>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QSqlDatabase>

namespace ResticGUI {
namespace Test {

/**
 * @brief 测试基类，提供通用的测试工具和设施
 *
 * 所有测试类应继承此类以获得：
 * - 临时目录管理
 * - 测试数据库设置
 * - 通用断言辅助方法
 * - 资源清理
 */
class TestBase : public QObject
{
    Q_OBJECT

public:
    explicit TestBase(QObject *parent = nullptr);
    virtual ~TestBase();

protected slots:
    /**
     * @brief 在所有测试用例执行前调用一次
     */
    virtual void initTestCase();

    /**
     * @brief 在所有测试用例执行后调用一次
     */
    virtual void cleanupTestCase();

    /**
     * @brief 在每个测试函数执行前调用
     */
    virtual void init();

    /**
     * @brief 在每个测试函数执行后调用
     */
    virtual void cleanup();

protected:
    // ========== 临时目录管理 ==========

    /**
     * @brief 获取临时测试目录路径
     * @return 临时目录的绝对路径
     */
    QString tempDir() const { return m_tempDir ? m_tempDir->path() : QString(); }

    /**
     * @brief 在临时目录中创建子目录
     * @param subPath 子目录路径（相对于临时目录）
     * @return 创建的目录的绝对路径，失败返回空字符串
     */
    QString createTempSubDir(const QString& subPath);

    /**
     * @brief 在临时目录中创建测试文件
     * @param fileName 文件名（相对于临时目录）
     * @param content 文件内容
     * @return 创建的文件的绝对路径，失败返回空字符串
     */
    QString createTempFile(const QString& fileName, const QString& content = QString());

    // ========== 测试数据库管理 ==========

    /**
     * @brief 创建测试用的 SQLite 数据库
     * @param connectionName 数据库连接名称
     * @return 数据库连接，失败时返回无效连接
     */
    QSqlDatabase createTestDatabase(const QString& connectionName = "test_connection");

    /**
     * @brief 关闭并删除测试数据库
     * @param connectionName 数据库连接名称
     */
    void closeTestDatabase(const QString& connectionName = "test_connection");

    // ========== 断言辅助方法 ==========

    /**
     * @brief 验证文件是否存在
     * @param filePath 文件路径
     * @param message 失败时的消息
     */
    void verifyFileExists(const QString& filePath, const QString& message = QString());

    /**
     * @brief 验证文件是否不存在
     * @param filePath 文件路径
     * @param message 失败时的消息
     */
    void verifyFileNotExists(const QString& filePath, const QString& message = QString());

    /**
     * @brief 验证目录是否存在
     * @param dirPath 目录路径
     * @param message 失败时的消息
     */
    void verifyDirExists(const QString& dirPath, const QString& message = QString());

    /**
     * @brief 等待信号触发（带超时）
     * @param sender 信号发送者
     * @param signal 信号
     * @param timeout 超时时间（毫秒），默认 5000ms
     * @return 是否在超时前接收到信号
     */
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);

    /**
     * @brief 比较两个 QVariantMap 是否相等
     * @param expected 期望的 map
     * @param actual 实际的 map
     * @param message 失败时的消息
     */
    void compareVariantMaps(const QVariantMap& expected, const QVariantMap& actual,
                           const QString& message = QString());

private:
    QTemporaryDir* m_tempDir;        ///< 临时测试目录
    QStringList m_testDatabases;     ///< 测试数据库连接名称列表
};

} // namespace Test
} // namespace ResticGUI

#endif // TESTBASE_H
