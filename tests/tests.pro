#-------------------------------------------------
# Restic GUI - Tests Project File
# 使用 Qt Test 框架的测试项目
#-------------------------------------------------

QT       += core gui widgets sql network testlib concurrent
QT       -= gui  # 某些测试不需要 GUI

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = restic-gui-tests
TEMPLATE = app

# C++ 标准
CONFIG += c++17
CONFIG += testcase
CONFIG += qt warn_on depend_includepath

# 测试配置
CONFIG += console
CONFIG -= app_bundle

# 编译选项
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += RESTIC_GUI_TESTS  # 测试模式标记

# MSVC 编码设置
msvc {
    QMAKE_CXXFLAGS += /utf-8
}

# 输出目录
DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# ===== 主项目源文件（用于测试） =====

# 注意：我们引用主项目的源代码，但不包括 main.cpp（测试有自己的 main）

MAIN_SRC_DIR = $$PWD/../src

# 数据模型
SOURCES += \
    $$MAIN_SRC_DIR/models/Repository.cpp \
    $$MAIN_SRC_DIR/models/BackupTask.cpp \
    $$MAIN_SRC_DIR/models/Schedule.cpp \
    $$MAIN_SRC_DIR/models/Snapshot.cpp \
    $$MAIN_SRC_DIR/models/FileInfo.cpp \
    $$MAIN_SRC_DIR/models/BackupResult.cpp \
    $$MAIN_SRC_DIR/models/RestoreOptions.cpp \
    $$MAIN_SRC_DIR/models/RepoStats.cpp

# 数据访问层
SOURCES += \
    $$MAIN_SRC_DIR/data/DatabaseManager.cpp \
    $$MAIN_SRC_DIR/data/ConfigManager.cpp \
    $$MAIN_SRC_DIR/data/PasswordManager.cpp \
    $$MAIN_SRC_DIR/data/CacheManager.cpp

# 工具类
SOURCES += \
    $$MAIN_SRC_DIR/utils/Logger.cpp \
    $$MAIN_SRC_DIR/utils/CryptoUtil.cpp \
    $$MAIN_SRC_DIR/utils/FileSystemUtil.cpp \
    $$MAIN_SRC_DIR/utils/NetworkUtil.cpp

# 核心业务逻辑
SOURCES += \
    $$MAIN_SRC_DIR/core/ResticWrapper.cpp \
    $$MAIN_SRC_DIR/core/RepositoryManager.cpp \
    $$MAIN_SRC_DIR/core/BackupManager.cpp \
    $$MAIN_SRC_DIR/core/RestoreManager.cpp \
    $$MAIN_SRC_DIR/core/SnapshotManager.cpp \
    $$MAIN_SRC_DIR/core/SchedulerManager.cpp

# UI - 主窗口
SOURCES += \
    $$MAIN_SRC_DIR/ui/MainWindow.cpp

# UI - 页面
SOURCES += \
    $$MAIN_SRC_DIR/ui/pages/HomePage.cpp \
    $$MAIN_SRC_DIR/ui/pages/RepositoryPage.cpp \
    $$MAIN_SRC_DIR/ui/pages/BackupPage.cpp \
    $$MAIN_SRC_DIR/ui/pages/SnapshotPage.cpp \
    $$MAIN_SRC_DIR/ui/pages/RestorePage.cpp \
    $$MAIN_SRC_DIR/ui/pages/StatsPage.cpp

# UI - 向导
SOURCES += \
    $$MAIN_SRC_DIR/ui/wizards/CreateRepoWizard.cpp \
    $$MAIN_SRC_DIR/ui/wizards/RestoreWizard.cpp

# UI - 对话框
SOURCES += \
    $$MAIN_SRC_DIR/ui/dialogs/ProgressDialog.cpp \
    $$MAIN_SRC_DIR/ui/dialogs/SettingsDialog.cpp \
    $$MAIN_SRC_DIR/ui/dialogs/CreateTaskDialog.cpp \
    $$MAIN_SRC_DIR/ui/dialogs/SnapshotBrowserDialog.cpp \
    $$MAIN_SRC_DIR/ui/dialogs/PruneOptionsDialog.cpp \
    $$MAIN_SRC_DIR/ui/dialogs/PasswordDialog.cpp

# UI - 自定义控件
SOURCES += \
    $$MAIN_SRC_DIR/ui/widgets/SnapshotListWidget.cpp \
    $$MAIN_SRC_DIR/ui/widgets/FileTreeWidget.cpp

# ===== 主项目头文件 =====

HEADERS += \
    $$MAIN_SRC_DIR/models/Repository.h \
    $$MAIN_SRC_DIR/models/BackupTask.h \
    $$MAIN_SRC_DIR/models/Schedule.h \
    $$MAIN_SRC_DIR/models/Snapshot.h \
    $$MAIN_SRC_DIR/models/FileInfo.h \
    $$MAIN_SRC_DIR/models/BackupResult.h \
    $$MAIN_SRC_DIR/models/RestoreOptions.h \
    $$MAIN_SRC_DIR/models/RepoStats.h \
    $$MAIN_SRC_DIR/data/DatabaseManager.h \
    $$MAIN_SRC_DIR/data/ConfigManager.h \
    $$MAIN_SRC_DIR/data/PasswordManager.h \
    $$MAIN_SRC_DIR/data/CacheManager.h \
    $$MAIN_SRC_DIR/utils/Logger.h \
    $$MAIN_SRC_DIR/utils/CryptoUtil.h \
    $$MAIN_SRC_DIR/utils/FileSystemUtil.h \
    $$MAIN_SRC_DIR/utils/NetworkUtil.h \
    $$MAIN_SRC_DIR/core/ResticWrapper.h \
    $$MAIN_SRC_DIR/core/RepositoryManager.h \
    $$MAIN_SRC_DIR/core/BackupManager.h \
    $$MAIN_SRC_DIR/core/RestoreManager.h \
    $$MAIN_SRC_DIR/core/SnapshotManager.h \
    $$MAIN_SRC_DIR/core/SchedulerManager.h \
    $$MAIN_SRC_DIR/ui/MainWindow.h \
    $$MAIN_SRC_DIR/ui/pages/HomePage.h \
    $$MAIN_SRC_DIR/ui/pages/RepositoryPage.h \
    $$MAIN_SRC_DIR/ui/pages/BackupPage.h \
    $$MAIN_SRC_DIR/ui/pages/SnapshotPage.h \
    $$MAIN_SRC_DIR/ui/pages/RestorePage.h \
    $$MAIN_SRC_DIR/ui/pages/StatsPage.h \
    $$MAIN_SRC_DIR/ui/wizards/CreateRepoWizard.h \
    $$MAIN_SRC_DIR/ui/wizards/RestoreWizard.h \
    $$MAIN_SRC_DIR/ui/dialogs/ProgressDialog.h \
    $$MAIN_SRC_DIR/ui/dialogs/SettingsDialog.h \
    $$MAIN_SRC_DIR/ui/dialogs/CreateTaskDialog.h \
    $$MAIN_SRC_DIR/ui/dialogs/SnapshotBrowserDialog.h \
    $$MAIN_SRC_DIR/ui/dialogs/PruneOptionsDialog.h \
    $$MAIN_SRC_DIR/ui/dialogs/PasswordDialog.h \
    $$MAIN_SRC_DIR/ui/widgets/SnapshotListWidget.h \
    $$MAIN_SRC_DIR/ui/widgets/FileTreeWidget.h

# ===== 主项目 UI 文件 =====

FORMS += \
    $$MAIN_SRC_DIR/ui/MainWindow.ui \
    $$MAIN_SRC_DIR/ui/pages/HomePage.ui \
    $$MAIN_SRC_DIR/ui/pages/RepositoryPage.ui \
    $$MAIN_SRC_DIR/ui/pages/BackupPage.ui \
    $$MAIN_SRC_DIR/ui/pages/SnapshotPage.ui \
    $$MAIN_SRC_DIR/ui/pages/RestorePage.ui \
    $$MAIN_SRC_DIR/ui/pages/StatsPage.ui \
    $$MAIN_SRC_DIR/ui/dialogs/ProgressDialog.ui \
    $$MAIN_SRC_DIR/ui/dialogs/SettingsDialog.ui \
    $$MAIN_SRC_DIR/ui/dialogs/PruneOptionsDialog.ui

# ===== 主项目资源文件 =====

RESOURCES += \
    $$PWD/../resources/restic-gui.qrc

# ===== 测试源文件 =====

# 通用测试工具
SOURCES += \
    common/TestBase.cpp

HEADERS += \
    common/TestBase.h

# 单元测试 - 核心管理器
SOURCES += \
    unit/core/ResticWrapperTest.cpp \
    unit/core/RepositoryManagerTest.cpp \
    unit/core/BackupManagerTest.cpp \
    unit/core/RestoreManagerTest.cpp \
    unit/core/SnapshotManagerTest.cpp \
    unit/core/SchedulerManagerTest.cpp

HEADERS += \
    unit/core/ResticWrapperTest.h \
    unit/core/RepositoryManagerTest.h \
    unit/core/BackupManagerTest.h \
    unit/core/RestoreManagerTest.h \
    unit/core/SnapshotManagerTest.h \
    unit/core/SchedulerManagerTest.h

# 单元测试 - 数据访问层
SOURCES += \
    unit/data/DatabaseManagerTest.cpp \
    unit/data/ConfigManagerTest.cpp \
    unit/data/PasswordManagerTest.cpp \
    unit/data/CacheManagerTest.cpp

HEADERS += \
    unit/data/DatabaseManagerTest.h \
    unit/data/ConfigManagerTest.h \
    unit/data/PasswordManagerTest.h \
    unit/data/CacheManagerTest.h

# 单元测试 - 工具类
SOURCES += \
    unit/utils/LoggerTest.cpp \
    unit/utils/CryptoUtilTest.cpp \
    unit/utils/FileSystemUtilTest.cpp \
    unit/utils/NetworkUtilTest.cpp

HEADERS += \
    unit/utils/LoggerTest.h \
    unit/utils/CryptoUtilTest.h \
    unit/utils/FileSystemUtilTest.h \
    unit/utils/NetworkUtilTest.h

# 单元测试 - 数据模型
SOURCES += \
    unit/models/ModelsTest.cpp

HEADERS += \
    unit/models/ModelsTest.h

# 单元测试 - UI 组件
SOURCES += \
    ui/MainWindowTest.cpp \
    ui/PagesTest.cpp \
    ui/DialogsTest.cpp

HEADERS += \
    ui/MainWindowTest.h \
    ui/PagesTest.h \
    ui/DialogsTest.h

# 集成测试
SOURCES += \
    integration/BackupFlowTest.cpp \
    integration/RestoreFlowTest.cpp \
    integration/RepositoryManagementTest.cpp \
    integration/SchedulerTest.cpp

HEADERS += \
    integration/BackupFlowTest.h \
    integration/RestoreFlowTest.h \
    integration/RepositoryManagementTest.h \
    integration/SchedulerTest.h

# 测试主函数
SOURCES += \
    main.cpp

# ===== 包含路径 =====

INCLUDEPATH += \
    $$PWD \
    $$PWD/common \
    $$MAIN_SRC_DIR \
    $$MAIN_SRC_DIR/core \
    $$MAIN_SRC_DIR/ui \
    $$MAIN_SRC_DIR/models \
    $$MAIN_SRC_DIR/data \
    $$MAIN_SRC_DIR/utils

# ===== 测试配置 =====

# Debug 配置
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE
}

# 测试输出详细信息
QMAKE_CXXFLAGS += -g

# 禁用某些警告
!msvc {
    QMAKE_CXXFLAGS += -Wno-unused-parameter
}

# ===== 说明 =====
#
# 编译测试项目：
#   qmake tests.pro
#   make (或 nmake)
#
# 运行所有测试：
#   ../bin/restic-gui-tests
#
# 运行特定测试：
#   ../bin/restic-gui-tests -functions  # 列出所有测试函数
#   ../bin/restic-gui-tests testName    # 运行特定测试
#
