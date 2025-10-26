#include "TestBase.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace ResticGUI {
namespace Test {

TestBase::TestBase(QObject *parent)
    : QObject(parent)
    , m_tempDir(nullptr)
{
}

TestBase::~TestBase()
{
    // 清理所有测试数据库
    for (const QString& connName : m_testDatabases) {
        closeTestDatabase(connName);
    }

    // 清理临时目录
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
}

void TestBase::initTestCase()
{
    // 创建临时测试目录
    m_tempDir = new QTemporaryDir();
    QVERIFY2(m_tempDir->isValid(), "Failed to create temporary test directory");

    qDebug() << "Test temporary directory:" << m_tempDir->path();
}

void TestBase::cleanupTestCase()
{
    // 清理所有测试数据库
    for (const QString& connName : m_testDatabases) {
        closeTestDatabase(connName);
    }
    m_testDatabases.clear();

    // 清理临时目录
    if (m_tempDir) {
        qDebug() << "Cleaning up temporary directory:" << m_tempDir->path();
        delete m_tempDir;
        m_tempDir = nullptr;
    }
}

void TestBase::init()
{
    // 每个测试函数执行前的初始化
    // 子类可以重写此方法
}

void TestBase::cleanup()
{
    // 每个测试函数执行后的清理
    // 子类可以重写此方法
}

// ========== 临时目录管理 ==========

QString TestBase::createTempSubDir(const QString& subPath)
{
    if (!m_tempDir || !m_tempDir->isValid()) {
        qWarning() << "Temporary directory is not valid";
        return QString();
    }

    QString fullPath = m_tempDir->path() + "/" + subPath;
    QDir dir;
    if (!dir.mkpath(fullPath)) {
        qWarning() << "Failed to create subdirectory:" << fullPath;
        return QString();
    }

    return fullPath;
}

QString TestBase::createTempFile(const QString& fileName, const QString& content)
{
    if (!m_tempDir || !m_tempDir->isValid()) {
        qWarning() << "Temporary directory is not valid";
        return QString();
    }

    QString filePath = m_tempDir->path() + "/" + fileName;

    // 确保父目录存在
    QFileInfo fileInfo(filePath);
    QDir dir;
    if (!dir.mkpath(fileInfo.absolutePath())) {
        qWarning() << "Failed to create parent directory for:" << filePath;
        return QString();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create file:" << filePath;
        return QString();
    }

    if (!content.isEmpty()) {
        file.write(content.toUtf8());
    }

    file.close();
    return filePath;
}

// ========== 测试数据库管理 ==========

QSqlDatabase TestBase::createTestDatabase(const QString& connectionName)
{
    // 如果连接已存在，先关闭
    if (QSqlDatabase::contains(connectionName)) {
        closeTestDatabase(connectionName);
    }

    // 创建内存数据库（SQLite :memory: 数据库）
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(":memory:");

    if (!db.open()) {
        qWarning() << "Failed to open test database:" << db.lastError().text();
        return QSqlDatabase();
    }

    m_testDatabases.append(connectionName);
    qDebug() << "Created test database:" << connectionName;

    return db;
}

void TestBase::closeTestDatabase(const QString& connectionName)
{
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen()) {
            db.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
        qDebug() << "Closed test database:" << connectionName;
    }

    m_testDatabases.removeAll(connectionName);
}

// ========== 断言辅助方法 ==========

void TestBase::verifyFileExists(const QString& filePath, const QString& message)
{
    QFileInfo fileInfo(filePath);
    QString msg = message.isEmpty() ?
                      QString("File should exist: %1").arg(filePath) : message;
    QVERIFY2(fileInfo.exists() && fileInfo.isFile(), msg.toUtf8().constData());
}

void TestBase::verifyFileNotExists(const QString& filePath, const QString& message)
{
    QFileInfo fileInfo(filePath);
    QString msg = message.isEmpty() ?
                      QString("File should not exist: %1").arg(filePath) : message;
    QVERIFY2(!fileInfo.exists(), msg.toUtf8().constData());
}

void TestBase::verifyDirExists(const QString& dirPath, const QString& message)
{
    QDir dir(dirPath);
    QString msg = message.isEmpty() ?
                      QString("Directory should exist: %1").arg(dirPath) : message;
    QVERIFY2(dir.exists(), msg.toUtf8().constData());
}

bool TestBase::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    if (!sender) {
        qWarning() << "Sender object is null";
        return false;
    }

    QSignalSpy spy(sender, signal);
    if (!spy.isValid()) {
        qWarning() << "Invalid signal:" << signal;
        return false;
    }

    // 等待信号触发
    bool received = spy.wait(timeout);

    if (!received) {
        qWarning() << "Signal not received within timeout:" << timeout << "ms";
    }

    return received;
}

void TestBase::compareVariantMaps(const QVariantMap& expected, const QVariantMap& actual,
                                 const QString& message)
{
    // 检查键数量
    QString msg = message.isEmpty() ? "VariantMap key count mismatch" : message;
    QCOMPARE(actual.keys().size(), expected.keys().size());

    // 逐个比较键值对
    for (auto it = expected.constBegin(); it != expected.constEnd(); ++it) {
        QString key = it.key();
        QVERIFY2(actual.contains(key),
                 QString("Missing key: %1").arg(key).toUtf8().constData());

        QVariant expectedValue = it.value();
        QVariant actualValue = actual.value(key);

        // 处理不同类型的值
        if (expectedValue.type() == QVariant::Double || actualValue.type() == QVariant::Double) {
            // 浮点数比较（带容差）
            QVERIFY2(qAbs(expectedValue.toDouble() - actualValue.toDouble()) < 0.0001,
                     QString("Value mismatch for key '%1': expected %2, got %3")
                         .arg(key)
                         .arg(expectedValue.toDouble())
                         .arg(actualValue.toDouble())
                         .toUtf8().constData());
        } else if (expectedValue.type() == QVariant::Map) {
            // 递归比较嵌套 Map
            compareVariantMaps(expectedValue.toMap(), actualValue.toMap(),
                             QString("Nested map mismatch for key '%1'").arg(key));
        } else {
            // 其他类型直接比较
            QCOMPARE(actualValue, expectedValue);
        }
    }
}

} // namespace Test
} // namespace ResticGUI
