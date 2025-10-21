#include "BackupTask.h"

namespace ResticGUI {
namespace Models {

QVariantMap BackupTask::toVariantMap() const
{
    QVariantMap map;
    map["id"] = id;
    map["name"] = name;
    map["description"] = description;
    map["repositoryId"] = repositoryId;
    map["sourcePaths"] = sourcePaths;
    map["excludePatterns"] = excludePatterns;
    map["tags"] = tags;
    map["hostname"] = hostname;
    map["options"] = options;
    map["schedule"] = schedule.toVariantMap();
    map["enabled"] = enabled;
    map["createdAt"] = createdAt;
    map["updatedAt"] = updatedAt;
    return map;
}

BackupTask BackupTask::fromVariantMap(const QVariantMap& map)
{
    BackupTask task;
    task.id = map.value("id", -1).toInt();
    task.name = map.value("name").toString();
    task.description = map.value("description").toString();
    task.repositoryId = map.value("repositoryId", -1).toInt();
    task.sourcePaths = map.value("sourcePaths").toStringList();
    task.excludePatterns = map.value("excludePatterns").toStringList();
    task.tags = map.value("tags").toStringList();
    task.hostname = map.value("hostname").toString();
    task.options = map.value("options").toMap();
    task.schedule = Schedule::fromVariantMap(map.value("schedule").toMap());
    task.enabled = map.value("enabled", true).toBool();
    task.createdAt = map.value("createdAt").toDateTime();
    task.updatedAt = map.value("updatedAt").toDateTime();
    return task;
}

} // namespace Models
} // namespace ResticGUI
