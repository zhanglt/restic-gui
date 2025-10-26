#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
修复 BackupManagerTest.cpp 中剩余的语法错误
请关闭所有可能打开该文件的程序（IDE、编译器等），然后运行此脚本
"""

file_path = r'E:\test\qt\restic-gui\tests\unit\core\BackupManagerTest.cpp'

print("正在修复 BackupManagerTest.cpp...")

try:
    # Read
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # Replace the problematic section with standalone )
    old_section = '''void BackupManagerTest::testRunBackupTask()
{
    // *K
    // Note: This test requires actual restic repository
    // Skipping actual execution here
)

    Repository repo = createTestRepository();'''

    new_section = '''void BackupManagerTest::testRunBackupTask()
{
    // Note: This test requires actual restic repository
    // Skipping actual execution here

    Repository repo = createTestRepository();'''

    content = content.replace(old_section, new_section)

    # Write back
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)

    print("✓ 修复成功！")

except PermissionError:
    print("✗ 权限错误：文件被占用")
    print("  请关闭所有正在使用该文件的程序（Qt Creator、编译器等）后重试")
except Exception as e:
    print(f"✗ 错误: {e}")
