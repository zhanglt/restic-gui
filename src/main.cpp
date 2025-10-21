/**
 * @file main.cpp
 * @brief Restic GUI 应用程序入口
 * @author Restic GUI Project
 * @date 2025-10-20
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QStandardPaths>
#include "ui/MainWindow.h"
#include "utils/Logger.h"
#include "data/DatabaseManager.h"
#include "data/ConfigManager.h"

using namespace ResticGUI;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    QCoreApplication::setOrganizationName("ResticGUI");
    QCoreApplication::setOrganizationDomain("restic-gui.org");
    QCoreApplication::setApplicationName("Restic GUI");
    QCoreApplication::setApplicationVersion("1.0.0");

    // 初始化日志系统
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logPath);
    Utils::Logger::instance()->setLogFile(logPath + "/restic-gui.log");
    Utils::Logger::instance()->setLevel(Utils::Logger::Debug);  // 开启调试日志

    Utils::Logger::instance()->info("========================================");
    Utils::Logger::instance()->info("Restic GUI v1.0.0 启动");
    Utils::Logger::instance()->info("========================================");

    // 加载翻译文件
    QTranslator translator;
    QString translationPath = ":/translations";
    if (translator.load(QLocale(), "restic_gui", "_", translationPath)) {
        app.installTranslator(&translator);
        Utils::Logger::instance()->info("已加载中文翻译");
    }

    // 初始化数据库
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    Utils::Logger::instance()->debug(QString("数据目录: %1").arg(dbPath));
    QDir().mkpath(dbPath);

    QString dbFile = dbPath + "/restic-gui.db";
    Utils::Logger::instance()->info(QString("初始化数据库: %1").arg(dbFile));

    if (!Data::DatabaseManager::instance()->initialize(dbFile)) {
        QString error = Data::DatabaseManager::instance()->lastError();
        Utils::Logger::instance()->critical(QString("数据库初始化失败: %1").arg(error));
        return -1;
    }
    Utils::Logger::instance()->info("数据库初始化成功");

    // 加载配置
    Utils::Logger::instance()->info(QString("restic 路径: %1")
        .arg(Data::ConfigManager::instance()->getResticPath()));

    // 创建并显示主窗口
    UI::MainWindow mainWindow;
    mainWindow.show();

    Utils::Logger::instance()->info("主窗口已显示");

    // 进入事件循环
    int ret = app.exec();

    Utils::Logger::instance()->info(QString("应用程序退出，返回码: %1").arg(ret));
    Utils::Logger::instance()->info("========================================\n");

    return ret;
}
