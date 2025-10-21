#include "SchedulerManager.h"
#include "BackupManager.h"
#include "../data/DatabaseManager.h"
#include "../utils/Logger.h"
#include <QMutexLocker>

namespace ResticGUI {
namespace Core {

SchedulerManager* SchedulerManager::s_instance = nullptr;
QMutex SchedulerManager::s_instanceMutex;

SchedulerManager* SchedulerManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new SchedulerManager();
        }
    }
    return s_instance;
}

SchedulerManager::SchedulerManager(QObject* parent)
    : QObject(parent), m_timer(nullptr), m_running(false)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(60000); // 每分钟检查一次
    connect(m_timer, &QTimer::timeout, this, &SchedulerManager::onTimerTimeout);
}

SchedulerManager::~SchedulerManager()
{
    stop();
}

void SchedulerManager::initialize()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "调度管理器初始化完成");

    // 加载所有启用的任务并计算下次运行时间
    QList<Models::BackupTask> tasks = Data::DatabaseManager::instance()->getEnabledBackupTasks();
    for (const Models::BackupTask& task : tasks) {
        QDateTime nextRun = calculateNextRun(task.id);
        m_nextRunTimes[task.id] = nextRun;
    }
}

void SchedulerManager::start()
{
    QMutexLocker locker(&m_mutex);

    if (m_running) {
        return;
    }

    m_running = true;
    m_timer->start();

    Utils::Logger::instance()->log(Utils::Logger::Info, "调度器已启动");
}

void SchedulerManager::stop()
{
    QMutexLocker locker(&m_mutex);

    if (!m_running) {
        return;
    }

    m_running = false;
    m_timer->stop();

    Utils::Logger::instance()->log(Utils::Logger::Info, "调度器已停止");
}

void SchedulerManager::checkScheduledTasks()
{
    checkAndRunTasks();
}

void SchedulerManager::onTimerTimeout()
{
    checkAndRunTasks();
}

void SchedulerManager::checkAndRunTasks()
{
    QMutexLocker locker(&m_mutex);

    QDateTime now = QDateTime::currentDateTime();
    QList<int> tasksToRun;

    // 检查需要运行的任务
    for (auto it = m_nextRunTimes.begin(); it != m_nextRunTimes.end(); ++it) {
        if (it.value() <= now) {
            tasksToRun.append(it.key());
        }
    }

    // 运行任务
    for (int taskId : tasksToRun) {
        Utils::Logger::instance()->log(Utils::Logger::Info,
            QString("触发计划任务，ID: %1").arg(taskId));

        emit taskTriggered(taskId);

        // 在新线程中执行备份
        // TODO: 使用线程池执行
        BackupManager::instance()->runBackupTask(taskId);

        // 计算下次运行时间
        QDateTime nextRun = calculateNextRun(taskId);
        m_nextRunTimes[taskId] = nextRun;

        emit taskScheduled(taskId, nextRun);
    }
}

QDateTime SchedulerManager::calculateNextRun(int taskId)
{
    Models::BackupTask task = BackupManager::instance()->getBackupTask(taskId);
    if (task.id < 0) {
        return QDateTime();
    }

    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextRun = now;

    switch (task.schedule.type) {
    case Models::Schedule::Hourly:
        nextRun = now.addSecs(3600);
        break;

    case Models::Schedule::Daily:
        nextRun = QDateTime(now.date().addDays(1), task.schedule.time);
        if (nextRun <= now) {
            nextRun = nextRun.addDays(1);
        }
        break;

    case Models::Schedule::Weekly:
        nextRun = QDateTime(now.date().addDays((task.schedule.dayOfWeek - now.date().dayOfWeek() + 7) % 7),
                           task.schedule.time);
        if (nextRun <= now) {
            nextRun = nextRun.addDays(7);
        }
        break;

    case Models::Schedule::Monthly:
        // 简化实现：每月同一天
        nextRun = QDateTime(QDate(now.date().year(), now.date().month(), 1).addMonths(1),
                           task.schedule.time);
        break;

    case Models::Schedule::Custom:
        // TODO: 实现cron表达式解析
        break;

    default:
        break;
    }

    return nextRun;
}

} // namespace Core
} // namespace ResticGUI
