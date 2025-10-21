#ifndef FILEINFO_H
#define FILEINFO_H

#include <QString>
#include <QDateTime>

namespace ResticGUI {
namespace Models {

enum class FileType
{
    File,
    Directory,
    Symlink,
    Other
};

struct FileInfo
{
    QString path;
    QString name;
    FileType type = FileType::File;
    qint64 size = 0;
    QDateTime mtime;
    QString permissions;
    QString mode;
    int uid = 0;
    int gid = 0;
    QString user;
    QString group;
};

} // namespace Models
} // namespace ResticGUI

#endif
