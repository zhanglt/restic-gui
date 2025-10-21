/**
 * @file Repository.cpp
 * @brief 仓库数据模型实现
 */

#include "Repository.h"
#include <QJsonObject>
#include <QJsonDocument>

namespace ResticGUI {
namespace Models {

QString Repository::displayName() const
{
    return name.isEmpty() ? path : name;
}

QString Repository::typeDisplayName() const
{
    return typeToString(type);
}

bool Repository::isValid() const
{
    return !name.isEmpty() && !path.isEmpty();
}

QString Repository::buildConnectionString() const
{
    switch (type) {
    case RepositoryType::Local:
        return path;

    case RepositoryType::SFTP: {
        QString host = config.value("host").toString();
        int port = config.value("port", 22).toInt();
        QString user = config.value("user").toString();
        QString remotePath = config.value("path", path).toString();

        if (port != 22) {
            return QString("sftp://%1@%2:%3/%4").arg(user, host).arg(port).arg(remotePath);
        }
        return QString("sftp:%1@%2:%3").arg(user, host, remotePath);
    }

    case RepositoryType::S3: {
        QString endpoint = config.value("endpoint", "s3.amazonaws.com").toString();
        QString bucket = config.value("bucket", path).toString();
        return QString("s3:%1/%2").arg(endpoint, bucket);
    }

    case RepositoryType::REST:
        return QString("rest:%1").arg(path);

    case RepositoryType::Azure:
        return QString("azure:%1").arg(path);

    case RepositoryType::GS:
        return QString("gs:%1").arg(path);

    case RepositoryType::B2:
        return QString("b2:%1").arg(path);

    case RepositoryType::Rclone:
        return QString("rclone:%1").arg(path);

    default:
        return path;
    }
}

QVariantMap Repository::toVariantMap() const
{
    QVariantMap map;
    map["id"] = id;
    map["name"] = name;
    map["type"] = typeToString(type);
    map["path"] = path;
    map["config"] = config;
    map["passwordHash"] = passwordHash;
    map["createdAt"] = createdAt;
    map["lastBackup"] = lastBackup;
    map["isDefault"] = isDefault;
    return map;
}

Repository Repository::fromVariantMap(const QVariantMap& map)
{
    Repository repo;
    repo.id = map.value("id", -1).toInt();
    repo.name = map.value("name").toString();
    repo.type = stringToType(map.value("type").toString());
    repo.path = map.value("path").toString();
    repo.config = map.value("config").toMap();
    repo.passwordHash = map.value("passwordHash").toString();
    repo.createdAt = map.value("createdAt").toDateTime();
    repo.lastBackup = map.value("lastBackup").toDateTime();
    repo.isDefault = map.value("isDefault", false).toBool();
    return repo;
}

QString Repository::typeToString(RepositoryType type)
{
    switch (type) {
    case RepositoryType::Local:  return "local";
    case RepositoryType::SFTP:   return "sftp";
    case RepositoryType::S3:     return "s3";
    case RepositoryType::REST:   return "rest";
    case RepositoryType::Azure:  return "azure";
    case RepositoryType::GS:     return "gs";
    case RepositoryType::B2:     return "b2";
    case RepositoryType::Rclone: return "rclone";
    default: return "local";
    }
}

RepositoryType Repository::stringToType(const QString& str)
{
    if (str == "local")  return RepositoryType::Local;
    if (str == "sftp")   return RepositoryType::SFTP;
    if (str == "s3")     return RepositoryType::S3;
    if (str == "rest")   return RepositoryType::REST;
    if (str == "azure")  return RepositoryType::Azure;
    if (str == "gs")     return RepositoryType::GS;
    if (str == "b2")     return RepositoryType::B2;
    if (str == "rclone") return RepositoryType::Rclone;
    return RepositoryType::Local;
}

} // namespace Models
} // namespace ResticGUI
