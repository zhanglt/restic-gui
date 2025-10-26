#!/usr/bin/env python3
"""
测试骨架生成脚本
自动生成所有测试类的骨架代码（.h 和 .cpp 文件）
"""

import os

# 测试类配置
TEST_CLASSES = {
    # 数据模型测试（1个测试类，测试所有8个模型）
    "unit/models": [
        {
            "name": "ModelsTest",
            "includes": ["models/Repository.h", "models/BackupTask.h", "models/Schedule.h",
                        "models/Snapshot.h", "models/FileInfo.h", "models/BackupResult.h",
                        "models/RestoreOptions.h", "models/RepoStats.h"],
            "tests": [
                "testRepositorySerialize", "testRepositoryDeserialize",
                "testBackupTaskSerialize", "testBackupTaskDeserialize",
                "testScheduleSerialize", "testScheduleDeserialize",
                "testSnapshotSerialize", "testSnapshotDeserialize",
                "testFileInfoSerialize", "testFileInfoDeserialize",
                "testBackupResultSerialize", "testBackupResultDeserialize",
                "testRestoreOptionsSerialize", "testRestoreOptionsDeserialize",
                "testRepoStatsSerialize", "testRepoStatsDeserialize"
            ]
        }
    ],

    # 核心管理器测试（6个）
    "unit/core": [
        {
            "name": "ResticWrapperTest",
            "includes": ["core/ResticWrapper.h"],
            "tests": ["testResticPathValidation", "testCommandLineBuilder", "testJsonParsing",
                     "testProgressSignals", "testErrorHandling", "testPasswordSecurity",
                     "testAsyncExecution", "testProcessTermination"]
        },
        {
            "name": "RepositoryManagerTest",
            "includes": ["core/RepositoryManager.h"],
            "tests": ["testCreateRepository", "testDeleteRepository", "testListRepositories",
                     "testConnectRepository", "testPasswordCache", "testUpdateRepository",
                     "testInvalidRepository"]
        },
        {
            "name": "BackupManagerTest",
            "includes": ["core/BackupManager.h"],
            "tests": ["testCreateBackupTask", "testDeleteBackupTask", "testExecuteBackup",
                     "testBackupHistory", "testExcludeRules", "testTagManagement",
                     "testConcurrentBackups", "testBackupFailure"]
        },
        {
            "name": "RestoreManagerTest",
            "includes": ["core/RestoreManager.h"],
            "tests": ["testBuildFileTree", "testSelectiveRestore", "testRestorePathValidation",
                     "testRestoreProgress", "testRestoreRollback", "testFileSearch"]
        },
        {
            "name": "SnapshotManagerTest",
            "includes": ["core/SnapshotManager.h"],
            "tests": ["testListSnapshots", "testSnapshotCache", "testForgetSnapshot",
                     "testPruneSnapshots", "testTagFilter", "testSnapshotStats"]
        },
        {
            "name": "SchedulerManagerTest",
            "includes": ["core/SchedulerManager.h"],
            "tests": ["testScheduleTask", "testConditionChecks", "testStartStopScheduler",
                     "testTaskConflicts", "testScheduleHistory"]
        }
    ],

    # 数据访问层测试（4个）
    "unit/data": [
        {
            "name": "DatabaseManagerTest",
            "includes": ["data/DatabaseManager.h"],
            "tests": ["testDatabaseInit", "testCRUD_Repository", "testCRUD_BackupTask",
                     "testCRUD_BackupHistory", "testTransaction", "testConcurrentAccess",
                     "testMigration", "testDataIntegrity"]
        },
        {
            "name": "ConfigManagerTest",
            "includes": ["data/ConfigManager.h"],
            "tests": ["testReadWriteConfig", "testDefaultValues", "testConfigValidation",
                     "testConfigReload"]
        },
        {
            "name": "PasswordManagerTest",
            "includes": ["data/PasswordManager.h"],
            "tests": ["testEncryptPassword", "testDecryptPassword", "testSessionCache",
                     "testPasswordStorage"]
        },
        {
            "name": "CacheManagerTest",
            "includes": ["data/CacheManager.h"],
            "tests": ["testCacheStore", "testCacheRetrieve", "testCacheExpiration",
                     "testCacheClear"]
        }
    ],

    # UI 测试（3个）
    "ui": [
        {
            "name": "MainWindowTest",
            "includes": ["ui/MainWindow.h"],
            "tests": ["testWindowInit", "testPageSwitching", "testMenuActions",
                     "testSystemTray", "testWindowState"]
        },
        {
            "name": "PagesTest",
            "includes": ["ui/pages/HomePage.h", "ui/pages/RepositoryPage.h",
                        "ui/pages/BackupPage.h", "ui/pages/SnapshotPage.h",
                        "ui/pages/RestorePage.h", "ui/pages/StatsPage.h"],
            "tests": ["testHomePageInit", "testRepositoryPageActions",
                     "testBackupPageTaskList", "testSnapshotPageList",
                     "testRestorePageFileTree", "testStatsPageCharts"]
        },
        {
            "name": "DialogsTest",
            "includes": ["ui/dialogs/ProgressDialog.h", "ui/dialogs/PasswordDialog.h",
                        "ui/dialogs/SettingsDialog.h"],
            "tests": ["testProgressDialogUpdate", "testPasswordDialogInput",
                     "testSettingsDialogSave"]
        }
    ],

    # 集成测试（4个）
    "integration": [
        {
            "name": "BackupFlowTest",
            "includes": ["core/RepositoryManager.h", "core/BackupManager.h"],
            "tests": ["testCompleteBackupFlow", "testBackupWithExcludes"]
        },
        {
            "name": "RestoreFlowTest",
            "includes": ["core/SnapshotManager.h", "core/RestoreManager.h"],
            "tests": ["testCompleteRestoreFlow", "testSelectiveRestore"]
        },
        {
            "name": "RepositoryManagementTest",
            "includes": ["core/RepositoryManager.h"],
            "tests": ["testRepositoryLifecycle", "testRepositoryCheck"]
        },
        {
            "name": "SchedulerTest",
            "includes": ["core/SchedulerManager.h", "core/BackupManager.h"],
            "tests": ["testScheduledBackup", "testSchedulerConditions"]
        }
    ]
}


def generate_header(class_info, category):
    """生成测试头文件"""
    name = class_info["name"]
    tests = class_info["tests"]
    includes = class_info["includes"]

    header_content = f"""#ifndef {name.upper()}_H
#define {name.upper()}_H

#include "common/TestBase.h"
"""

    # 添加包含文件
    for include in includes:
        header_content += f'#include "{include}"\n'

    header_content += f"""
namespace ResticGUI {{
namespace Test {{

/**
 * @brief {name} 测试类
 *
 * TODO: 实现具体的测试逻辑
 */
class {name} : public TestBase
{{
    Q_OBJECT

private slots:
"""

    # 添加测试函数声明
    for test in tests:
        header_content += f"    void {test}();\n"

    header_content += f"""
}};

}} // namespace Test
}} // namespace ResticGUI

#endif // {name.upper()}_H
"""

    return header_content


def generate_cpp(class_info, category):
    """生成测试实现文件"""
    name = class_info["name"]
    tests = class_info["tests"]

    cpp_content = f"""#include "{name}.h"

namespace ResticGUI {{
namespace Test {{

"""

    # 添加测试函数实现
    for test in tests:
        cpp_content += f"""void {name}::{test}()
{{
    // TODO: 实现测试逻辑
    QVERIFY(true);  // 占位符
}}

"""

    cpp_content += """} // namespace Test
} // namespace ResticGUI
"""

    return cpp_content


def main():
    """主函数"""
    script_dir = os.path.dirname(os.path.abspath(__file__))

    generated_count = 0

    for category, classes in TEST_CLASSES.items():
        category_dir = os.path.join(script_dir, category)
        os.makedirs(category_dir, exist_ok=True)

        for class_info in classes:
            name = class_info["name"]

            # 生成头文件
            header_path = os.path.join(category_dir, f"{name}.h")
            if not os.path.exists(header_path):
                with open(header_path, 'w', encoding='utf-8') as f:
                    f.write(generate_header(class_info, category))
                print(f"[+] Generated: {header_path}")
                generated_count += 1
            else:
                print(f"[-] Skipped (exists): {header_path}")

            # 生成实现文件
            cpp_path = os.path.join(category_dir, f"{name}.cpp")
            if not os.path.exists(cpp_path):
                with open(cpp_path, 'w', encoding='utf-8') as f:
                    f.write(generate_cpp(class_info, category))
                print(f"[+] Generated: {cpp_path}")
                generated_count += 1
            else:
                print(f"[-] Skipped (exists): {cpp_path}")

    print(f"\nTotal files generated: {generated_count}")
    print("\nNote: All test functions are currently placeholder implementations (QVERIFY(true))")
    print("Please implement specific test logic for each test function as needed.")


if __name__ == "__main__":
    main()
