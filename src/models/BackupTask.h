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

    // 高级排除选项
    QString excludeFile;           // 从文件读取排除列表的路径
    QString excludeLargerThan;     // 排除大于此大小的文件 (如 "100M", "1G")
    bool excludeCaches = false;    // 排除包含 CACHEDIR.TAG 的缓存目录
    QString excludeIfPresent;      // 排除包含指定文件的目录

    // 高级包含选项
    QString filesFrom;             // 从文件读取模式列表（支持通配符）
    QString filesFromVerbatim;     // 从文件逐字读取路径列表
    QString filesFromRaw;          // 从文件读取 NUL 分隔的路径列表

    // 高级备份参数
    bool noScan = false;           // 禁用进度扫描
    QString compression;           // 压缩级别: off/fastest/auto/better/max
    bool noExtraVerify = false;    // 禁用额外验证
    int readConcurrency = 0;       // 文件读取并发数 (0表示使用默认值)
    int packSize = 0;              // 包大小(MiB) (0表示使用默认值16)

    // 运行时填充
    Repository repository;

    bool isValid() const { return !name.isEmpty() && repositoryId > 0; }
    QVariantMap toVariantMap() const;
    static BackupTask fromVariantMap(const QVariantMap& map);
};

} // namespace Models
} // namespace ResticGUI

#endif // BACKUPTASK_H
