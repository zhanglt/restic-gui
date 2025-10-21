/**
 * @file Repository.h
 * @brief 仓库数据模型
 */

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <QString>
#include <QDateTime>
#include <QVariantMap>

namespace ResticGUI {
namespace Models {

/**
 * @brief 仓库类型枚举
 */
enum class RepositoryType {
    Local,      ///< 本地文件系统
    SFTP,       ///< SFTP
    S3,         ///< Amazon S3 或兼容服务
    REST,       ///< REST Server
    Azure,      ///< Azure Blob Storage
    GS,         ///< Google Cloud Storage
    B2,         ///< Backblaze B2
    Rclone      ///< Rclone
};

/**
 * @brief 仓库信息结构
 */
struct Repository
{
    int id = -1;                    ///< 数据库ID
    QString name;                   ///< 仓库名称（GUI显示）
    RepositoryType type;            ///< 仓库类型
    QString path;                   ///< 路径或连接字符串
    QVariantMap config;             ///< 类型特定配置（JSON）
    QString passwordHash;           ///< 加密密码（可选）
    QDateTime createdAt;            ///< 创建时间
    QDateTime lastBackup;           ///< 最后备份时间
    bool isDefault = false;         ///< 是否为默认仓库

    /**
     * @brief 获取显示名称
     */
    QString displayName() const;

    /**
     * @brief 获取类型显示名称
     */
    QString typeDisplayName() const;

    /**
     * @brief 检查仓库配置是否有效
     */
    bool isValid() const;

    /**
     * @brief 构建restic连接字符串
     */
    QString buildConnectionString() const;

    /**
     * @brief 转换为QVariantMap（用于JSON序列化）
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从QVariantMap创建（用于JSON反序列化）
     */
    static Repository fromVariantMap(const QVariantMap& map);

    /**
     * @brief 类型枚举转字符串
     */
    static QString typeToString(RepositoryType type);

    /**
     * @brief 字符串转类型枚举
     */
    static RepositoryType stringToType(const QString& str);
};

} // namespace Models
} // namespace ResticGUI

#endif // REPOSITORY_H
