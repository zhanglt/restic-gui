/**
 * @file BackupTask.h
 * @brief 备份任务数据模型
 */

#ifndef BACKUPTASK_H
#define BACKUPTASK_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include "Repository.h"
#include "Schedule.h"

namespace ResticGUI {
namespace Models {

struct BackupTask
{
    int id = -1;
    QString name;
    QString description;
    int repositoryId = -1;
    QStringList sourcePaths;
    QStringList excludePatterns;
    QStringList tags;
    QString hostname;
    QVariantMap options;
    Schedule schedule;
    bool enabled = true;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime lastRun;
    QDateTime nextRun;

    // 运行时填充
    Repository repository;

    bool isValid() const { return !name.isEmpty() && repositoryId > 0; }
    QVariantMap toVariantMap() const;
    static BackupTask fromVariantMap(const QVariantMap& map);
};

} // namespace Models
} // namespace ResticGUI

#endif // BACKUPTASK_H
