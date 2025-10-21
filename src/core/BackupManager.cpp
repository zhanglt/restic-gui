/**
 * @file BackupManager.cpp
 * @brief 备份管理器实现
 */

#include "BackupManager.h"
#include "ResticWrapper.h"
#include "RepositoryManager.h"
#include "../data/DatabaseManager.h"
#include "../data/PasswordManager.h"
#include "../utils/Logger.h"
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrent>

namespace ResticGUI {
namespace Core {

BackupManager* BackupManager::s_instance = nullptr;
QMutex BackupManager::s_instanceMutex;

BackupManager* BackupManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new BackupManager();
        }
    }
    return s_instance;
}

BackupManager::BackupManager(QObject* parent)
    : QObject(parent)
    , m_currentTaskId(-1)
    , m_running(false)
{
}

BackupManager::~BackupManager()
{
}

void BackupManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "备份管理器初始化完成");
}

// ========== 备份任务CRUD ==========

int BackupManager::createBackupTask(const Models::BackupTask& task)
{
    QMutexLocker locker(&m_mutex);

    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    int taskId = db->insertBackupTask(task);

    if (taskId > 0) {
        emit taskCreated(taskId);
        emit taskListChanged();
        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("备份任务已创建，ID: %1").arg(taskId));
    }

    return taskId;
}

bool BackupManager::updateBackupTask(const Models::BackupTask& task)
{
    QMutexLocker locker(&m_mutex);

    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    if (db->updateBackupTask(task)) {
        emit taskUpdated(task.id);
        emit taskListChanged();
        return true;
    }

    return false;
}

bool BackupManager::deleteBackupTask(int taskId)
{
    QMutexLocker locker(&m_mutex);

    Data::DatabaseManager* db = Data::DatabaseManager::instance();
    if (db->deleteBackupTask(taskId)) {
        emit taskDeleted(taskId);
        emit taskListChanged();
        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("备份任务已删除，ID: %1").arg(taskId));
        return true;
    }

    return false;
}

Models::BackupTask BackupManager::getBackupTask(int taskId)
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getBackupTask(taskId);
}

QList<Models::BackupTask> BackupManager::getAllBackupTasks()
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getAllBackupTasks();
}

QList<Models::BackupTask> BackupManager::getTasksByRepository(int repoId)
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getBackupTasksByRepository(repoId);
}

// ========== 备份执行 ==========

bool BackupManager::runBackupTask(int taskId)
{
    if (m_running) {
        Utils::Logger::instance()->log(Utils::Logger::Warning, "已有备份任务正在运行");
        return false;
    }

    Models::BackupTask task = getBackupTask(taskId);
    if (task.id < 0) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "备份任务不存在");
        return false;
    }

    // 获取仓库信息
    Models::Repository repo = RepositoryManager::instance()->getRepository(task.repositoryId);
    if (repo.id < 0) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "仓库不存在");
        return false;
    }

    // 获取密码
    QString password;
    if (!Data::PasswordManager::instance()->getPassword(task.repositoryId, password)) {
        Utils::Logger::instance()->log(Utils::Logger::Warning, "需要仓库密码");
        // TODO: 触发密码请求信号
        return false;
    }

    m_running = true;
    m_currentTaskId = taskId;

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始执行备份任务: %1").arg(task.name));

    emit backupStarted(taskId);

    // 异步执行备份，避免阻塞UI线程
    QtConcurrent::run([this, taskId, task, repo, password]() {
        // 执行备份
        ResticWrapper wrapper;
        connect(&wrapper, &ResticWrapper::progressUpdated,
                this, &BackupManager::backupProgress);

        Models::BackupResult result;
        result.taskId = taskId;

        bool success = wrapper.backup(repo, password, task.sourcePaths,
                                      task.excludePatterns, task.tags, result);

        // 保存备份历史
        Data::DatabaseManager::instance()->insertBackupHistory(result);

        // 更新任务的最后运行时间
        Models::BackupTask updatedTask = task;
        updatedTask.lastRun = QDateTime::currentDateTime();
        Data::DatabaseManager::instance()->updateBackupTask(updatedTask);

        m_running = false;
        m_currentTaskId = -1;

        emit backupFinished(taskId, success);

        if (success) {
            Utils::Logger::instance()->log(Utils::Logger::Info, "备份任务完成");
        } else {
            Utils::Logger::instance()->log(Utils::Logger::Error, "备份任务失败");
            emit backupError(result.errorMessage);
        }
    });

    return true;
}

bool BackupManager::runBackupNow(int repoId, const QStringList& sourcePaths,
                                const QStringList& excludePatterns, const QStringList& tags)
{
    if (m_running) {
        return false;
    }

    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    if (repo.id < 0) {
        return false;
    }

    QString password;
    if (!Data::PasswordManager::instance()->getPassword(repoId, password)) {
        return false;
    }

    m_running = true;

    ResticWrapper wrapper;
    connect(&wrapper, &ResticWrapper::progressUpdated,
            this, &BackupManager::backupProgress);

    Models::BackupResult result;
    bool success = wrapper.backup(repo, password, sourcePaths,
                                  excludePatterns, tags, result);

    m_running = false;

    return success;
}

void BackupManager::cancelBackup()
{
    // TODO: 实现取消功能
    Utils::Logger::instance()->log(Utils::Logger::Warning, "取消备份");
}

// ========== 备份历史 ==========

QList<Models::BackupResult> BackupManager::getBackupHistory(int taskId, int limit)
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getBackupHistory(taskId, limit);
}

Models::BackupResult BackupManager::getLastBackupResult(int taskId)
{
    QList<Models::BackupResult> history = getBackupHistory(taskId, 1);
    if (!history.isEmpty()) {
        return history.first();
    }
    return Models::BackupResult();
}

} // namespace Core
} // namespace ResticGUI
