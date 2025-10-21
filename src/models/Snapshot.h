#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <QString>
#include <QStringList>
#include <QDateTime>

namespace ResticGUI {
namespace Models {

struct Snapshot
{
    QString id;
    QString fullId;
    QDateTime time;
    QString hostname;
    QString username;
    QStringList paths;
    QStringList tags;
    QString parent;
    qint64 size = 0;
    int fileCount = 0;
    int dirCount = 0;
};

} // namespace Models
} // namespace ResticGUI

#endif
