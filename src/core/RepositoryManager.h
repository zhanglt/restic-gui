#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H

#include <QObject>
#include <QMutex>
#include "../models/Repository.h"
#include "../models/RepoStats.h"

namespace ResticGUI {
namespace Core {

/**
 * @brief 仓库管理器（单例模式）
 *
 * 负责仓库的增删改查、初始化、检查等操作
 */
class RepositoryManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static RepositoryManager* instance();

    /**
     * @brief 初始化管理器
     */
    void initialize();

    // ========== 仓库CRUD操作 ==========

    /**
     * @brief 创建新仓库
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param initialize 是否初始化restic仓库
     * @return 成功返回仓库ID，失败返回-1
     */
    int createRepository(const Models::Repository& repo, const QString& password, bool initialize = true);

    /**
     * @brief 更新仓库信息
     */
    bool updateRepository(const Models::Repository& repo);

    /**
     * @brief 删除仓库
     * @param repoId 仓库ID
     * @param deleteData 是否删除仓库数据
     */
    bool deleteRepository(int repoId, bool deleteData = false);

    /**
     * @brief 获取仓库信息
     */
    Models::Repository getRepository(int repoId);

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
    bool setDefaultRepository(int repoId);

    // ========== 仓库操作 ==========

    /**
     * @brief 初始化仓库
     * @param repoId 仓库ID
     * @param password 密码
     */
    bool initializeRepository(int repoId, const QString& password);

    /**
     * @brief 检查仓库
     * @param repoId 仓库ID
     * @param password 密码
     * @param readData 是否读取数据包（更彻底的检查）
     */
    bool checkRepository(int repoId, const QString& password, bool readData = false);

    /**
     * @brief 解锁仓库
     */
    bool unlockRepository(int repoId, const QString& password);

    /**
     * @brief 测试仓库连接
     */
    bool testConnection(const Models::Repository& repo, const QString& password);

    /**
     * @brief 获取仓库统计信息
     */
    Models::RepoStats getRepositoryStats(int repoId, const QString& password);

    /**
     * @brief 维护仓库（prune）
     */
    bool pruneRepository(int repoId, const QString& password,
                        int keepLast = 0, int keepDaily = 0, int keepWeekly = 0,
                        int keepMonthly = 0, int keepYearly = 0);

    // ========== 密码管理 ==========

    /**
     * @brief 获取仓库密码
     * @param repoId 仓库ID
     * @param password 输出参数，密码
     * @return 成功返回true（密码已缓存），失败返回false（需要用户输入）
     */
    bool getPassword(int repoId, QString& password);

    /**
     * @brief 设置仓库密码
     */
    void setPassword(int repoId, const QString& password);

    /**
     * @brief 清除密码缓存
     */
    void clearPasswordCache(int repoId);

signals:
    /**
     * @brief 仓库列表已更改
     */
    void repositoryListChanged();

    /**
     * @brief 仓库已创建
     */
    void repositoryCreated(int repoId);

    /**
     * @brief 仓库已更新
     */
    void repositoryUpdated(int repoId);

    /**
     * @brief 仓库已删除
     */
    void repositoryDeleted(int repoId);

    /**
     * @brief 需要密码
     */
    void passwordRequired(int repoId);

    /**
     * @brief 操作进度
     */
    void progressUpdated(int percent, const QString& message);

private:
    explicit RepositoryManager(QObject* parent = nullptr);
    ~RepositoryManager();
    RepositoryManager(const RepositoryManager&) = delete;
    RepositoryManager& operator=(const RepositoryManager&) = delete;

    static RepositoryManager* s_instance;
    static QMutex s_instanceMutex;

    mutable QMutex m_mutex;
};

} // namespace Core
} // namespace ResticGUI

#endif // REPOSITORYMANAGER_H
