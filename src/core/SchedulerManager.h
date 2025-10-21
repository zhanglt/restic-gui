#ifndef SCHEDULERMANAGER_H
#define SCHEDULERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMap>

namespace ResticGUI {
namespace Core {

class SchedulerManager : public QObject
{
    Q_OBJECT

public:
    static SchedulerManager* instance();
    void initialize();

    // 调度控制
    void start();
    void stop();
    bool isRunning() const { return m_running; }

    // 手动触发检查
    void checkScheduledTasks();

signals:
    void taskScheduled(int taskId, const QDateTime& nextRun);
    void taskTriggered(int taskId);

private slots:
    void onTimerTimeout();
    void checkAndRunTasks();

private:
    explicit SchedulerManager(QObject* parent = nullptr);
    ~SchedulerManager();
    SchedulerManager(const SchedulerManager&) = delete;
    SchedulerManager& operator=(const SchedulerManager&) = delete;

    QDateTime calculateNextRun(int taskId);

    static SchedulerManager* s_instance;
    static QMutex s_instanceMutex;

    QTimer* m_timer;
    QMap<int, QDateTime> m_nextRunTimes; // taskId -> nextRunTime
    bool m_running;
    mutable QMutex m_mutex;
};

} // namespace Core
} // namespace ResticGUI

#endif // SCHEDULERMANAGER_H
