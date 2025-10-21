/**
 * @file ConfigManager.cpp
 * @brief 配置管理器实现
 */

#include "ConfigManager.h"
#include "../utils/Logger.h"
#include <QCoreApplication>
#include <QMutexLocker>

namespace ResticGUI {
namespace Data {

ConfigManager* ConfigManager::s_instance = nullptr;
QMutex ConfigManager::s_instanceMutex;

ConfigManager* ConfigManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new ConfigManager();
        }
    }
    return s_instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
    // 使用QSettings管理配置文件
    m_settings = new QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        "ResticGUI",
        "ResticGUI"
    );
}

ConfigManager::~ConfigManager()
{
    delete m_settings;
}

void ConfigManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("配置文件位置: %1").arg(m_settings->fileName()));

    // 设置默认值
    if (!contains("App/Language")) {
        setValue("App/Language", "zh_CN");
    }
    if (!contains("App/Theme")) {
        setValue("App/Theme", "default");
    }
    if (!contains("App/AutoStart")) {
        setValue("App/AutoStart", false);
    }
    if (!contains("App/MinimizeToTray")) {
        setValue("App/MinimizeToTray", true);
    }
    if (!contains("App/CheckUpdatesOnStartup")) {
        setValue("App/CheckUpdatesOnStartup", true);
    }

    // 备份设置默认值
    if (!contains("Backup/ShowNotifications")) {
        setValue("Backup/ShowNotifications", true);
    }
    if (!contains("Backup/MaxParallelBackups")) {
        setValue("Backup/MaxParallelBackups", 1);
    }
    if (!contains("Backup/LogRetentionDays")) {
        setValue("Backup/LogRetentionDays", 30);
    }
    if (!contains("Backup/DefaultExcludePatterns")) {
        QStringList defaultExcludes = {
            "*.tmp", "*.temp", "*.cache",
            "Thumbs.db", ".DS_Store",
            "node_modules/", ".git/"
        };
        setValue("Backup/DefaultExcludePatterns", defaultExcludes);
    }

    // 密码设置默认值
    if (!contains("Password/StorageMode")) {
        setValue("Password/StorageMode", 1); // 默认会话缓存
    }
    if (!contains("Password/CacheTimeout")) {
        setValue("Password/CacheTimeout", 30); // 30分钟
    }

    // 网络设置默认值
    if (!contains("Network/UseProxy")) {
        setValue("Network/UseProxy", false);
    }
    if (!contains("Network/Timeout")) {
        setValue("Network/Timeout", 60); // 60秒
    }

    sync();
}

// ========== 通用配置 ==========

QVariant ConfigManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->value(key, defaultValue);
}

void ConfigManager::setValue(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    m_settings->setValue(key, value);
    emit configChanged(key, value);
}

bool ConfigManager::contains(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings->contains(key);
}

void ConfigManager::remove(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    m_settings->remove(key);
}

void ConfigManager::sync()
{
    QMutexLocker locker(&m_mutex);
    m_settings->sync();
}

// ========== 应用程序设置 ==========

QString ConfigManager::getResticPath() const
{
    return getValue("App/ResticPath", "restic").toString();
}

void ConfigManager::setResticPath(const QString& path)
{
    setValue("App/ResticPath", path);
}

QString ConfigManager::getLanguage() const
{
    return getValue("App/Language", "zh_CN").toString();
}

void ConfigManager::setLanguage(const QString& lang)
{
    setValue("App/Language", lang);
}

QString ConfigManager::getTheme() const
{
    return getValue("App/Theme", "default").toString();
}

void ConfigManager::setTheme(const QString& theme)
{
    setValue("App/Theme", theme);
}

bool ConfigManager::getAutoStart() const
{
    return getValue("App/AutoStart", false).toBool();
}

void ConfigManager::setAutoStart(bool enabled)
{
    setValue("App/AutoStart", enabled);
}

bool ConfigManager::getMinimizeToTray() const
{
    return getValue("App/MinimizeToTray", true).toBool();
}

void ConfigManager::setMinimizeToTray(bool enabled)
{
    setValue("App/MinimizeToTray", enabled);
}

bool ConfigManager::getCheckUpdatesOnStartup() const
{
    return getValue("App/CheckUpdatesOnStartup", true).toBool();
}

void ConfigManager::setCheckUpdatesOnStartup(bool enabled)
{
    setValue("App/CheckUpdatesOnStartup", enabled);
}

// ========== 备份设置 ==========

QStringList ConfigManager::getDefaultExcludePatterns() const
{
    return getValue("Backup/DefaultExcludePatterns").toStringList();
}

void ConfigManager::setDefaultExcludePatterns(const QStringList& patterns)
{
    setValue("Backup/DefaultExcludePatterns", patterns);
}

bool ConfigManager::getShowBackupNotifications() const
{
    return getValue("Backup/ShowNotifications", true).toBool();
}

void ConfigManager::setShowBackupNotifications(bool enabled)
{
    setValue("Backup/ShowNotifications", enabled);
}

int ConfigManager::getMaxParallelBackups() const
{
    return getValue("Backup/MaxParallelBackups", 1).toInt();
}

void ConfigManager::setMaxParallelBackups(int count)
{
    setValue("Backup/MaxParallelBackups", count);
}

int ConfigManager::getLogRetentionDays() const
{
    return getValue("Backup/LogRetentionDays", 30).toInt();
}

void ConfigManager::setLogRetentionDays(int days)
{
    setValue("Backup/LogRetentionDays", days);
}

// ========== 密码设置 ==========

int ConfigManager::getPasswordStorageMode() const
{
    return getValue("Password/StorageMode", 1).toInt();
}

void ConfigManager::setPasswordStorageMode(int mode)
{
    setValue("Password/StorageMode", mode);
}

int ConfigManager::getPasswordCacheTimeout() const
{
    return getValue("Password/CacheTimeout", 30).toInt();
}

void ConfigManager::setPasswordCacheTimeout(int minutes)
{
    setValue("Password/CacheTimeout", minutes);
}

// ========== 网络设置 ==========

bool ConfigManager::getUseProxy() const
{
    return getValue("Network/UseProxy", false).toBool();
}

void ConfigManager::setUseProxy(bool enabled)
{
    setValue("Network/UseProxy", enabled);
}

QString ConfigManager::getProxyHost() const
{
    return getValue("Network/ProxyHost").toString();
}

void ConfigManager::setProxyHost(const QString& host)
{
    setValue("Network/ProxyHost", host);
}

int ConfigManager::getProxyPort() const
{
    return getValue("Network/ProxyPort", 8080).toInt();
}

void ConfigManager::setProxyPort(int port)
{
    setValue("Network/ProxyPort", port);
}

QString ConfigManager::getProxyUsername() const
{
    return getValue("Network/ProxyUsername").toString();
}

void ConfigManager::setProxyUsername(const QString& username)
{
    setValue("Network/ProxyUsername", username);
}

int ConfigManager::getNetworkTimeout() const
{
    return getValue("Network/Timeout", 60).toInt();
}

void ConfigManager::setNetworkTimeout(int seconds)
{
    setValue("Network/Timeout", seconds);
}

// ========== UI设置 ==========

QByteArray ConfigManager::getWindowGeometry() const
{
    return getValue("UI/WindowGeometry").toByteArray();
}

void ConfigManager::setWindowGeometry(const QByteArray& geometry)
{
    setValue("UI/WindowGeometry", geometry);
}

QByteArray ConfigManager::getWindowState() const
{
    return getValue("UI/WindowState").toByteArray();
}

void ConfigManager::setWindowState(const QByteArray& state)
{
    setValue("UI/WindowState", state);
}

int ConfigManager::getLastSelectedRepository() const
{
    return getValue("UI/LastSelectedRepository", -1).toInt();
}

void ConfigManager::setLastSelectedRepository(int repoId)
{
    setValue("UI/LastSelectedRepository", repoId);
}

} // namespace Data
} // namespace ResticGUI
