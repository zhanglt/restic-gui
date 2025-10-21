#include "FileSystemUtil.h"
#include <QDir>
#include <QFileInfo>

namespace ResticGUI {
namespace Utils {

bool FileSystemUtil::ensureDirectoryExists(const QString& path)
{
    return QDir().mkpath(path);
}

QString FileSystemUtil::normalizePath(const QString& path)
{
    return QDir::cleanPath(path);
}

qint64 FileSystemUtil::getDirectorySize(const QString& path)
{
    qint64 size = 0;
    QDir dir(path);
    for (const QFileInfo& info : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (info.isFile()) {
            size += info.size();
        } else if (info.isDir()) {
            size += getDirectorySize(info.absoluteFilePath());
        }
    }
    return size;
}

bool FileSystemUtil::isWritable(const QString& path)
{
    QFileInfo info(path);
    return info.isWritable();
}

} // namespace Utils
} // namespace ResticGUI
