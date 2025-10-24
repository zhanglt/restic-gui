/**
 * @file DatabaseManager.cpp
 * @brief 数据库管理器实现
 */

#include "DatabaseManager.h"
#include "../utils/Logger.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>

namespace ResticGUI {
namespace Data {

DatabaseManager* DatabaseManager::s_instance = nullptr;
QMutex DatabaseManager::s_instanceMutex;

DatabaseManager* DatabaseManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new DatabaseManager();
        }
    }
    return s_instance;
}

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    QMutexLocker locker(&m_mutex);

    m_databasePath = dbPath;

    // 确保数据库目录存在
    QFileInfo dbFileInfo(dbPath);
    QDir dbDir = dbFileInfo.absoluteDir();
    if (!dbDir.exists()) {
        dbDir.mkpath(".");
    }

    // 连接数据库
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("无法打开数据库: %1").arg(m_lastError));
        emit databaseError(m_lastError);
        return false;
    }

    // 初始化或升级数据库架构
    if (!initializeSchema()) {
        return false;
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("数据库初始化成功: %1").arg(dbPath));
    return true;
}

bool DatabaseManager::initializeSchema()
{
    // 检查数据库是否已初始化
    QSqlQuery query(m_database);
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='settings'");

    if (!query.next()) {
        // 数据库未初始化，执行初始化脚本
        QString scriptPath = ":/sql/init_database.sql";

        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("尝试从资源加载SQL脚本: %1").arg(scriptPath));

        // 如果资源文件不存在，尝试从文件系统读取
        if (!QFile::exists(scriptPath)) {
            scriptPath = QDir::currentPath() + "/resources/sql/init_database.sql";
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("资源文件不存在，尝试文件系统: %1").arg(scriptPath));
        }

        if (!executeSqlScript(scriptPath)) {
            m_lastError = QString("无法执行数据库初始化脚本: %1").arg(m_lastError);
            Utils::Logger::instance()->log(Utils::Logger::Error, m_lastError);
            return false;
        }

        Utils::Logger::instance()->log(Utils::Logger::Info, "数据库架构初始化成功");
    } else {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "数据库已存在，跳过初始化");
    }

    // 检查并升级数据库版本
    return checkAndUpgradeSchema();
}

bool DatabaseManager::checkAndUpgradeSchema()
{
    QSqlQuery query(m_database);
    query.prepare("SELECT version FROM schema_version ORDER BY version DESC LIMIT 1");

    if (!query.exec() || !query.next()) {
        // 如果无法读取版本，可能是旧的数据库格式，设置为版本0
        Utils::Logger::instance()->log(Utils::Logger::Warning,
            "无法读取数据库版本，假定为版本1");
        m_schemaVersion = 1;
        return true;
    }

    int currentVersion = query.value(0).toInt();
    m_schemaVersion = currentVersion;

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("当前数据库版本: %1").arg(currentVersion));

    // 升级到版本 2：添加 last_run 和 next_run 字段
    if (currentVersion < 2) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "升级数据库到版本2：添加备份任务时间字段");

        QSqlQuery upgradeQuery(m_database);

        // 检查字段是否已存在
        upgradeQuery.exec("PRAGMA table_info(backup_tasks)");
        bool hasLastRun = false;
        bool hasNextRun = false;

        while (upgradeQuery.next()) {
            QString columnName = upgradeQuery.value(1).toString();
            if (columnName == "last_run") hasLastRun = true;
            if (columnName == "next_run") hasNextRun = true;
        }

        // 添加缺失的字段
        if (!hasLastRun) {
            if (!upgradeQuery.exec("ALTER TABLE backup_tasks ADD COLUMN last_run TEXT")) {
                Utils::Logger::instance()->log(Utils::Logger::Error,
                    QString("添加 last_run 字段失败: %1").arg(upgradeQuery.lastError().text()));
                return false;
            }
            Utils::Logger::instance()->log(Utils::Logger::Info, "已添加 last_run 字段");
        }

        if (!hasNextRun) {
            if (!upgradeQuery.exec("ALTER TABLE backup_tasks ADD COLUMN next_run TEXT")) {
                Utils::Logger::instance()->log(Utils::Logger::Error,
                    QString("添加 next_run 字段失败: %1").arg(upgradeQuery.lastError().text()));
                return false;
            }
            Utils::Logger::instance()->log(Utils::Logger::Info, "已添加 next_run 字段");
        }

        // 更新版本号
        upgradeQuery.exec("INSERT OR REPLACE INTO schema_version (version, applied_at) VALUES (2, datetime('now'))");
        m_schemaVersion = 2;
        Utils::Logger::instance()->log(Utils::Logger::Info, "数据库已升级到版本2");
    }

    return true;
}

bool DatabaseManager::executeSqlScript(const QString& scriptPath)
{
    QFile file(scriptPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开SQL脚本: %1").arg(scriptPath);
        Utils::Logger::instance()->log(Utils::Logger::Error, m_lastError);
        return false;
    }

    QString sqlScript = file.readAll();
    file.close();

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("读取SQL脚本成功，大小: %1 字节").arg(sqlScript.length()));

    // 分割SQL语句（简单实现，按分号分割）
    QStringList statements = sqlScript.split(';', Qt::SkipEmptyParts);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("共 %1 条SQL语句待执行").arg(statements.size()));

    QSqlQuery query(m_database);
    int successCount = 0;
    for (const QString& statement : statements) {
        QString trimmed = statement.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        if (!query.exec(trimmed)) {
            m_lastError = QString("SQL执行失败: %1\n语句: %2")
                .arg(query.lastError().text())
                .arg(trimmed.left(100));
            Utils::Logger::instance()->log(Utils::Logger::Error, m_lastError);
            return false;
        }
        successCount++;
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("成功执行 %1 条SQL语句").arg(successCount));

    return true;
}

bool DatabaseManager::executeQuery(QSqlQuery& query)
{
    QMutexLocker locker(&m_mutex);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("查询执行失败: %1").arg(m_lastError));
        return false;
    }
    return true;
}

bool DatabaseManager::beginTransaction()
{
    QMutexLocker locker(&m_mutex);
    return m_database.transaction();
}

bool DatabaseManager::commit()
{
    QMutexLocker locker(&m_mutex);
    return m_database.commit();
}

bool DatabaseManager::rollback()
{
    QMutexLocker locker(&m_mutex);
    return m_database.rollback();
}

// ========== 仓库表操作 ==========

int DatabaseManager::insertRepository(const Models::Repository& repo)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO repositories (name, type, path, config, password_hash, created_at, last_backup, is_default) "
        "VALUES (:name, :type, :path, :config, :password_hash, :created_at, :last_backup, :is_default)"
    );

    query.bindValue(":name", repo.name);
    query.bindValue(":type", Models::Repository::typeToString(repo.type));
    query.bindValue(":path", repo.path);

    // 将config转换为JSON字符串
    QJsonDocument configDoc = QJsonDocument::fromVariant(repo.config);
    query.bindValue(":config", QString::fromUtf8(configDoc.toJson(QJsonDocument::Compact)));

    query.bindValue(":password_hash", repo.passwordHash);
    query.bindValue(":created_at", repo.createdAt.toString(Qt::ISODate));
    query.bindValue(":last_backup", repo.lastBackup.toString(Qt::ISODate));
    query.bindValue(":is_default", repo.isDefault ? 1 : 0);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("插入仓库失败: %1").arg(m_lastError));
        return -1;
    }

    return query.lastInsertId().toInt();
}

bool DatabaseManager::updateRepository(const Models::Repository& repo)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare(
        "UPDATE repositories SET name=:name, type=:type, path=:path, config=:config, "
        "password_hash=:password_hash, last_backup=:last_backup, is_default=:is_default "
        "WHERE id=:id"
    );

    query.bindValue(":id", repo.id);
    query.bindValue(":name", repo.name);
    query.bindValue(":type", Models::Repository::typeToString(repo.type));
    query.bindValue(":path", repo.path);

    QJsonDocument configDoc = QJsonDocument::fromVariant(repo.config);
    query.bindValue(":config", QString::fromUtf8(configDoc.toJson(QJsonDocument::Compact)));

    query.bindValue(":password_hash", repo.passwordHash);
    query.bindValue(":last_backup", repo.lastBackup.toString(Qt::ISODate));
    query.bindValue(":is_default", repo.isDefault ? 1 : 0);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteRepository(int id)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM repositories WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

Models::Repository DatabaseManager::getRepository(int id)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM repositories WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        return Models::Repository();
    }

    Models::Repository repo;
    repo.id = query.value("id").toInt();
    repo.name = query.value("name").toString();
    repo.type = Models::Repository::stringToType(query.value("type").toString());
    repo.path = query.value("path").toString();

    // 解析JSON配置
    QString configJson = query.value("config").toString();
    QJsonDocument configDoc = QJsonDocument::fromJson(configJson.toUtf8());
    repo.config = configDoc.toVariant().toMap();

    repo.passwordHash = query.value("password_hash").toString();
    repo.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    repo.lastBackup = QDateTime::fromString(query.value("last_backup").toString(), Qt::ISODate);
    repo.isDefault = query.value("is_default").toInt() == 1;

    return repo;
}

QList<Models::Repository> DatabaseManager::getAllRepositories()
{
    QMutexLocker locker(&m_mutex);

    QList<Models::Repository> repositories;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM repositories ORDER BY is_default DESC, name ASC");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return repositories;
    }

    while (query.next()) {
        Models::Repository repo;
        repo.id = query.value("id").toInt();
        repo.name = query.value("name").toString();
        repo.type = Models::Repository::stringToType(query.value("type").toString());
        repo.path = query.value("path").toString();

        QString configJson = query.value("config").toString();
        QJsonDocument configDoc = QJsonDocument::fromJson(configJson.toUtf8());
        repo.config = configDoc.toVariant().toMap();

        repo.passwordHash = query.value("password_hash").toString();
        repo.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        repo.lastBackup = QDateTime::fromString(query.value("last_backup").toString(), Qt::ISODate);
        repo.isDefault = query.value("is_default").toInt() == 1;

        repositories.append(repo);
    }

    return repositories;
}

Models::Repository DatabaseManager::getDefaultRepository()
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM repositories WHERE is_default=1 LIMIT 1");

    if (!query.exec() || !query.next()) {
        return Models::Repository();
    }

    Models::Repository repo;
    repo.id = query.value("id").toInt();
    repo.name = query.value("name").toString();
    repo.type = Models::Repository::stringToType(query.value("type").toString());
    repo.path = query.value("path").toString();

    QString configJson = query.value("config").toString();
    QJsonDocument configDoc = QJsonDocument::fromJson(configJson.toUtf8());
    repo.config = configDoc.toVariant().toMap();

    repo.passwordHash = query.value("password_hash").toString();
    repo.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    repo.lastBackup = QDateTime::fromString(query.value("last_backup").toString(), Qt::ISODate);
    repo.isDefault = true;

    return repo;
}

bool DatabaseManager::setDefaultRepository(int id)
{
    QMutexLocker locker(&m_mutex);

    // 开始事务
    if (!m_database.transaction()) {
        return false;
    }

    // 清除所有默认标记
    QSqlQuery query1(m_database);
    query1.prepare("UPDATE repositories SET is_default=0");
    if (!query1.exec()) {
        m_database.rollback();
        return false;
    }

    // 设置新的默认仓库
    QSqlQuery query2(m_database);
    query2.prepare("UPDATE repositories SET is_default=1 WHERE id=:id");
    query2.bindValue(":id", id);
    if (!query2.exec()) {
        m_database.rollback();
        return false;
    }

    return m_database.commit();
}

// ========== 备份任务表操作 ==========

int DatabaseManager::insertBackupTask(const Models::BackupTask& task)
{
    QMutexLocker locker(&m_mutex);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("开始插入备份任务: name=%1, repoId=%2").arg(task.name).arg(task.repositoryId));

    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO backup_tasks (name, description, repository_id, source_paths, exclude_patterns, "
        "tags, hostname, options, schedule_type, schedule_config, enabled, created_at, updated_at) "
        "VALUES (:name, :description, :repository_id, :source_paths, :exclude_patterns, "
        ":tags, :hostname, :options, :schedule_type, :schedule_config, :enabled, :created_at, :updated_at)"
    );

    // 准备schedule_config（将时间信息存为JSON格式）
    QString scheduleConfig;
    if (task.schedule.time.isValid()) {
        scheduleConfig = QString("{\"time\":\"%1\"}").arg(task.schedule.time.toString("HH:mm"));
    }

    QString currentTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    query.bindValue(":name", task.name);
    query.bindValue(":description", task.description.isEmpty() ? QVariant() : task.description);
    query.bindValue(":repository_id", task.repositoryId);
    query.bindValue(":source_paths", task.sourcePaths.join("\n"));
    query.bindValue(":exclude_patterns", task.excludePatterns.join("\n"));
    query.bindValue(":tags", task.tags.join(","));
    query.bindValue(":hostname", QVariant()); // 可选字段
    query.bindValue(":options", QVariant()); // 可选字段
    query.bindValue(":schedule_type", static_cast<int>(task.schedule.type));
    query.bindValue(":schedule_config", scheduleConfig.isEmpty() ? QVariant() : scheduleConfig);
    query.bindValue(":enabled", task.enabled ? 1 : 0);
    query.bindValue(":created_at", currentTime);
    query.bindValue(":updated_at", currentTime);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("执行SQL插入: sourcePaths=%1, scheduleType=%2")
            .arg(task.sourcePaths.join(","))
            .arg(static_cast<int>(task.schedule.type)));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("插入备份任务失败: %1").arg(m_lastError));
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("SQL错误代码: %1").arg(query.lastError().number()));
        return -1;
    }

    int newId = query.lastInsertId().toInt();
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("备份任务插入成功，ID=%1").arg(newId));

    return newId;
}

bool DatabaseManager::updateBackupTask(const Models::BackupTask& task)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare(
        "UPDATE backup_tasks SET name=:name, description=:description, repository_id=:repository_id, "
        "source_paths=:source_paths, exclude_patterns=:exclude_patterns, tags=:tags, "
        "schedule_type=:schedule_type, schedule_config=:schedule_config, enabled=:enabled, "
        "last_run=:last_run, next_run=:next_run, updated_at=:updated_at WHERE id=:id"
    );

    // 准备schedule_config
    QString scheduleConfig;
    if (task.schedule.time.isValid()) {
        scheduleConfig = QString("{\"time\":\"%1\"}").arg(task.schedule.time.toString("HH:mm"));
    }

    query.bindValue(":id", task.id);
    query.bindValue(":name", task.name);
    query.bindValue(":description", task.description.isEmpty() ? QVariant() : task.description);
    query.bindValue(":repository_id", task.repositoryId);
    query.bindValue(":source_paths", task.sourcePaths.join("\n"));
    query.bindValue(":exclude_patterns", task.excludePatterns.join("\n"));
    query.bindValue(":tags", task.tags.join(","));
    query.bindValue(":schedule_type", static_cast<int>(task.schedule.type));
    query.bindValue(":schedule_config", scheduleConfig.isEmpty() ? QVariant() : scheduleConfig);
    query.bindValue(":enabled", task.enabled ? 1 : 0);
    query.bindValue(":last_run", task.lastRun.isValid() ? task.lastRun.toString(Qt::ISODate) : QVariant());
    query.bindValue(":next_run", task.nextRun.isValid() ? task.nextRun.toString(Qt::ISODate) : QVariant());
    query.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));

    // 添加调试日志
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("updateBackupTask: 任务ID=%1, enabled=%2 (数据库值=%3)")
            .arg(task.id)
            .arg(task.enabled ? "true" : "false")
            .arg(task.enabled ? 1 : 0));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("更新备份任务失败: %1").arg(m_lastError));
        return false;
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("备份任务更新成功，ID=%1").arg(task.id));

    return true;
}

bool DatabaseManager::deleteBackupTask(int id)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM backup_tasks WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

Models::BackupTask DatabaseManager::getBackupTask(int id)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM backup_tasks WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        return Models::BackupTask();
    }

    Models::BackupTask task;
    task.id = query.value("id").toInt();
    task.name = query.value("name").toString();
    task.description = query.value("description").toString();
    task.repositoryId = query.value("repository_id").toInt();
    task.sourcePaths = query.value("source_paths").toString().split("\n", Qt::SkipEmptyParts);
    task.excludePatterns = query.value("exclude_patterns").toString().split("\n", Qt::SkipEmptyParts);
    task.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
    task.schedule.type = static_cast<Models::Schedule::Type>(query.value("schedule_type").toInt());

    // 解析 schedule_config JSON
    QString scheduleConfig = query.value("schedule_config").toString();
    if (!scheduleConfig.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(scheduleConfig.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("time")) {
                task.schedule.time = QTime::fromString(obj["time"].toString(), "HH:mm");
            }
        }
    }

    task.enabled = query.value("enabled").toInt() == 1;

    // 读取 last_run 和 next_run
    QString lastRunStr = query.value("last_run").toString();
    if (!lastRunStr.isEmpty()) {
        task.lastRun = QDateTime::fromString(lastRunStr, Qt::ISODate);
    }

    QString nextRunStr = query.value("next_run").toString();
    if (!nextRunStr.isEmpty()) {
        task.nextRun = QDateTime::fromString(nextRunStr, Qt::ISODate);
    }

    return task;
}

QList<Models::BackupTask> DatabaseManager::getAllBackupTasks()
{
    QMutexLocker locker(&m_mutex);

    QList<Models::BackupTask> tasks;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM backup_tasks ORDER BY name ASC");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Models::BackupTask task;
        task.id = query.value("id").toInt();
        task.name = query.value("name").toString();
        task.description = query.value("description").toString();
        task.repositoryId = query.value("repository_id").toInt();
        task.sourcePaths = query.value("source_paths").toString().split("\n", Qt::SkipEmptyParts);
        task.excludePatterns = query.value("exclude_patterns").toString().split("\n", Qt::SkipEmptyParts);
        task.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
        task.schedule.type = static_cast<Models::Schedule::Type>(query.value("schedule_type").toInt());

        // 解析 schedule_config JSON
        QString scheduleConfig = query.value("schedule_config").toString();
        if (!scheduleConfig.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(scheduleConfig.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("time")) {
                    task.schedule.time = QTime::fromString(obj["time"].toString(), "HH:mm");
                }
            }
        }

        task.enabled = query.value("enabled").toInt() == 1;

        // 添加调试日志
        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("getAllBackupTasks: 任务 \"%1\" (ID: %2) 从数据库读取 enabled=%3 (原始值=%4)")
                .arg(task.name)
                .arg(task.id)
                .arg(task.enabled ? "true" : "false")
                .arg(query.value("enabled").toInt()));

        // 读取 last_run 和 next_run
        QString lastRunStr = query.value("last_run").toString();
        if (!lastRunStr.isEmpty()) {
            task.lastRun = QDateTime::fromString(lastRunStr, Qt::ISODate);
        }

        QString nextRunStr = query.value("next_run").toString();
        if (!nextRunStr.isEmpty()) {
            task.nextRun = QDateTime::fromString(nextRunStr, Qt::ISODate);
        }

        tasks.append(task);
    }

    return tasks;
}

QList<Models::BackupTask> DatabaseManager::getBackupTasksByRepository(int repoId)
{
    QMutexLocker locker(&m_mutex);

    QList<Models::BackupTask> tasks;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM backup_tasks WHERE repository_id=:repoId ORDER BY name ASC");
    query.bindValue(":repoId", repoId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Models::BackupTask task;
        task.id = query.value("id").toInt();
        task.name = query.value("name").toString();
        task.description = query.value("description").toString();
        task.repositoryId = query.value("repository_id").toInt();
        task.sourcePaths = query.value("source_paths").toString().split("\n", Qt::SkipEmptyParts);
        task.excludePatterns = query.value("exclude_patterns").toString().split("\n", Qt::SkipEmptyParts);
        task.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
        task.schedule.type = static_cast<Models::Schedule::Type>(query.value("schedule_type").toInt());

        // 解析 schedule_config JSON
        QString scheduleConfig = query.value("schedule_config").toString();
        if (!scheduleConfig.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(scheduleConfig.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("time")) {
                    task.schedule.time = QTime::fromString(obj["time"].toString(), "HH:mm");
                }
            }
        }

        task.enabled = query.value("enabled").toInt() == 1;

        // 读取 last_run 和 next_run
        QString lastRunStr = query.value("last_run").toString();
        if (!lastRunStr.isEmpty()) {
            task.lastRun = QDateTime::fromString(lastRunStr, Qt::ISODate);
        }

        QString nextRunStr = query.value("next_run").toString();
        if (!nextRunStr.isEmpty()) {
            task.nextRun = QDateTime::fromString(nextRunStr, Qt::ISODate);
        }

        tasks.append(task);
    }

    return tasks;
}

QList<Models::BackupTask> DatabaseManager::getEnabledBackupTasks()
{
    QMutexLocker locker(&m_mutex);

    QList<Models::BackupTask> tasks;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM backup_tasks WHERE enabled=1 ORDER BY name ASC");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Models::BackupTask task;
        task.id = query.value("id").toInt();
        task.name = query.value("name").toString();
        task.description = query.value("description").toString();
        task.repositoryId = query.value("repository_id").toInt();
        task.sourcePaths = query.value("source_paths").toString().split("\n", Qt::SkipEmptyParts);
        task.excludePatterns = query.value("exclude_patterns").toString().split("\n", Qt::SkipEmptyParts);
        task.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
        task.schedule.type = static_cast<Models::Schedule::Type>(query.value("schedule_type").toInt());

        // 解析 schedule_config JSON
        QString scheduleConfig = query.value("schedule_config").toString();
        if (!scheduleConfig.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(scheduleConfig.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("time")) {
                    task.schedule.time = QTime::fromString(obj["time"].toString(), "HH:mm");
                }
            }
        }

        task.enabled = true;

        // 读取 last_run 和 next_run
        QString lastRunStr = query.value("last_run").toString();
        if (!lastRunStr.isEmpty()) {
            task.lastRun = QDateTime::fromString(lastRunStr, Qt::ISODate);
        }

        QString nextRunStr = query.value("next_run").toString();
        if (!nextRunStr.isEmpty()) {
            task.nextRun = QDateTime::fromString(nextRunStr, Qt::ISODate);
        }

        tasks.append(task);
    }

    return tasks;
}

// ========== 备份历史表操作 ==========

int DatabaseManager::insertBackupHistory(const Models::BackupResult& result)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO backup_history (task_id, snapshot_id, start_time, end_time, status, "
        "files_new, files_changed, files_unmodified, dirs_new, dirs_changed, dirs_unmodified, "
        "data_added, total_files, total_bytes, error_message) "
        "VALUES (:task_id, :snapshot_id, :start_time, :end_time, :status, "
        ":files_new, :files_changed, :files_unmodified, :dirs_new, :dirs_changed, :dirs_unmodified, "
        ":data_added, :total_files, :total_bytes, :error_message)"
    );

    query.bindValue(":task_id", result.taskId);
    query.bindValue(":snapshot_id", result.snapshotId);
    query.bindValue(":start_time", result.startTime.toString(Qt::ISODate));
    query.bindValue(":end_time", result.endTime.toString(Qt::ISODate));
    query.bindValue(":status", static_cast<int>(result.status));
    query.bindValue(":files_new", static_cast<qulonglong>(result.filesNew));
    query.bindValue(":files_changed", static_cast<qulonglong>(result.filesChanged));
    query.bindValue(":files_unmodified", static_cast<qulonglong>(result.filesUnmodified));
    query.bindValue(":dirs_new", static_cast<qulonglong>(result.dirsNew));
    query.bindValue(":dirs_changed", static_cast<qulonglong>(result.dirsChanged));
    query.bindValue(":dirs_unmodified", static_cast<qulonglong>(result.dirsUnmodified));
    query.bindValue(":data_added", static_cast<qulonglong>(result.dataAdded));
    query.bindValue(":total_files", static_cast<qulonglong>(result.totalFiles));
    query.bindValue(":total_bytes", static_cast<qulonglong>(result.totalBytes));
    query.bindValue(":error_message", result.errorMessage);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

QList<Models::BackupResult> DatabaseManager::getBackupHistory(int taskId, int limit)
{
    QMutexLocker locker(&m_mutex);

    QList<Models::BackupResult> results;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM backup_history WHERE task_id=:taskId ORDER BY start_time DESC LIMIT :limit");
    query.bindValue(":taskId", taskId);
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return results;
    }

    while (query.next()) {
        Models::BackupResult result;
        result.taskId = query.value("task_id").toInt();
        result.snapshotId = query.value("snapshot_id").toString();
        result.startTime = QDateTime::fromString(query.value("start_time").toString(), Qt::ISODate);
        result.endTime = QDateTime::fromString(query.value("end_time").toString(), Qt::ISODate);

        // 从数据库读取 success 和 status
        result.success = query.value("success").toInt() != 0;
        result.status = static_cast<Models::BackupStatus>(query.value("status").toInt());

        result.filesNew = query.value("files_new").toULongLong();
        result.filesChanged = query.value("files_changed").toULongLong();
        result.filesUnmodified = query.value("files_unmodified").toULongLong();
        result.dirsNew = query.value("dirs_new").toULongLong();
        result.dirsChanged = query.value("dirs_changed").toULongLong();
        result.dirsUnmodified = query.value("dirs_unmodified").toULongLong();
        result.dataAdded = query.value("data_added").toULongLong();
        result.totalFiles = query.value("total_files").toULongLong();
        result.totalBytes = query.value("total_bytes").toULongLong();
        result.errorMessage = query.value("error_message").toString();

        results.append(result);
    }

    return results;
}

QList<Models::BackupResult> DatabaseManager::getRecentBackupHistory(int limit)
{
    QMutexLocker locker(&m_mutex);

    QList<Models::BackupResult> results;
    QSqlQuery query(m_database);

    // 联接 backup_tasks 表获取任务名称
    query.prepare(
        "SELECT h.*, t.name as task_name "
        "FROM backup_history h "
        "LEFT JOIN backup_tasks t ON h.task_id = t.id "
        "ORDER BY h.start_time DESC "
        "LIMIT :limit"
    );
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("获取最近备份历史失败: %1").arg(m_lastError));
        return results;
    }

    while (query.next()) {
        Models::BackupResult result;
        result.taskId = query.value("task_id").toInt();
        result.taskName = query.value("task_name").toString();
        result.snapshotId = query.value("snapshot_id").toString();
        result.startTime = QDateTime::fromString(query.value("start_time").toString(), Qt::ISODate);
        result.endTime = QDateTime::fromString(query.value("end_time").toString(), Qt::ISODate);

        // 从数据库读取 success 和 status
        result.success = query.value("success").toInt() != 0;
        result.status = static_cast<Models::BackupStatus>(query.value("status").toInt());

        result.filesNew = query.value("files_new").toULongLong();
        result.filesChanged = query.value("files_changed").toULongLong();
        result.filesUnmodified = query.value("files_unmodified").toULongLong();
        result.dirsNew = query.value("dirs_new").toULongLong();
        result.dirsChanged = query.value("dirs_changed").toULongLong();
        result.dirsUnmodified = query.value("dirs_unmodified").toULongLong();
        result.dataAdded = query.value("data_added").toULongLong();
        result.totalFiles = query.value("total_files").toULongLong();
        result.totalBytes = query.value("total_bytes").toULongLong();
        result.errorMessage = query.value("error_message").toString();

        results.append(result);
    }

    return results;
}

// ========== 快照缓存表操作 ==========

bool DatabaseManager::cacheSnapshots(int repoId, const QList<Models::Snapshot>& snapshots)
{
    QMutexLocker locker(&m_mutex);

    // 开始事务
    if (!m_database.transaction()) {
        return false;
    }

    // 清除旧缓存
    QSqlQuery deleteQuery(m_database);
    deleteQuery.prepare("DELETE FROM snapshots_cache WHERE repository_id=:repoId");
    deleteQuery.bindValue(":repoId", repoId);
    if (!deleteQuery.exec()) {
        m_database.rollback();
        return false;
    }

    // 插入新缓存
    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(
        "INSERT INTO snapshots_cache (repository_id, snapshot_id, time, hostname, username, "
        "paths, tags, parent) VALUES (:repoId, :snapshotId, :time, :hostname, :username, "
        ":paths, :tags, :parent)"
    );

    for (const Models::Snapshot& snapshot : snapshots) {
        insertQuery.bindValue(":repoId", repoId);
        insertQuery.bindValue(":snapshotId", snapshot.id);
        insertQuery.bindValue(":time", snapshot.time.toString(Qt::ISODate));
        insertQuery.bindValue(":hostname", snapshot.hostname);
        insertQuery.bindValue(":username", snapshot.username);
        insertQuery.bindValue(":paths", snapshot.paths.join("\n"));
        insertQuery.bindValue(":tags", snapshot.tags.join(","));
        insertQuery.bindValue(":parent", snapshot.parent);

        if (!insertQuery.exec()) {
            m_database.rollback();
            m_lastError = insertQuery.lastError().text();
            return false;
        }
    }

    return m_database.commit();
}

QList<Models::Snapshot> DatabaseManager::getCachedSnapshots(int repoId)
{
    QMutexLocker locker(&m_mutex);

    QList<Models::Snapshot> snapshots;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM snapshots_cache WHERE repository_id=:repoId ORDER BY time DESC");
    query.bindValue(":repoId", repoId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return snapshots;
    }

    while (query.next()) {
        Models::Snapshot snapshot;
        snapshot.id = query.value("snapshot_id").toString();
        snapshot.time = QDateTime::fromString(query.value("time").toString(), Qt::ISODate);
        snapshot.hostname = query.value("hostname").toString();
        snapshot.username = query.value("username").toString();
        snapshot.paths = query.value("paths").toString().split("\n", Qt::SkipEmptyParts);
        snapshot.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
        snapshot.parent = query.value("parent").toString();

        snapshots.append(snapshot);
    }

    return snapshots;
}

bool DatabaseManager::clearSnapshotCache(int repoId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM snapshots_cache WHERE repository_id=:repoId");
    query.bindValue(":repoId", repoId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

// ========== 设置表操作 ==========

QString DatabaseManager::getSetting(const QString& key, const QString& defaultValue)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM settings WHERE key=:key");
    query.bindValue(":key", key);

    if (!query.exec() || !query.next()) {
        return defaultValue;
    }

    return query.value(0).toString();
}

bool DatabaseManager::setSetting(const QString& key, const QString& value)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:key, :value)");
    query.bindValue(":key", key);
    query.bindValue(":value", value);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

// ========== 密码存储表操作 ==========

bool DatabaseManager::storePassword(int repoId, const QString& encryptedPassword)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare(
        "INSERT OR REPLACE INTO password_store (repository_id, encrypted_password, created_at) "
        "VALUES (:repoId, :password, :createdAt)"
    );
    query.bindValue(":repoId", repoId);
    query.bindValue(":password", encryptedPassword);
    query.bindValue(":createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

QString DatabaseManager::getStoredPassword(int repoId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("SELECT encrypted_password FROM password_store WHERE repository_id=:repoId");
    query.bindValue(":repoId", repoId);

    if (!query.exec() || !query.next()) {
        return QString();
    }

    return query.value(0).toString();
}

bool DatabaseManager::deleteStoredPassword(int repoId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM password_store WHERE repository_id=:repoId");
    query.bindValue(":repoId", repoId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

} // namespace Data
} // namespace ResticGUI
