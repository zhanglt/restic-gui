#include "RestoreManager.h"
#include "ResticWrapper.h"
#include "RepositoryManager.h"
#include "../data/PasswordManager.h"
#include "../utils/Logger.h"
#include <QMutexLocker>

namespace ResticGUI {
namespace Core {

RestoreManager* RestoreManager::s_instance = nullptr;
QMutex RestoreManager::s_instanceMutex;

RestoreManager* RestoreManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new RestoreManager();
        }
    }
    return s_instance;
}

RestoreManager::RestoreManager(QObject* parent)
    : QObject(parent), m_running(false) {}

RestoreManager::~RestoreManager() {}

void RestoreManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "恢复管理器初始化完成");
}

bool RestoreManager::restore(int repoId, const QString& snapshotId, const Models::RestoreOptions& options)
{
    if (m_running) {
        Utils::Logger::instance()->log(Utils::Logger::Warning, "已有恢复任务正在运行");
        return false;
    }

    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    if (repo.id < 0) {
        return false;
    }

    QString password;
    if (!Data::PasswordManager::instance()->getPassword(repoId, password)) {
        return false;
    }

    m_running = true;
    emit restoreStarted();

    Utils::Logger::instance()->log(Utils::Logger::Info,
        QString("开始恢复，快照: %1, 目标: %2").arg(snapshotId).arg(options.targetPath));

    ResticWrapper wrapper;
    connect(&wrapper, &ResticWrapper::progressUpdated,
            this, &RestoreManager::restoreProgress);

    bool success = wrapper.restore(repo, password, snapshotId, options);

    m_running = false;
    emit restoreFinished(success);

    if (success) {
        Utils::Logger::instance()->log(Utils::Logger::Info, "恢复完成");
    } else {
        Utils::Logger::instance()->log(Utils::Logger::Error, "恢复失败");
    }

    return success;
}

void RestoreManager::cancelRestore()
{
    Utils::Logger::instance()->log(Utils::Logger::Warning, "取消恢复");
    // TODO: 实现取消功能
}

bool RestoreManager::mountRepository(int repoId, const QString& mountPoint, const QString& snapshotId)
{
    Models::Repository repo = RepositoryManager::instance()->getRepository(repoId);
    QString password;

    if (!Data::PasswordManager::instance()->getPassword(repoId, password)) {
        return false;
    }

    ResticWrapper wrapper;
    return wrapper.mount(repo, password, mountPoint, snapshotId);
}

bool RestoreManager::unmountRepository(const QString& mountPoint)
{
    ResticWrapper wrapper;
    return wrapper.umount(mountPoint);
}

} // namespace Core
} // namespace ResticGUI
