#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMutex>

namespace ResticGUI {
namespace Data {

/**
 * @brief 配置管理器（单例模式）
 *
 * 使用QSettings管理应用程序配置
 * 支持应用级配置和用户级配置
 */
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static ConfigManager* instance();

    /**
     * @brief 初始化配置管理器
     */
    void initialize();

    // ========== 通用配置 ==========

    /**
     * @brief 获取配置值
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置配置值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 检查配置键是否存在
     */
    bool contains(const QString& key) const;

    /**
     * @brief 移除配置键
     */
    void remove(const QString& key);

    /**
     * @brief 同步配置到磁盘
     */
    void sync();

    // ========== 应用程序设置 ==========

    /**
     * @brief 获取restic可执行文件路径
     */
    QString getResticPath() const;
    void setResticPath(const QString& path);

    /**
     * @brief 获取语言设置
     */
    QString getLanguage() const;
    void setLanguage(const QString& lang);

    /**
     * @brief 获取主题设置
     */
    QString getTheme() const;
    void setTheme(const QString& theme);

    /**
     * @brief 获取自动启动设置
     */
    bool getAutoStart() const;
    void setAutoStart(bool enabled);

    /**
     * @brief 获取最小化到系统托盘设置
     */
    bool getMinimizeToTray() const;
    void setMinimizeToTray(bool enabled);

    /**
     * @brief 获取启动时检查更新设置
     */
    bool getCheckUpdatesOnStartup() const;
    void setCheckUpdatesOnStartup(bool enabled);

    // ========== 备份设置 ==========

    /**
     * @brief 获取默认排除模式
     */
    QStringList getDefaultExcludePatterns() const;
    void setDefaultExcludePatterns(const QStringList& patterns);

    /**
     * @brief 获取备份进度通知设置
     */
    bool getShowBackupNotifications() const;
    void setShowBackupNotifications(bool enabled);

    /**
     * @brief 获取并行备份任务数量
     */
    int getMaxParallelBackups() const;
    void setMaxParallelBackups(int count);

    /**
     * @brief 获取备份日志保留天数
     */
    int getLogRetentionDays() const;
    void setLogRetentionDays(int days);

    // ========== 密码设置 ==========

    /**
     * @brief 获取密码存储模式
     * @return 0=不存储, 1=会话缓存, 2=加密存储
     */
    int getPasswordStorageMode() const;
    void setPasswordStorageMode(int mode);

    /**
     * @brief 获取密码缓存超时（分钟）
     */
    int getPasswordCacheTimeout() const;
    void setPasswordCacheTimeout(int minutes);

    // ========== 网络设置 ==========

    /**
     * @brief 获取代理设置
     */
    bool getUseProxy() const;
    void setUseProxy(bool enabled);

    QString getProxyHost() const;
    void setProxyHost(const QString& host);

    int getProxyPort() const;
    void setProxyPort(int port);

    QString getProxyUsername() const;
    void setProxyUsername(const QString& username);

    /**
     * @brief 获取网络超时设置（秒）
     */
    int getNetworkTimeout() const;
    void setNetworkTimeout(int seconds);

    // ========== UI设置 ==========

    /**
     * @brief 获取窗口几何信息
     */
    QByteArray getWindowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);

    /**
     * @brief 获取窗口状态
     */
    QByteArray getWindowState() const;
    void setWindowState(const QByteArray& state);

    /**
     * @brief 获取上次选中的仓库ID
     */
    int getLastSelectedRepository() const;
    void setLastSelectedRepository(int repoId);

signals:
    /**
     * @brief 配置改变信号
     */
    void configChanged(const QString& key, const QVariant& value);

private:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager* s_instance;
    static QMutex s_instanceMutex;

    QSettings* m_settings;
    mutable QMutex m_mutex;
};

} // namespace Data
} // namespace ResticGUI

#endif // CONFIGMANAGER_H
