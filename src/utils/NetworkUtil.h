#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H

#include <QString>

namespace ResticGUI {
namespace Utils {

class NetworkUtil
{
public:
    static bool isNetworkAvailable();
    static bool isHostReachable(const QString& host, int port = 22);

private:
    NetworkUtil() = delete;
};

} // namespace Utils
} // namespace ResticGUI

#endif
