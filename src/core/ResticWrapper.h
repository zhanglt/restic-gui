#ifndef RESTICWRAPPER_H
#define RESTICWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include "../models/Repository.h"
#include "../models/Snapshot.h"
#include "../models/FileInfo.h"
#include "../models/BackupResult.h"
#include "../models/BackupTask.h"
#include "../models/RestoreOptions.h"
#include "../models/RepoStats.h"

namespace ResticGUI {
namespace Core {

/**
 * @brief Restic命令行工具包装器
 *
 * 提供对restic所有命令的封装，使用QProcess执行命令
 * 支持JSON输出解析和进度监控
 */
class ResticWrapper : public QObject
{
    Q_OBJECT

public:
    explicit ResticWrapper(QObject* parent = nullptr);
    ~ResticWrapper();

    /**
     * @brief 设置restic可执行文件路径
     */
    void setResticPath(const QString& path);

    /**
     * @brief 获取restic版本信息
     */
    QString getVersion();

    /**
     * @brief 取消当前操作
     */
    void cancel();

    // ========== 仓库操作 ==========

    /**
     * @brief 初始化仓库
     * @param repo 仓库信息
     * @param password 仓库密码
     * @return 成功返回true
     */
    bool initRepository(const Models::Repository& repo, const QString& password);

    /**
     * @brief 检查仓库
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param readData 是否读取数据包（更彻底的检查）
     * @return 成功返回true
     */
    bool checkRepository(const Models::Repository& repo, const QString& password, bool readData = false);

    /**
     * @brief 修复仓库
     * @param repo 仓库信息
     * @param password 仓库密码
     * @return 成功返回true
     */
    bool repairRepository(const Models::Repository& repo, const QString& password);

    /**
     * @brief 解锁仓库
     * @param repo 仓库信息
     * @param password 仓库密码
     */
    bool unlockRepository(const Models::Repository& repo, const QString& password);

    /**
     * @brief 获取仓库统计信息
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param stats 输出参数，统计信息
     * @return 成功返回true
     */
    bool getStats(const Models::Repository& repo, const QString& password, Models::RepoStats& stats);

    /**
     * @brief 维护仓库（prune）
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param keepLast 保留最近N个快照
     * @param keepDaily 保留每日快照N天
     * @param keepWeekly 保留每周快照N周
     * @param keepMonthly 保留每月快照N月
     * @param keepYearly 保留每年快照N年
     * @return 成功返回true
     */
    bool prune(const Models::Repository& repo, const QString& password,
               int keepLast = 0, int keepDaily = 0, int keepWeekly = 0,
               int keepMonthly = 0, int keepYearly = 0);

    // ========== 备份操作 ==========

    /**
     * @brief 执行备份
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param task 备份任务（包含源路径、排除规则等）
     * @param result 输出参数，备份结果
     * @return 成功返回true
     */
    bool backup(const Models::Repository& repo, const QString& password,
                const Models::BackupTask& task,
                Models::BackupResult& result);

    // ========== 快照操作 ==========

    /**
     * @brief 获取快照列表
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param snapshots 输出参数，快照列表
     * @return 成功返回true
     */
    bool listSnapshots(const Models::Repository& repo, const QString& password,
                      QList<Models::Snapshot>& snapshots);

    /**
     * @brief 获取快照详细信息
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param snapshotId 快照ID
     * @param snapshot 输出参数，快照信息
     * @return 成功返回true
     */
    bool getSnapshotInfo(const Models::Repository& repo, const QString& password,
                        const QString& snapshotId, Models::Snapshot& snapshot);

    /**
     * @brief 列出快照中的文件
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param snapshotId 快照ID
     * @param path 路径（默认为根路径）
     * @param files 输出参数，文件列表
     * @return 成功返回true
     */
    bool listFiles(const Models::Repository& repo, const QString& password,
                  const QString& snapshotId, const QString& path,
                  QList<Models::FileInfo>& files);

    /**
     * @brief 删除快照
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param snapshotIds 要删除的快照ID列表
     * @return 成功返回true
     */
    bool deleteSnapshots(const Models::Repository& repo, const QString& password,
                        const QStringList& snapshotIds);

    // ========== 恢复操作 ==========

    /**
     * @brief 恢复数据
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param snapshotId 快照ID
     * @param options 恢复选项
     * @return 成功返回true
     */
    bool restore(const Models::Repository& repo, const QString& password,
                const QString& snapshotId, const Models::RestoreOptions& options);

    // ========== 挂载操作 ==========

    /**
     * @brief 挂载仓库（仅Linux/macOS）
     * @param repo 仓库信息
     * @param password 仓库密码
     * @param mountPoint 挂载点
     * @param snapshotId 快照ID（可选，为空则挂载所有快照）
     * @return 成功返回true
     */
    bool mount(const Models::Repository& repo, const QString& password,
              const QString& mountPoint, const QString& snapshotId = QString());

    /**
     * @brief 卸载仓库
     * @param mountPoint 挂载点
     */
    bool umount(const QString& mountPoint);

signals:
    /**
     * @brief 命令开始执行
     */
    void commandStarted(const QString& command);

    /**
     * @brief 命令执行完成
     */
    void commandFinished(int exitCode, const QString& output);

    /**
     * @brief 命令执行错误
     */
    void commandError(const QString& error);

    /**
     * @brief 标准输出
     */
    void standardOutput(const QString& output);

    /**
     * @brief 标准错误
     */
    void standardError(const QString& error);

    /**
     * @brief 进度更新（百分比：0-100）
     */
    void progressUpdated(int percent, const QString& message);

    /**
     * @brief 备份进度详细信息
     */
    void backupProgress(quint64 filesProcessed, quint64 bytesProcessed,
                       quint64 totalFiles, quint64 totalBytes);

private:
    /**
     * @brief 执行restic命令
     * @param args 命令参数
     * @param output 输出参数，命令输出
     * @param usePassword 是否使用密码
     * @param password 密码
     * @param repo 仓库信息（可选，用于设置RESTIC_REPOSITORY环境变量）
     * @return 成功返回true
     */
    bool executeCommand(const QStringList& args, QString& output,
                       bool usePassword = false, const QString& password = QString(),
                       const Models::Repository* repo = nullptr);

    /**
     * @brief 执行restic命令（带进度监控）
     * @param args 命令参数
     * @param output 输出参数，命令输出
     * @param password 密码
     * @param repo 仓库信息（可选，用于设置RESTIC_REPOSITORY环境变量）
     * @return 成功返回true
     */
    bool executeCommandWithProgress(const QStringList& args, QString& output,
                                   const QString& password = QString(),
                                   const Models::Repository* repo = nullptr);

    /**
     * @brief 构建仓库环境变量
     */
    QProcessEnvironment buildEnvironment(const Models::Repository& repo,
                                        const QString& password = QString());

    /**
     * @brief 解析JSON输出
     */
    QVariant parseJsonOutput(const QString& output);

    /**
     * @brief 解析快照列表JSON
     */
    QList<Models::Snapshot> parseSnapshotsJson(const QString& json);

    /**
     * @brief 解析文件列表JSON
     */
    QList<Models::FileInfo> parseFilesJson(const QString& json);

    /**
     * @brief 解析统计信息JSON
     */
    Models::RepoStats parseStatsJson(const QString& json);

    /**
     * @brief 解析备份结果JSON
     */
    Models::BackupResult parseBackupResultJson(const QString& json);

    /**
     * @brief 解析进度JSON
     */
    void parseProgressJson(const QString& json);

private slots:
    /**
     * @brief 处理进程标准输出
     */
    void onReadyReadStandardOutput();

    /**
     * @brief 处理进程标准错误
     */
    void onReadyReadStandardError();

    /**
     * @brief 处理进程完成
     */
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief 处理进程错误
     */
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess* m_process;
    QString m_resticPath;
    QString m_currentOutput;
    QString m_currentError;
    bool m_cancelled;
};

} // namespace Core
} // namespace ResticGUI

#endif // RESTICWRAPPER_H
