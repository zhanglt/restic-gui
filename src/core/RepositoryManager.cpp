/**
 * @file RepositoryManager.cpp
 * @brief 仓库管理器实现
 */

#include "RepositoryManager.h"
#include "ResticWrapper.h"
#include "../data/DatabaseManager.h"
#include "../data/PasswordManager.h"
#include "../data/CacheManager.h"
#include "../utils/Logger.h"
#include <QMutexLocker>

namespace ResticGUI {
namespace Core {

RepositoryManager* RepositoryManager::s_instance = nullptr;
QMutex RepositoryManager::s_instanceMutex;

RepositoryManager* RepositoryManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new RepositoryManager();
        }
    }
    return s_instance;
}

RepositoryManager::RepositoryManager(QObject* parent)
    : QObject(parent)
{
}

RepositoryManager::~RepositoryManager()
{
}

void RepositoryManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "仓库管理器初始化完成");
}

// ========== 仓库CRUD操作 ==========

int RepositoryManager::createRepository(const Models::Repository& repo, const QString& password, bool initialize)
{
    int repoId;
    {
        QMutexLocker locker(&m_mutex);

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("创建仓库: %1").arg(repo.name));

        // 插入数据库
        Data::DatabaseManager* db = Data::DatabaseManager::instance();
        repoId = db->insertRepository(repo);

        if (repoId < 0) {
            Utils::Logger::instance()->log(Utils::Logger::Error, "插入仓库到数据库失败");
            return -1;
        }

        // 缓存密码
        if (!password.isEmpty()) {
            Data::PasswordManager::instance()->setPassword(repoId, password);
        }
    } // 先释放锁

    // 如果需要初始化restic仓库（无需锁）
    if (initialize) {
        if (!initializeRepository(repoId, password)) {
            Utils::Logger::instance()->log(Utils::Logger::Error, "初始化restic仓库失败");
            // 删除数据库记录
            Data::DatabaseManager::instance()->deleteRepository(repoId);
            return -1;
        }
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("仓库创建成功，ID: %1").arg(repoId));

    // 在锁释放后发出信号，避免死锁
    emit repositoryCreated(repoId);
    emit repositoryListChanged();

    return repoId;
}

bool RepositoryManager::updateRepository(const Models::Repository& repo)
{
    {
        QMutexLocker locker(&m_mutex);

        Data::DatabaseManager* db = Data::DatabaseManager::instance();
        if (!db->updateRepository(repo)) {
            return false;
        }

        // 清除该仓库的缓存
        Data::CacheManager::instance()->clearRepositoryCache(repo.id);

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("仓库已更新，ID: %1").arg(repo.id));
    } // 锁在这里释放

    // 在锁释放后发出信号，避免死锁
    emit repositoryUpdated(repo.id);
    emit repositoryListChanged();

    return true;
}

bool RepositoryManager::deleteRepository(int repoId, bool deleteData)
{
    {
        QMutexLocker locker(&m_mutex);

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("删除仓库，ID: %1, 删除数据: %2").arg(repoId).arg(deleteData));

        // TODO: 如果deleteData为true，删除restic仓库数据
        // 这需要特别小心，可能需要用户二次确认

        // 从数据库删除
        Data::DatabaseManager* db = Data::DatabaseManager::instance();
        if (!db->deleteRepository(repoId)) {
            return false;
        }

        // 清除密码
        Data::PasswordManager::instance()->removePassword(repoId);

        // 清除缓存
        Data::CacheManager::instance()->clearRepositoryCache(repoId);

        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库已删除");
    } // 锁在这里释放

    // 在锁释放后发出信号，避免死锁
    emit repositoryDeleted(repoId);
    emit repositoryListChanged();

    return true;
}

Models::Repository RepositoryManager::getRepository(int repoId)
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getRepository(repoId);
}

QList<Models::Repository> RepositoryManager::getAllRepositories()
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getAllRepositories();
}

Models::Repository RepositoryManager::getDefaultRepository()
{
    QMutexLocker locker(&m_mutex);
    return Data::DatabaseManager::instance()->getDefaultRepository();
}

bool RepositoryManager::setDefaultRepository(int repoId)
{
    bool result;
    {
        QMutexLocker locker(&m_mutex);
        result = Data::DatabaseManager::instance()->setDefaultRepository(repoId);
    } // 锁在这里释放

    // 在锁释放后发出信号，避免死锁
    if (result) {
        emit repositoryListChanged();
    }

    return result;
}

// ========== 仓库操作 ==========

bool RepositoryManager::initializeRepository(int repoId, const QString& password)
{
    Models::Repository repo = getRepository(repoId);
    if (repo.id < 0) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "仓库不存在");
        return false;
    }

    ResticWrapper wrapper;
    bool success = wrapper.initRepository(repo, password);

    if (success) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库初始化成功");
    }

    return success;
}

bool RepositoryManager::checkRepository(int repoId, const QString& password, bool readData)
{
    Models::Repository repo = getRepository(repoId);
    if (repo.id < 0) {
        return false;
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("检查仓库: %1").arg(repo.name));

    ResticWrapper wrapper;
    return wrapper.checkRepository(repo, password, readData);
}

bool RepositoryManager::unlockRepository(int repoId, const QString& password)
{
    Models::Repository repo = getRepository(repoId);
    if (repo.id < 0) {
        return false;
    }

    ResticWrapper wrapper;
    return wrapper.unlockRepository(repo, password);
}

bool RepositoryManager::testConnection(const Models::Repository& repo, const QString& password)
{
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("测试仓库连接: %1").arg(repo.name));

    ResticWrapper wrapper;

    // 尝试列出快照（最简单的测试方法）
    QList<Models::Snapshot> snapshots;
    return wrapper.listSnapshots(repo, password, snapshots);
}

Models::RepoStats RepositoryManager::getRepositoryStats(int repoId, const QString& password)
{
    Models::Repository repo = getRepository(repoId);
    Models::RepoStats stats;

    if (repo.id < 0) {
        return stats;
    }

    // 检查缓存
    Data::CacheManager* cache = Data::CacheManager::instance();
    if (cache->isRepoStatsCacheValid(repoId, 10)) {
        if (cache->getCachedRepoStats(repoId, stats)) {
            Utils::Logger::instance()->log(Utils::Logger::Debug, "使用缓存的仓库统计信息");
            return stats;
        }
    }

    // 从restic获取
    ResticWrapper wrapper;
    if (wrapper.getStats(repo, password, stats)) {
        // 缓存结果
        cache->cacheRepoStats(repoId, stats);
    }

    return stats;
}

bool RepositoryManager::pruneRepository(int repoId, const QString& password,
                                       int keepLast, int keepDaily, int keepWeekly,
                                       int keepMonthly, int keepYearly)
{
    Models::Repository repo = getRepository(repoId);
    if (repo.id < 0) {
        return false;
    }

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("维护仓库: %1").arg(repo.name));

    ResticWrapper wrapper;

    // 连接进度信号
    connect(&wrapper, &ResticWrapper::progressUpdated,
            this, &RepositoryManager::progressUpdated);

    bool success = wrapper.prune(repo, password, keepLast, keepDaily,
                                 keepWeekly, keepMonthly, keepYearly);

    if (success) {
        // 清除统计缓存（因为仓库大小可能已改变）
        Data::CacheManager::instance()->clearRepoStatsCache(repoId);
    }

    return success;
}

// ========== 密码管理 ==========

bool RepositoryManager::getPassword(int repoId, QString& password)
{
    return Data::PasswordManager::instance()->getPassword(repoId, password);
}

void RepositoryManager::setPassword(int repoId, const QString& password)
{
    Data::PasswordManager::instance()->setPassword(repoId, password);
}

void RepositoryManager::clearPasswordCache(int repoId)
{
    Data::PasswordManager::instance()->removePassword(repoId);
}

} // namespace Core
} // namespace ResticGUI
