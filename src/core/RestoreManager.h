#ifndef RESTOREMANAGER_H
#define RESTOREMANAGER_H

#include <QObject>
#include <QMutex>
#include "../models/RestoreOptions.h"

namespace ResticGUI {
namespace Core {

class RestoreManager : public QObject
{
    Q_OBJECT

public:
    static RestoreManager* instance();
    void initialize();

    // 恢复操作
    bool restore(int repoId, const QString& snapshotId, const Models::RestoreOptions& options);
    void cancelRestore();

    // 挂载操作（仅Linux/macOS）
    bool mountRepository(int repoId, const QString& mountPoint, const QString& snapshotId = QString());
    bool unmountRepository(const QString& mountPoint);

signals:
    void restoreStarted();
    void restoreProgress(int percent, const QString& message);
    void restoreFinished(bool success);
    void restoreError(const QString& error);

private:
    explicit RestoreManager(QObject* parent = nullptr);
    ~RestoreManager();
    RestoreManager(const RestoreManager&) = delete;
    RestoreManager& operator=(const RestoreManager&) = delete;

    static RestoreManager* s_instance;
    static QMutex s_instanceMutex;
    mutable QMutex m_mutex;
    bool m_running;
};

} // namespace Core
} // namespace ResticGUI

#endif // RESTOREMANAGER_H
