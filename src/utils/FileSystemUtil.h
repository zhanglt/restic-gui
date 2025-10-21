#ifndef FILESYSTEMUTIL_H
#define FILESYSTEMUTIL_H

#include <QString>

namespace ResticGUI {
namespace Utils {

class FileSystemUtil
{
public:
    static bool ensureDirectoryExists(const QString& path);
    static QString normalizePath(const QString& path);
    static qint64 getDirectorySize(const QString& path);
    static bool isWritable(const QString& path);

private:
    FileSystemUtil() = delete;
};

} // namespace Utils
} // namespace ResticGUI

#endif
