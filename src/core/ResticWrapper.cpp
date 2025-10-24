/**
 * @file ResticWrapper.cpp
 * @brief Restic命令行工具包装器实现
 */

#include "ResticWrapper.h"
#include "../utils/Logger.h"
#include "../data/ConfigManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QFileInfo>

namespace ResticGUI {
namespace Core {

ResticWrapper::ResticWrapper(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_cancelled(false)
{
    m_resticPath = Data::ConfigManager::instance()->getResticPath();
}

ResticWrapper::~ResticWrapper()
{
    if (m_process) {
        if (m_process->state() == QProcess::Running) {
            m_process->kill();
            m_process->waitForFinished(3000);
        }
        delete m_process;
    }
}

void ResticWrapper::setResticPath(const QString& path)
{
    m_resticPath = path;
    Data::ConfigManager::instance()->setResticPath(path);
}

QString ResticWrapper::getVersion()
{
    QString output;
    if (executeCommand(QStringList() << "version", output)) {
        return output.trimmed();
    }
    return QString();
}

void ResticWrapper::cancel()
{
    m_cancelled = true;
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
        Utils::Logger::instance()->log(Utils::Logger::Warning, "Restic操作已取消");
    }
}

// ========== 仓库操作 ==========

bool ResticWrapper::initRepository(const Models::Repository& repo, const QString& password)
{
    QStringList args;
    args << "init";

    QString output;

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("初始化仓库: %1").arg(repo.name));

    if (executeCommand(args, output, true, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库初始化成功");
        return true;
    }

    return false;
}

bool ResticWrapper::checkRepository(const Models::Repository& repo, const QString& password, bool readData)
{
    QStringList args;
    args << "check";

    if (readData) {
        args << "--read-data";
    }

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("检查仓库: %1").arg(repo.name));

    if (executeCommand(args, output, true, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库检查完成");
        return true;
    }

    return false;
}

bool ResticWrapper::repairRepository(const Models::Repository& repo, const QString& password)
{
    QStringList args;
    args << "repair" << "index";

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("修复仓库索引: %1").arg(repo.name));

    if (executeCommand(args, output, true, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库索引修复完成");

        // 修复快照
        args.clear();
        args << "repair" << "snapshots";

        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("修复仓库快照: %1").arg(repo.name));

        if (executeCommand(args, output, true, password, &repo)) {
            Utils::Logger::instance()->log(Utils::Logger::Info, "仓库快照修复完成");
            return true;
        }
    }

    return false;
}

bool ResticWrapper::unlockRepository(const Models::Repository& repo, const QString& password)
{
    QStringList args;
    args << "unlock";

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("解锁仓库: %1").arg(repo.name));

    if (executeCommand(args, output, true, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库已解锁");
        return true;
    }

    return false;
}

bool ResticWrapper::getStats(const Models::Repository& repo, const QString& password, Models::RepoStats& stats)
{
    QStringList args;
    args << "stats" << "--json";

    QString output;
    if (!executeCommand(args, output, true, password, &repo)) {
        return false;
    }

    stats = parseStatsJson(output);
    return true;
}

bool ResticWrapper::prune(const Models::Repository& repo, const QString& password,
                         int keepLast, int keepDaily, int keepWeekly,
                         int keepMonthly, int keepYearly)
{
    // 注意：保留策略参数应该用于 forget 命令，而不是 prune 命令
    // 正确的做法是使用 forget --prune 来一次性完成标记删除和清理操作
    QStringList args;
    args << "forget" << "--prune";

    if (keepLast > 0) {
        args << "--keep-last" << QString::number(keepLast);
    }
    if (keepDaily > 0) {
        args << "--keep-daily" << QString::number(keepDaily);
    }
    if (keepWeekly > 0) {
        args << "--keep-weekly" << QString::number(keepWeekly);
    }
    if (keepMonthly > 0) {
        args << "--keep-monthly" << QString::number(keepMonthly);
    }
    if (keepYearly > 0) {
        args << "--keep-yearly" << QString::number(keepYearly);
    }

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("维护仓库: %1").arg(repo.name));

    if (executeCommandWithProgress(args, output, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "仓库维护完成");
        return true;
    }

    return false;
}

// ========== 备份操作 ==========

bool ResticWrapper::backup(const Models::Repository& repo, const QString& password,
                          const QStringList& sourcePaths,
                          const QStringList& excludePatterns,
                          const QStringList& tags,
                          Models::BackupResult& result)
{
    QStringList args;
    args << "backup" << "--json";

    // 添加源路径
    args << sourcePaths;

    // 添加排除模式
    for (const QString& pattern : excludePatterns) {
        args << "--exclude" << pattern;
    }

    // 添加标签
    for (const QString& tag : tags) {
        args << "--tag" << tag;
    }

    result.startTime = QDateTime::currentDateTime();

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始备份，源路径数: %1").arg(sourcePaths.size()));

    bool success = executeCommandWithProgress(args, output, password, &repo);

    result.endTime = QDateTime::currentDateTime();
    result.status = success ? Models::BackupStatus::Success : Models::BackupStatus::Failed;

    if (success) {
        result = parseBackupResultJson(output);
        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("备份完成，快照ID: %1").arg(result.snapshotId));
    }

    return success;
}

// ========== 快照操作 ==========

bool ResticWrapper::listSnapshots(const Models::Repository& repo, const QString& password,
                                 QList<Models::Snapshot>& snapshots)
{
    QStringList args;
    args << "snapshots" << "--json";

    QString output;
    if (!executeCommand(args, output, true, password, &repo)) {
        return false;
    }

    snapshots = parseSnapshotsJson(output);
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("获取快照列表成功，数量: %1").arg(snapshots.size()));

    return true;
}

bool ResticWrapper::getSnapshotInfo(const Models::Repository& repo, const QString& password,
                                   const QString& snapshotId, Models::Snapshot& snapshot)
{
    QStringList args;
    args << "snapshots" << snapshotId << "--json";

    QString output;
    if (!executeCommand(args, output, true, password, &repo)) {
        return false;
    }

    QList<Models::Snapshot> snapshots = parseSnapshotsJson(output);
    if (!snapshots.isEmpty()) {
        snapshot = snapshots.first();
        return true;
    }

    return false;
}

bool ResticWrapper::listFiles(const Models::Repository& repo, const QString& password,
                             const QString& snapshotId, const QString& path,
                             QList<Models::FileInfo>& files)
{
    QStringList args;
    args << "ls" << snapshotId << "--json";

    if (!path.isEmpty()) {
        args << path;
    }

    QString output;
    if (!executeCommand(args, output, true, password, &repo)) {
        return false;
    }

    files = parseFilesJson(output);
    return true;
}

bool ResticWrapper::deleteSnapshots(const Models::Repository& repo, const QString& password,
                                   const QStringList& snapshotIds)
{
    QStringList args;
    args << "forget";
    args << snapshotIds;
    args << "--prune"; // 同时清理数据

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("删除快照，数量: %1").arg(snapshotIds.size()));

    if (executeCommandWithProgress(args, output, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "快照已删除");
        return true;
    }

    return false;
}

// ========== 恢复操作 ==========

bool ResticWrapper::restore(const Models::Repository& repo, const QString& password,
                           const QString& snapshotId, const Models::RestoreOptions& options)
{
    QStringList args;
    args << "restore" << snapshotId;
    args << "--target" << options.targetPath;

    // 添加包含路径
    for (const QString& include : options.includePaths) {
        args << "--include" << include;
    }

    // 添加排除路径
    for (const QString& exclude : options.excludePaths) {
        args << "--exclude" << exclude;
    }

    if (options.verify) {
        args << "--verify";
    }

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始恢复，快照: %1, 目标: %2").arg(snapshotId).arg(options.targetPath));

    if (executeCommandWithProgress(args, output, password, &repo)) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "恢复完成");
        return true;
    }

    return false;
}

// ========== 挂载操作 ==========

bool ResticWrapper::mount(const Models::Repository& repo, const QString& password,
                         const QString& mountPoint, const QString& snapshotId)
{
    QStringList args;
    args << "mount" << mountPoint;

    if (!snapshotId.isEmpty()) {
        args << snapshotId;
    }

    QString output;
    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("挂载仓库到: %1").arg(mountPoint));

    // mount命令需要在后台运行
    // TODO: 需要特殊处理，在后台进程中运行
    return executeCommand(args, output, true, password, &repo);
}

bool ResticWrapper::umount(const QString& mountPoint)
{
#ifdef Q_OS_WIN
    // Windows不支持挂载
    Utils::Logger::instance()->log(Utils::Logger::Error, "Windows不支持挂载功能");
    return false;
#else
    QProcess umountProcess;
    umountProcess.start("umount", QStringList() << mountPoint);
    umountProcess.waitForFinished();

    return umountProcess.exitCode() == 0;
#endif
}

// ========== 私有辅助函数 ==========

bool ResticWrapper::executeCommand(const QStringList& args, QString& output,
                                  bool usePassword, const QString& password,
                                  const Models::Repository* repo)
{
    m_cancelled = false;

    // 检查 restic 可执行文件是否存在
    if (m_resticPath.isEmpty()) {
        QString error = "Restic 可执行文件路径未设置。请在设置中配置 Restic 路径。";
        Utils::Logger::instance()->log(Utils::Logger::Error, error);
        emit commandError(error);
        return false;
    }

    QFileInfo resticFile(m_resticPath);
    if (!resticFile.exists()) {
        QString error = QString("找不到 Restic 可执行文件: %1\n请在设置中配置正确的 Restic 路径。").arg(m_resticPath);
        Utils::Logger::instance()->log(Utils::Logger::Error, error);
        emit commandError(error);
        return false;
    }

    if (!resticFile.isExecutable()) {
        QString error = QString("文件不可执行: %1\n请检查文件权限。").arg(m_resticPath);
        Utils::Logger::instance()->log(Utils::Logger::Error, error);
        emit commandError(error);
        return false;
    }

    // 创建进程
    if (m_process) {
        delete m_process;
    }
    m_process = new QProcess(this);

    // 设置环境变量
    QProcessEnvironment env;
    if (repo) {
        // 如果提供了仓库信息，使用buildEnvironment构建完整的环境变量
        env = buildEnvironment(*repo, password);
    } else {
        // 否则只设置密码
        env = QProcessEnvironment::systemEnvironment();
        if (usePassword && !password.isEmpty()) {
            env.insert("RESTIC_PASSWORD", password);
        }
    }
    m_process->setProcessEnvironment(env);

    // 连接信号
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ResticWrapper::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &ResticWrapper::onReadyReadStandardError);

    // 启动进程
    m_currentOutput.clear();
    m_currentError.clear();

    QString command = m_resticPath + " " + args.join(" ");
    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("执行命令: %1").arg(command));

    emit commandStarted(command);

    m_process->start(m_resticPath, args);

    if (!m_process->waitForStarted()) {
        QString error = m_process->errorString();
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("无法启动restic: %1").arg(error));
        emit commandError(error);
        return false;
    }

    // 等待进程完成（最长等待时间：1小时）
    if (!m_process->waitForFinished(3600000)) {
        if (!m_cancelled) {
            QString error = "命令执行超时";
            Utils::Logger::instance()->log(Utils::Logger::Error, error);
            emit commandError(error);
            m_process->kill();
        }
        return false;
    }

    int exitCode = m_process->exitCode();
    output = m_currentOutput;

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("命令完成，退出码: %1").arg(exitCode));

    emit commandFinished(exitCode, output);

    if (exitCode != 0) {
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("命令执行失败: %1").arg(m_currentError));
        return false;
    }

    return true;
}

bool ResticWrapper::executeCommandWithProgress(const QStringList& args, QString& output,
                                              const QString& password,
                                              const Models::Repository* repo)
{
    // 与executeCommand类似，但解析进度信息
    // 为了简化，这里直接调用executeCommand
    // TODO: 实现实时进度解析
    return executeCommand(args, output, !password.isEmpty(), password, repo);
}

QProcessEnvironment ResticWrapper::buildEnvironment(const Models::Repository& repo,
                                                   const QString& password)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // 设置仓库路径
    QString repoPath = repo.buildConnectionString();
    env.insert("RESTIC_REPOSITORY", repoPath);

    // 设置密码
    if (!password.isEmpty()) {
        env.insert("RESTIC_PASSWORD", password);
    }

    // 根据仓库类型设置额外的环境变量
    if (repo.type == Models::RepositoryType::SFTP) {
        // SFTP相关配置
    } else if (repo.type == Models::RepositoryType::S3) {
        // S3相关配置
        if (repo.config.contains("aws_access_key_id")) {
            env.insert("AWS_ACCESS_KEY_ID", repo.config["aws_access_key_id"].toString());
        }
        if (repo.config.contains("aws_secret_access_key")) {
            env.insert("AWS_SECRET_ACCESS_KEY", repo.config["aws_secret_access_key"].toString());
        }
    }
    // 其他仓库类型...

    return env;
}

QVariant ResticWrapper::parseJsonOutput(const QString& output)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        Utils::Logger::instance()->log(Utils::Logger::Error,
            QString("JSON解析错误: %1").arg(error.errorString()));
        return QVariant();
    }

    if (doc.isObject()) {
        return doc.object().toVariantMap();
    } else if (doc.isArray()) {
        return doc.array().toVariantList();
    }

    return QVariant();
}

QList<Models::Snapshot> ResticWrapper::parseSnapshotsJson(const QString& json)
{
    QList<Models::Snapshot> snapshots;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isArray()) {
        return snapshots;
    }

    QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject obj = value.toObject();
        Models::Snapshot snapshot;

        snapshot.id = obj["id"].toString();
        snapshot.time = QDateTime::fromString(obj["time"].toString(), Qt::ISODate);
        snapshot.hostname = obj["hostname"].toString();
        snapshot.username = obj["username"].toString();

        // 解析paths数组
        QJsonArray pathsArray = obj["paths"].toArray();
        for (const QJsonValue& pathValue : pathsArray) {
            snapshot.paths.append(pathValue.toString());
        }

        // 解析tags数组
        QJsonArray tagsArray = obj["tags"].toArray();
        for (const QJsonValue& tagValue : tagsArray) {
            snapshot.tags.append(tagValue.toString());
        }

        snapshot.parent = obj["parent"].toString();

        snapshots.append(snapshot);
    }

    return snapshots;
}

QList<Models::FileInfo> ResticWrapper::parseFilesJson(const QString& json)
{
    QList<Models::FileInfo> files;

    // 解析ls命令的JSON输出
    // restic ls输出是每行一个JSON对象
    QStringList lines = json.split('\n', Qt::SkipEmptyParts);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("parseFilesJson: 共 %1 行JSON数据").arg(lines.size()));

    for (const QString& line : lines) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            Utils::Logger::instance()->log(Utils::Logger::Debug,
                QString("JSON解析失败: %1").arg(error.errorString()));
            continue;
        }

        QJsonObject obj = doc.object();
        Models::FileInfo file;

        file.name = obj["name"].toString();
        file.path = obj["path"].toString();

        // 解析类型
        QString typeStr = obj["type"].toString();
        if (typeStr == "dir") {
            file.type = Models::FileType::Directory;
        } else if (typeStr == "symlink") {
            file.type = Models::FileType::Symlink;
        } else {
            file.type = Models::FileType::File;
        }

        file.size = obj["size"].toVariant().toLongLong();
        file.mode = QString::number(obj["mode"].toInt(), 8); // 转换为八进制字符串
        file.mtime = QDateTime::fromString(obj["mtime"].toString(), Qt::ISODate);
        file.uid = obj["uid"].toInt();
        file.gid = obj["gid"].toInt();
        file.user = obj["user"].toString();
        file.group = obj["group"].toString();

        Utils::Logger::instance()->log(Utils::Logger::Debug,
            QString("解析文件: name=%1, type=%2, path=%3")
                .arg(file.name).arg(typeStr).arg(file.path));

        files.append(file);
    }

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("parseFilesJson: 成功解析 %1 个文件").arg(files.size()));

    return files;
}

Models::RepoStats ResticWrapper::parseStatsJson(const QString& json)
{
    Models::RepoStats stats;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return stats;
    }

    QJsonObject obj = doc.object();

    stats.totalSize = obj["total_size"].toVariant().toLongLong();
    stats.totalFileCount = obj["total_file_count"].toVariant().toLongLong();
    stats.snapshotCount = obj["snapshots_count"].toInt();

    return stats;
}

Models::BackupResult ResticWrapper::parseBackupResultJson(const QString& json)
{
    Models::BackupResult result;

    // restic backup输出多行JSON，最后一行是summary
    QStringList lines = json.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        QJsonObject obj = doc.object();

        // 检查是否是summary消息
        if (obj["message_type"].toString() == "summary") {
            result.snapshotId = obj["snapshot_id"].toString();
            result.filesNew = obj["files_new"].toVariant().toULongLong();
            result.filesChanged = obj["files_changed"].toVariant().toULongLong();
            result.filesUnmodified = obj["files_unmodified"].toVariant().toULongLong();
            result.dirsNew = obj["dirs_new"].toVariant().toULongLong();
            result.dirsChanged = obj["dirs_changed"].toVariant().toULongLong();
            result.dirsUnmodified = obj["dirs_unmodified"].toVariant().toULongLong();
            result.dataAdded = obj["data_added"].toVariant().toULongLong();
            result.totalFilesProcessed = obj["total_files_processed"].toVariant().toULongLong();
            result.totalBytesProcessed = obj["total_bytes_processed"].toVariant().toULongLong();
            result.status = Models::BackupStatus::Success;
        }
    }

    return result;
}

void ResticWrapper::parseProgressJson(const QString& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    QString messageType = obj["message_type"].toString();

    if (messageType == "status") {
        double percentDone = obj["percent_done"].toDouble();
        quint64 bytesProcessed = obj["bytes_done"].toVariant().toULongLong();
        quint64 totalBytes = obj["total_bytes"].toVariant().toULongLong();

        emit progressUpdated(static_cast<int>(percentDone * 100), QString());
        emit backupProgress(0, bytesProcessed, 0, totalBytes);
    }
}

// ========== 槽函数 ==========

void ResticWrapper::onReadyReadStandardOutput()
{
    if (!m_process) {
        return;
    }

    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    m_currentOutput += output;

    // 尝试解析进度信息
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.trimmed().startsWith('{')) {
            parseProgressJson(line);
        }
    }

    emit standardOutput(output);
}

void ResticWrapper::onReadyReadStandardError()
{
    if (!m_process) {
        return;
    }

    QString error = QString::fromUtf8(m_process->readAllStandardError());
    m_currentError += error;

    emit standardError(error);
}

void ResticWrapper::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("进程完成，退出码: %1").arg(exitCode));
}

void ResticWrapper::onProcessError(QProcess::ProcessError error)
{
    QString errorStr;
    switch (error) {
    case QProcess::FailedToStart:
        errorStr = "进程启动失败";
        break;
    case QProcess::Crashed:
        errorStr = "进程崩溃";
        break;
    case QProcess::Timedout:
        errorStr = "进程超时";
        break;
    case QProcess::WriteError:
        errorStr = "写入错误";
        break;
    case QProcess::ReadError:
        errorStr = "读取错误";
        break;
    default:
        errorStr = "未知错误";
        break;
    }

    Utils::Logger::instance()->log(Utils::Logger::Error,
        QString("进程错误: %1").arg(errorStr));

    emit commandError(errorStr);
}

} // namespace Core
} // namespace ResticGUI
