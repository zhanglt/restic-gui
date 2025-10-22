#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QObject>
#include <QMutex>
#include "../models/BackupTask.h"
#include "../models/BackupResult.h"

namespace ResticGUI {
namespace Core {

/**
 * @brief 备份管理器（单例模式）
 *
 * 负责备份任务的管理和执行
 */
class BackupManager : public QObject
{
    Q_OBJECT

public:
    static BackupManager* instance();
    void initialize();

    // ========== 备份任务CRUD ==========
    int createBackupTask(const Models::BackupTask& task);
    bool updateBackupTask(const Models::BackupTask& task);
    bool deleteBackupTask(int taskId);
    Models::BackupTask getBackupTask(int taskId);
    QList<Models::BackupTask> getAllBackupTasks();
    QList<Models::BackupTask> getTasksByRepository(int repoId);

    // ========== 备份执行 ==========
    bool runBackupTask(int taskId);
    bool runBackupNow(int repoId, const QStringList& sourcePaths,
                     const QStringList& excludePatterns, const QStringList& tags);
    void cancelBackup();

    // ========== 备份历史 ==========
    QList<Models::BackupResult> getBackupHistory(int taskId, int limit = 100);
    Models::BackupResult getLastBackupResult(int taskId);

signals:
    void taskListChanged();
    void taskCreated(int taskId);
    void taskUpdated(int taskId);
    void taskDeleted(int taskId);
    void backupStarted(int taskId);
    void backupProgress(int percent, const QString& message);
    void backupFinished(int taskId, bool success);
    void backupError(const QString& error);
    void passwordError(int taskId, int repoId);  // 密码错误信号

private:
    explicit BackupManager(QObject* parent = nullptr);
    ~BackupManager();
    BackupManager(const BackupManager&) = delete;
    BackupManager& operator=(const BackupManager&) = delete;

    static BackupManager* s_instance;
    static QMutex s_instanceMutex;
    mutable QMutex m_mutex;
    int m_currentTaskId;
    bool m_running;
};

} // namespace Core
} // namespace ResticGUI

#endif // BACKUPMANAGER_H
