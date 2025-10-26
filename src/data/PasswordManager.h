#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QMutex>

namespace ResticGUI {
namespace Data {

/**
 * @brief 密码管理器（单例模式）
 *
 * 提供三种密码存储模式：
 * 1. 不存储 - 每次操作都需要输入密码
 * 2. 会话缓存 - 密码保存在内存中，应用退出后清除
 * 3. 加密存储 - 密码加密后存储在数据库中
 */
class PasswordManager : public QObject
{
    Q_OBJECT

public:
    enum StorageMode {
        NoStorage = 0,      // 不存储密码
        SessionCache = 1,   // 会话缓存
        EncryptedStorage = 2 // 加密存储
    };

    /**
     * @brief 获取单例实例
     */
    static PasswordManager* instance();

    /**
     * @brief 初始化密码管理器
     */
    void initialize();

    /**
     * @brief 设置密码存储模式
     */
    void setStorageMode(StorageMode mode);

    /**
     * @brief 获取密码存储模式
     */
    StorageMode getStorageMode() const { return m_storageMode; }

    /**
     * @brief 设置密码缓存超时时间（分钟）
     */
    void setCacheTimeout(int minutes);

    /**
     * @brief 获取密码缓存超时时间（分钟）
     */
    int getCacheTimeout() const { return m_cacheTimeout; }

    // ========== 密码操作 ==========

    /**
     * @brief 获取仓库密码
     * @param repoId 仓库ID
     * @param password 输出参数，密码
     * @return 成功返回true，失败返回false（需要用户输入）
     */
    bool getPassword(int repoId, QString& password);

    /**
     * @brief 设置仓库密码
     * @param repoId 仓库ID
     * @param password 密码明文
     * @return 成功返回true
     */
    bool setPassword(int repoId, const QString& password);

    /**
     * @brief 删除仓库密码
     * @param repoId 仓库ID
     */
    void removePassword(int repoId);

    /**
     * @brief 清除所有密码缓存
     */
    void clearCache();

    /**
     * @brief 检查密码是否已缓存
     */
    bool hasPassword(int repoId) const;

    /**
     * @brief 刷新密码缓存超时计时器
     */
    void refreshCacheTimer(int repoId);

    // ========== 主密码管理 ==========

    /**
     * @brief 设置主密码（用于加密存储模式）
     */
    void setMasterPassword(const QString& password);

    /**
     * @brief 检查是否已设置主密码
     */
    bool hasMasterPassword() const;

    /**
     * @brief 验证主密码
     */
    bool verifyMasterPassword(const QString& password) const;

    /**
     * @brief 更改主密码
     */
    bool changeMasterPassword(const QString& oldPassword, const QString& newPassword);

signals:
    /**
     * @brief 需要密码信号
     * @param repoId 仓库ID
     */
    void passwordRequired(int repoId);

    /**
     * @brief 密码缓存过期信号
     * @param repoId 仓库ID
     */
    void passwordExpired(int repoId);

private:
    explicit PasswordManager(QObject* parent = nullptr);
    ~PasswordManager();
    PasswordManager(const PasswordManager&) = delete;
    PasswordManager& operator=(const PasswordManager&) = delete;

    /**
     * @brief 从数据库加载加密的密码
     */
    bool loadFromDatabase(int repoId, QString& password);

    /**
     * @brief 保存加密的密码到数据库
     */
    bool saveToDatabase(int repoId, const QString& password);

    /**
     * @brief 加密密码
     */
    QString encryptPassword(const QString& password) const;

    /**
     * @brief 解密密码
     */
    QString decryptPassword(const QString& encrypted) const;

    /**
     * @brief 生成主密码hash
     */
    QString hashMasterPassword(const QString& password) const;

    /**
     * @brief 检查是否已设置主密码（无锁版本，内部使用）
     */
    bool hasMasterPasswordUnlocked() const;

    /**
     * @brief 验证主密码（无锁版本，内部使用）
     */
    bool verifyMasterPasswordUnlocked(const QString& password) const;

private slots:
    /**
     * @brief 密码缓存超时处理
     */
    void onCacheTimeout();

private:
    static PasswordManager* s_instance;
    static QMutex s_instanceMutex;

    StorageMode m_storageMode;
    int m_cacheTimeout; // 分钟

    // 密码缓存（仓库ID -> 密码明文）
    QMap<int, QString> m_passwordCache;

    // 缓存超时定时器（仓库ID -> 定时器）
    QMap<int, QTimer*> m_cacheTimers;

    // 主密码（用于加密存储模式）
    QString m_masterPassword;
    QString m_masterPasswordHash;

    mutable QMutex m_mutex;
};

} // namespace Data
} // namespace ResticGUI

#endif // PASSWORDMANAGER_H
