/**
 * @file PasswordManager.cpp
 * @brief 密码管理器实现
 */

#include "PasswordManager.h"
#include "DatabaseManager.h"
#include "ConfigManager.h"
#include "../utils/Logger.h"
#include "../utils/CryptoUtil.h"
#include <QMutexLocker>
#include <QMetaObject>

namespace ResticGUI {
namespace Data {

PasswordManager* PasswordManager::s_instance = nullptr;
QMutex PasswordManager::s_instanceMutex;

PasswordManager* PasswordManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new PasswordManager();
        }
    }
    return s_instance;
}

PasswordManager::PasswordManager(QObject* parent)
    : QObject(parent)
    , m_storageMode(SessionCache)
    , m_cacheTimeout(30)
{
}

PasswordManager::~PasswordManager()
{
    // clearCache() 已经会清理所有定时器和缓存
    clearCache();
}

void PasswordManager::initialize()
{
    // 从配置读取存储模式
    ConfigManager* config = ConfigManager::instance();
    m_storageMode = static_cast<StorageMode>(config->getPasswordStorageMode());
    m_cacheTimeout = config->getPasswordCacheTimeout();

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("密码管理器初始化完成，存储模式: %1").arg(m_storageMode));
}

void PasswordManager::setStorageMode(StorageMode mode)
{
    QMutexLocker locker(&m_mutex);

    if (m_storageMode == mode) {
        return;
    }

    // 如果从加密存储切换到其他模式，清除数据库中的密码
    if (m_storageMode == EncryptedStorage && mode != EncryptedStorage) {
        DatabaseManager* db = DatabaseManager::instance();
        for (int repoId : m_passwordCache.keys()) {
            db->deleteStoredPassword(repoId);
        }
    }

    m_storageMode = mode;

    // 保存到配置
    ConfigManager::instance()->setPasswordStorageMode(static_cast<int>(mode));

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("密码存储模式已更改为: %1").arg(mode));
}

void PasswordManager::setCacheTimeout(int minutes)
{
    QMutexLocker locker(&m_mutex);
    m_cacheTimeout = minutes;

    // 更新所有现有定时器 - 在主线程中执行
    int intervalMs = minutes * 60 * 1000;
    for (QTimer* timer : m_cacheTimers.values()) {
        QMetaObject::invokeMethod(timer, [timer, intervalMs]() {
            timer->setInterval(intervalMs);
        }, Qt::QueuedConnection);
    }

    // 保存到配置
    ConfigManager::instance()->setPasswordCacheTimeout(minutes);
}

// ========== 密码操作 ==========

bool PasswordManager::getPassword(int repoId, QString& password)
{
    QMutexLocker locker(&m_mutex);

    // 1. 检查缓存
    if (m_passwordCache.contains(repoId)) {
        password = m_passwordCache[repoId];
        refreshCacheTimer(repoId);
        return true;
    }

    // 2. 如果是加密存储模式，尝试从数据库加载
    if (m_storageMode == EncryptedStorage) {
        if (loadFromDatabase(repoId, password)) {
            // 加载成功，缓存到内存
            m_passwordCache[repoId] = password;
            refreshCacheTimer(repoId);
            return true;
        }
    }

    // 3. 没有找到密码，需要用户输入
    return false;
}

bool PasswordManager::setPassword(int repoId, const QString& password)
{
    QMutexLocker locker(&m_mutex);

    // 1. 缓存到内存
    if (m_storageMode == SessionCache || m_storageMode == EncryptedStorage) {
        m_passwordCache[repoId] = password;
        refreshCacheTimer(repoId);
    }

    // 2. 如果是加密存储模式，保存到数据库
    if (m_storageMode == EncryptedStorage) {
        if (!saveToDatabase(repoId, password)) {
            Utils::Logger::instance()->log(Utils::Logger::Error,
                QString("无法保存密码到数据库，仓库ID: %1").arg(repoId));
            return false;
        }
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("密码已设置，仓库ID: %1").arg(repoId));
    return true;
}

void PasswordManager::removePassword(int repoId)
{
    QMutexLocker locker(&m_mutex);

    // 从缓存移除
    m_passwordCache.remove(repoId);

    // 停止并删除定时器 - 在主线程中执行
    if (m_cacheTimers.contains(repoId)) {
        QTimer* timer = m_cacheTimers.take(repoId);
        // 使用 QMetaObject::invokeMethod 确保在主线程中停止定时器
        QMetaObject::invokeMethod(timer, [timer]() {
            timer->stop();
            timer->deleteLater();
        }, Qt::QueuedConnection);
    }

    // 从数据库移除
    if (m_storageMode == EncryptedStorage) {
        DatabaseManager::instance()->deleteStoredPassword(repoId);
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("密码已移除，仓库ID: %1").arg(repoId));
}

void PasswordManager::clearCache()
{
    QMutexLocker locker(&m_mutex);

    m_passwordCache.clear();

    // 停止所有定时器 - 在主线程中执行
    QList<QTimer*> timersToDelete = m_cacheTimers.values();
    m_cacheTimers.clear();

    for (QTimer* timer : timersToDelete) {
        // 使用 QMetaObject::invokeMethod 确保在主线程中停止定时器
        QMetaObject::invokeMethod(timer, [timer]() {
            timer->stop();
            timer->deleteLater();
        }, Qt::QueuedConnection);
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug, "所有密码缓存已清除");
}

bool PasswordManager::hasPassword(int repoId) const
{
    QMutexLocker locker(&m_mutex);
    return m_passwordCache.contains(repoId);
}

void PasswordManager::refreshCacheTimer(int repoId)
{
    // 注意：此函数调用者已经锁定了mutex，不需要再锁定

    if (m_cacheTimeout <= 0) {
        return; // 永不过期
    }

    // 如果定时器不存在，创建新的
    if (!m_cacheTimers.contains(repoId)) {
        // 使用 QMetaObject::invokeMethod 确保在主线程中创建定时器
        // 避免跨线程创建 QObject 子对象的问题
        QMetaObject::invokeMethod(this, [this, repoId]() {
            QMutexLocker locker(&m_mutex);

            // 再次检查，避免重复创建
            if (m_cacheTimers.contains(repoId)) {
                m_cacheTimers[repoId]->start();
                return;
            }

            QTimer* timer = new QTimer(this);
            timer->setInterval(m_cacheTimeout * 60 * 1000); // 转换为毫秒
            timer->setSingleShot(true);

            // 连接超时信号
            connect(timer, &QTimer::timeout, this, [this, repoId]() {
                QMutexLocker locker(&m_mutex);

                // 从缓存中移除
                m_passwordCache.remove(repoId);

                // 删除定时器
                if (m_cacheTimers.contains(repoId)) {
                    QTimer* expiredTimer = m_cacheTimers.take(repoId);
                    expiredTimer->deleteLater();
                }

                Utils::Logger::instance()->log(Utils::Logger::Debug,
                    QString("密码缓存已过期，仓库ID: %1").arg(repoId));

                emit passwordExpired(repoId);
            });

            m_cacheTimers[repoId] = timer;
            timer->start();
        }, Qt::QueuedConnection);
    } else {
        // 重置现有定时器 - 也需要在主线程中执行
        QMetaObject::invokeMethod(this, [this, repoId]() {
            QMutexLocker locker(&m_mutex);
            if (m_cacheTimers.contains(repoId)) {
                m_cacheTimers[repoId]->start();
            }
        }, Qt::QueuedConnection);
    }
}

// ========== 主密码管理 ==========

void PasswordManager::setMasterPassword(const QString& password)
{
    QMutexLocker locker(&m_mutex);

    m_masterPassword = password;
    m_masterPasswordHash = hashMasterPassword(password);

    // 保存hash到数据库
    DatabaseManager::instance()->setSetting("master_password_hash", m_masterPasswordHash);

    Utils::Logger::instance()->log(Utils::Logger::Info, "主密码已设置");
}

bool PasswordManager::hasMasterPassword() const
{
    QMutexLocker locker(&m_mutex);
    return hasMasterPasswordUnlocked();
}

bool PasswordManager::hasMasterPasswordUnlocked() const
{
    // 注意：调用此函数前必须已经获取了锁

    if (!m_masterPasswordHash.isEmpty()) {
        return true;
    }

    // 检查数据库
    QString hash = DatabaseManager::instance()->getSetting("master_password_hash");
    return !hash.isEmpty();
}

bool PasswordManager::verifyMasterPassword(const QString& password) const
{
    QMutexLocker locker(&m_mutex);
    return verifyMasterPasswordUnlocked(password);
}

bool PasswordManager::verifyMasterPasswordUnlocked(const QString& password) const
{
    // 注意：调用此函数前必须已经获取了锁

    QString hash = hashMasterPassword(password);

    // 如果内存中有hash，使用内存中的
    if (!m_masterPasswordHash.isEmpty()) {
        return hash == m_masterPasswordHash;
    }

    // 否则从数据库读取
    QString storedHash = DatabaseManager::instance()->getSetting("master_password_hash");
    return hash == storedHash;
}

bool PasswordManager::changeMasterPassword(const QString& oldPassword, const QString& newPassword)
{
    QMutexLocker locker(&m_mutex);

    // 验证旧密码（使用无锁版本）
    if (!verifyMasterPasswordUnlocked(oldPassword)) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "旧主密码验证失败");
        return false;
    }

    // 重新加密所有存储的密码
    DatabaseManager* db = DatabaseManager::instance();

    // TODO: 需要遍历所有仓库，重新加密密码
    // 这需要先用旧密码解密，再用新密码加密

    // 设置新主密码
    m_masterPassword = newPassword;
    m_masterPasswordHash = hashMasterPassword(newPassword);
    db->setSetting("master_password_hash", m_masterPasswordHash);

    Utils::Logger::instance()->log(Utils::Logger::Info, "主密码已更改");
    return true;
}

// ========== 私有辅助函数 ==========

bool PasswordManager::loadFromDatabase(int repoId, QString& password)
{
    // 注意：此函数调用者已经获取了锁

    if (!hasMasterPasswordUnlocked()) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "未设置主密码，无法加载密码");
        return false;
    }

    DatabaseManager* db = DatabaseManager::instance();
    QString encrypted = db->getStoredPassword(repoId);

    if (encrypted.isEmpty()) {
        return false;
    }

    password = decryptPassword(encrypted);
    return !password.isEmpty();
}

bool PasswordManager::saveToDatabase(int repoId, const QString& password)
{
    // 注意：此函数调用者已经获取了锁

    if (!hasMasterPasswordUnlocked()) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "未设置主密码，无法保存密码");
        return false;
    }

    QString encrypted = encryptPassword(password);
    if (encrypted.isEmpty()) {
        return false;
    }

    DatabaseManager* db = DatabaseManager::instance();
    return db->storePassword(repoId, encrypted);
}

QString PasswordManager::encryptPassword(const QString& password) const
{
    if (m_masterPassword.isEmpty()) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "主密码为空，无法加密");
        return QString();
    }

    return Utils::CryptoUtil::encrypt(password, m_masterPassword);
}

QString PasswordManager::decryptPassword(const QString& encrypted) const
{
    if (m_masterPassword.isEmpty()) {
        Utils::Logger::instance()->log(Utils::Logger::Error, "主密码为空，无法解密");
        return QString();
    }

    return Utils::CryptoUtil::decrypt(encrypted, m_masterPassword);
}

QString PasswordManager::hashMasterPassword(const QString& password) const
{
    return Utils::CryptoUtil::sha256(password);
}

void PasswordManager::onCacheTimeout()
{
    // 此函数由定时器信号触发，在lambda中已实现
}

} // namespace Data
} // namespace ResticGUI
