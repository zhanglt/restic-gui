#-------------------------------------------------
# Restic GUI - qmake Project File
# 基于 Qt 5.14 的 restic 图形界面程序
#-------------------------------------------------

QT       += core gui widgets sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = restic-gui
TEMPLATE = app

# C++ 标准
CONFIG += c++17

# 编译选项
DEFINES += QT_DEPRECATED_WARNINGS

# MSVC 编码设置
msvc {
    QMAKE_CXXFLAGS += /utf-8
}

# 输出目录
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# ===== 源文件 =====

# 主程序
SOURCES += \
    src/main.cpp

# 数据模型
SOURCES += \
    src/models/Repository.cpp \
    src/models/BackupTask.cpp \
    src/models/Schedule.cpp \
    src/models/Snapshot.cpp \
    src/models/FileInfo.cpp \
    src/models/BackupResult.cpp \
    src/models/RestoreOptions.cpp \
    src/models/RepoStats.cpp

# 数据访问层
SOURCES += \
    src/data/DatabaseManager.cpp \
    src/data/ConfigManager.cpp \
    src/data/PasswordManager.cpp \
    src/data/CacheManager.cpp

# 工具类
SOURCES += \
    src/utils/Logger.cpp \
    src/utils/CryptoUtil.cpp \
    src/utils/FileSystemUtil.cpp \
    src/utils/NetworkUtil.cpp

# 核心业务逻辑
SOURCES += \
    src/core/ResticWrapper.cpp \
    src/core/RepositoryManager.cpp \
    src/core/BackupManager.cpp \
    src/core/RestoreManager.cpp \
    src/core/SnapshotManager.cpp \
    src/core/SchedulerManager.cpp

# UI - 主窗口
SOURCES += \
    src/ui/MainWindow.cpp

# UI - 页面
SOURCES += \
    src/ui/pages/HomePage.cpp \
    src/ui/pages/RepositoryPage.cpp \
    src/ui/pages/BackupPage.cpp \
    src/ui/pages/SnapshotPage.cpp \
    src/ui/pages/RestorePage.cpp \
    src/ui/pages/StatsPage.cpp

# UI - 向导
SOURCES += \
    src/ui/wizards/CreateRepoWizard.cpp \
    src/ui/wizards/RestoreWizard.cpp

# UI - 对话框
SOURCES += \
    src/ui/dialogs/ProgressDialog.cpp \
    src/ui/dialogs/SettingsDialog.cpp \
    src/ui/dialogs/CreateTaskDialog.cpp \
    src/ui/dialogs/SnapshotBrowserDialog.cpp \
    src/ui/dialogs/PruneOptionsDialog.cpp \
    src/ui/dialogs/PasswordDialog.cpp

# UI - 自定义控件
SOURCES += \
    src/ui/widgets/SnapshotListWidget.cpp \
    src/ui/widgets/FileTreeWidget.cpp

# ===== 头文件 =====

HEADERS += \
    src/models/Repository.h \
    src/models/BackupTask.h \
    src/models/Schedule.h \
    src/models/Snapshot.h \
    src/models/FileInfo.h \
    src/models/BackupResult.h \
    src/models/RestoreOptions.h \
    src/models/RepoStats.h \
    src/data/DatabaseManager.h \
    src/data/ConfigManager.h \
    src/data/PasswordManager.h \
    src/data/CacheManager.h \
    src/utils/Logger.h \
    src/utils/CryptoUtil.h \
    src/utils/FileSystemUtil.h \
    src/utils/NetworkUtil.h \
    src/core/ResticWrapper.h \
    src/core/RepositoryManager.h \
    src/core/BackupManager.h \
    src/core/RestoreManager.h \
    src/core/SnapshotManager.h \
    src/core/SchedulerManager.h \
    src/ui/MainWindow.h \
    src/ui/pages/HomePage.h \
    src/ui/pages/RepositoryPage.h \
    src/ui/pages/BackupPage.h \
    src/ui/pages/SnapshotPage.h \
    src/ui/pages/RestorePage.h \
    src/ui/pages/StatsPage.h \
    src/ui/wizards/CreateRepoWizard.h \
    src/ui/wizards/RestoreWizard.h \
    src/ui/dialogs/ProgressDialog.h \
    src/ui/dialogs/SettingsDialog.h \
    src/ui/dialogs/CreateTaskDialog.h \
    src/ui/dialogs/SnapshotBrowserDialog.h \
    src/ui/dialogs/PruneOptionsDialog.h \
    src/ui/dialogs/PasswordDialog.h \
    src/ui/widgets/SnapshotListWidget.h \
    src/ui/widgets/FileTreeWidget.h

# ===== UI 文件 =====

FORMS += \
    src/ui/MainWindow.ui \
    src/ui/pages/HomePage.ui \
    src/ui/pages/RepositoryPage.ui \
    src/ui/pages/BackupPage.ui \
    src/ui/pages/SnapshotPage.ui \
    src/ui/pages/RestorePage.ui \
    src/ui/pages/StatsPage.ui \
    src/ui/dialogs/ProgressDialog.ui \
    src/ui/dialogs/SettingsDialog.ui \
    src/ui/dialogs/PruneOptionsDialog.ui

# ===== 资源文件 =====

RESOURCES += \
    resources/restic-gui.qrc

# ===== 翻译文件 =====

TRANSLATIONS += \
    resources/translations/restic_gui_zh_CN.ts

# ===== 包含路径 =====

INCLUDEPATH += \
    src \
    src/core \
    src/ui \
    src/models \
    src/data \
    src/utils

# ===== 部署配置 =====

# 默认部署规则
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Windows 特定配置
win32 {
    RC_ICONS = resources/icons/app.ico
    VERSION = 1.0.0.0
    QMAKE_TARGET_COMPANY = "Restic GUI Project"
    QMAKE_TARGET_PRODUCT = "Restic GUI"
    QMAKE_TARGET_DESCRIPTION = "Restic Backup GUI"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2025"
}

# macOS 特定配置
mac {
    ICON = resources/icons/app.icns
    QMAKE_INFO_PLIST = Info.plist
}

# ===== 其他配置 =====

# 启用QtConcurrent
QT += concurrent

# 禁用某些警告（仅非MSVC编译器）
!msvc {
    QMAKE_CXXFLAGS += -Wno-unused-parameter
}

# Release配置优化
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
    !msvc {
        QMAKE_CXXFLAGS += -O2
    }
}

# Debug配置
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE
}
