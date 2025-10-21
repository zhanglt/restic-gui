/**
 * @file Logger.cpp
 * @brief 日志管理类实现
 */

#include "Logger.h"
#include <QDateTime>
#include <QDebug>

namespace ResticGUI {
namespace Utils {

Logger* Logger::s_instance = nullptr;

Logger* Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_level(Info)
{
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

void Logger::setLogFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }

    m_logFile.setFileName(filePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
    }
}

void Logger::setLevel(Level level)
{
    m_level = level;
}

void Logger::debug(const QString& message)
{
    log(Debug, message);
}

void Logger::info(const QString& message)
{
    log(Info, message);
}

void Logger::warning(const QString& message)
{
    log(Warning, message);
}

void Logger::error(const QString& message)
{
    log(Error, message);
}

void Logger::critical(const QString& message)
{
    log(Critical, message);
}

void Logger::log(Level level, const QString& message)
{
    if (level < m_level) {
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString levelStr = levelToString(level);
    QString formattedMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);

    // 输出到控制台
    qDebug().noquote() << formattedMessage;

    // 写入文件
    writeToFile(formattedMessage);

    // 发送信号
    emit logMessage(static_cast<int>(level), message);
}

QString Logger::levelToString(Level level)
{
    switch (level) {
    case Debug:    return "DEBUG";
    case Info:     return "INFO ";
    case Warning:  return "WARN ";
    case Error:    return "ERROR";
    case Critical: return "CRIT ";
    default:       return "UNKN ";
    }
}

void Logger::writeToFile(const QString& message)
{
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen()) {
        m_logStream << message << "\n";
        m_logStream.flush();
    }
}

} // namespace Utils
} // namespace ResticGUI
