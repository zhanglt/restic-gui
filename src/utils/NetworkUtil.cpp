#include "NetworkUtil.h"
#include <QNetworkInterface>

namespace ResticGUI {
namespace Utils {

bool NetworkUtil::isNetworkAvailable()
{
    for (const QNetworkInterface& interface : QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return true;
        }
    }
    return false;
}

bool NetworkUtil::isHostReachable(const QString& host, int port)
{
    // 简化实现，实际应使用QTcpSocket测试连接
    Q_UNUSED(host);
    Q_UNUSED(port);
    return isNetworkAvailable();
}

} // namespace Utils
} // namespace ResticGUI
