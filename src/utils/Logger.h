/**
 * @file Logger.h
 * @brief 日志管理类
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>

namespace ResticGUI {
namespace Utils {

class Logger : public QObject
{
    Q_OBJECT

public:
    enum Level {
        Debug = 0,
        Info,
        Warning,
        Error,
        Critical
    };

    static Logger* instance();

    void setLogFile(const QString& filePath);
    void setLevel(Level level);

    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void critical(const QString& message);

    void log(Level level, const QString& message);

signals:
    void logMessage(int level, const QString& message);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    static Logger* s_instance;

    QFile m_logFile;
    QTextStream m_logStream;
    Level m_level;
    QMutex m_mutex;

    QString levelToString(Level level);
    void writeToFile(const QString& message);
};

} // namespace Utils
} // namespace ResticGUI

#endif // LOGGER_H
