#ifndef REPOSTATS_H
#define REPOSTATS_H

#include <QtGlobal>

namespace ResticGUI {
namespace Models {

struct RepoStats
{
    qint64 totalSize = 0;
    qint64 totalFileCount = 0;
    qint64 uniqueSize = 0;
    double compressionRatio = 0.0;
    int snapshotCount = 0;
};

} // namespace Models
} // namespace ResticGUI

#endif
