#ifndef RESTOREOPTIONS_H
#define RESTOREOPTIONS_H

#include <QString>
#include <QStringList>

namespace ResticGUI {
namespace Models {

struct RestoreOptions
{
    enum OverwritePolicy {
        Always,
        Never,
        IfNewer,
        Ask
    };

    QString targetPath;
    QStringList includePaths;
    QStringList excludePaths;
    OverwritePolicy overwritePolicy = Always;
    bool restorePermissions = true;
    bool restoreTimestamps = true;
    bool restoreOwnership = false;
    bool sparse = false;
    bool verify = false;
};

} // namespace Models
} // namespace ResticGUI

#endif
