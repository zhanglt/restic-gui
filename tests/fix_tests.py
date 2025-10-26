#!/usr/bin/env python3
"""
Test Files Fixer
Batch fix common issues in test files to match actual model definitions
"""

import os
import re

# Define replacement rules
REPLACEMENTS = [
    # BackupTask fixes
    (r'\.sources\b', '.sourcePaths'),
    (r'\.excludes\b', '.excludePatterns'),
    (r'"sources"', '"sourcePaths"'),
    (r'"excludes"', '"excludePatterns"'),

    # Snapshot fixes
    (r'\.shortId\b', '.id'),  # Use id instead of shortId

    # FileInfo fixes
    (r'\.isDir\b', '.type == FileType::Directory'),
    (r'\.modTime\b', '.mtime'),
    (r'FileType::RegularFile', 'FileType::File'),

    # RestoreOptions fixes
    (r'\.verifyData\b', '.verify'),
    (r'\.includePatterns\b', '.includePaths'),
    (r'\.excludePatterns\b', '.excludePaths'),
    (r'"verifyData"', '"verify"'),
    (r'"includePatterns"', '"includePaths"'),
    (r'"excludePatterns"', '"excludePaths"'),

    # Schedule fixes
    (r'Schedule::ScheduleType::', 'Schedule::'),
    (r'ScheduleType::', 'Schedule::'),
    (r'schedule\.enabled', 'schedule.type != Schedule::None'),
    (r'\.enabled\s*=\s*true', '.type = Schedule::Daily'),

    # RepoStats fixes
    (r'\.uniqueDataSize\b', '.uniqueSize'),
    (r'\.deduplicationRatio\b', '.compressionRatio'),

    # JSON value fixes
    (r'\.toLongLong\(\)', '.toVariant().toLongLong()'),
]

def fix_file(filepath):
    """Fix a single test file"""
    print(f"Fixing {filepath}...")

    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        original = content

        # Apply all replacements
        for pattern, replacement in REPLACEMENTS:
            content = re.sub(pattern, replacement, content)

        # Only write if changed
        if content != original:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  [+] Fixed {filepath}")
            return True
        else:
            print(f"  [-] No changes needed for {filepath}")
            return False

    except Exception as e:
        print(f"  [!] Error fixing {filepath}: {e}")
        return False

def main():
    """Fix all test files in the tests directory"""
    tests_dir = os.path.dirname(os.path.abspath(__file__))

    fixed_count = 0
    total_count = 0

    # Find all .cpp test files
    for root, dirs, files in os.walk(tests_dir):
        for file in files:
            if file.endswith('.cpp') and 'Test' in file:
                filepath = os.path.join(root, file)
                total_count += 1
                if fix_file(filepath):
                    fixed_count += 1

    print(f"\nSummary: Fixed {fixed_count}/{total_count} files")

if __name__ == '__main__':
    main()
