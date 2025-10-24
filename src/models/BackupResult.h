#ifndef BACKUPRESULT_H
#define BACKUPRESULT_H

#include <QString>
#include <QDateTime>

namespace ResticGUI {
namespace Models {

enum class BackupStatus
{
    Running,
    Success,
    Failed,
    Cancelled
};

struct BackupResult
{
    int taskId = -1;
    QString taskName; // 任务名称(用于显示)
    bool success = false;
    BackupStatus status = BackupStatus::Running;
    QString snapshotId;
    QDateTime startTime;
    QDateTime endTime;

    // 文件统计
    int filesNew = 0;
    int filesChanged = 0;
    int filesUnmodified = 0;
    int totalFiles = 0;
    int totalFilesProcessed = 0;

    // 目录统计
    int dirsNew = 0;
    int dirsChanged = 0;
    int dirsUnmodified = 0;

    // 数据统计
    qint64 dataAdded = 0;
    qint64 dataProcessed = 0;
    qint64 totalBytes = 0;
    qint64 totalBytesProcessed = 0;

    int duration = 0;
    QString errorMessage;
};

} // namespace Models
} // namespace ResticGUI

#endif
