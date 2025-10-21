#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMutex>
#include "../models/Repository.h"
#include "../models/BackupTask.h"
#include "../models/Snapshot.h"
#include "../models/BackupResult.h"

namespace ResticGUI {
namespace Data {

/**
 * @brief 数据库管理器（单例模式）
 *
 * 负责SQLite数据库的初始化、连接管理和基本CRUD操作
 * 线程安全设计，支持多线程访问
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static DatabaseManager* instance();

    /**
     * @brief 初始化数据库
     * @param dbPath 数据库文件路径
     * @return 成功返回true
     */
    bool initialize(const QString& dbPath);

    /**
     * @brief 执行SQL查询
     * @param query 要执行的查询对象
     * @return 成功返回true
     */
    bool executeQuery(QSqlQuery& query);

    /**
     * @brief 开始事务
     */
    bool beginTransaction();

    /**
     * @brief 提交事务
     */
    bool commit();

    /**
     * @brief 回滚事务
     */
    bool rollback();

    // ========== 仓库表操作 ==========

    /**
     * @brief 插入仓库记录
     */
    int insertRepository(const Models::Repository& repo);

    /**
     * @brief 更新仓库记录
     */
    bool updateRepository(const Models::Repository& repo);

    /**
     * @brief 删除仓库记录
     */
    bool deleteRepository(int id);

    /**
     * @brief 获取仓库记录
     */
    Models::Repository getRepository(int id);

    /**
     * @brief 获取所有仓库
     */
    QList<Models::Repository> getAllRepositories();

    /**
     * @brief 获取默认仓库
     */
    Models::Repository getDefaultRepository();

    /**
     * @brief 设置默认仓库
     */
    bool setDefaultRepository(int id);

    // ========== 备份任务表操作 ==========

    /**
     * @brief 插入备份任务
     */
    int insertBackupTask(const Models::BackupTask& task);

    /**
     * @brief 更新备份任务
     */
    bool updateBackupTask(const Models::BackupTask& task);

    /**
     * @brief 删除备份任务
     */
    bool deleteBackupTask(int id);

    /**
     * @brief 获取备份任务
     */
    Models::BackupTask getBackupTask(int id);

    /**
     * @brief 获取所有备份任务
     */
    QList<Models::BackupTask> getAllBackupTasks();

    /**
     * @brief 获取指定仓库的备份任务
     */
    QList<Models::BackupTask> getBackupTasksByRepository(int repoId);

    /**
     * @brief 获取启用的备份任务
     */
    QList<Models::BackupTask> getEnabledBackupTasks();

    // ========== 备份历史表操作 ==========

    /**
     * @brief 插入备份历史记录
     */
    int insertBackupHistory(const Models::BackupResult& result);

    /**
     * @brief 获取备份历史记录
     */
    QList<Models::BackupResult> getBackupHistory(int taskId, int limit = 100);

    // ========== 快照缓存表操作 ==========

    /**
     * @brief 缓存快照列表
     */
    bool cacheSnapshots(int repoId, const QList<Models::Snapshot>& snapshots);

    /**
     * @brief 获取缓存的快照列表
     */
    QList<Models::Snapshot> getCachedSnapshots(int repoId);

    /**
     * @brief 清除快照缓存
     */
    bool clearSnapshotCache(int repoId);

    // ========== 设置表操作 ==========

    /**
     * @brief 获取设置值
     */
    QString getSetting(const QString& key, const QString& defaultValue = QString());

    /**
     * @brief 设置设置值
     */
    bool setSetting(const QString& key, const QString& value);

    // ========== 密码存储表操作 ==========

    /**
     * @brief 存储加密的密码
     */
    bool storePassword(int repoId, const QString& encryptedPassword);

    /**
     * @brief 获取加密的密码
     */
    QString getStoredPassword(int repoId);

    /**
     * @brief 删除存储的密码
     */
    bool deleteStoredPassword(int repoId);

    /**
     * @brief 获取数据库错误信息
     */
    QString lastError() const { return m_lastError; }

signals:
    /**
     * @brief 数据库错误信号
     */
    void databaseError(const QString& error);

private:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief 初始化数据库表结构
     */
    bool initializeSchema();

    /**
     * @brief 检查并升级数据库版本
     */
    bool checkAndUpgradeSchema();

    /**
     * @brief 执行SQL脚本文件
     */
    bool executeSqlScript(const QString& scriptPath);

    static DatabaseManager* s_instance;
    static QMutex s_instanceMutex;

    QSqlDatabase m_database;
    QMutex m_mutex;
    QString m_lastError;
    QString m_databasePath;
    int m_schemaVersion = 1;
};

} // namespace Data
} // namespace ResticGUI

#endif // DATABASEMANAGER_H
