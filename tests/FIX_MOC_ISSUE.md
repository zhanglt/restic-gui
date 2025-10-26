# 修复链接错误：缺少 moc 文件

## 问题诊断

链接错误显示缺少以下测试类的 Qt 元对象（moc）文件：
- ❌ `moc_ResticWrapperTest.cpp`
- ❌ `moc_BackupManagerTest.cpp`
- ❌ `moc_DatabaseManagerTest.cpp`

## 根本原因

这些测试文件最近被修改（修复了注释和语法错误），但是：
1. qmake 没有重新运行，导致 Makefile 没有更新
2. moc 文件没有重新生成

## 解决方法

### 方法 1：使用提供的批处理脚本（推荐）

在 **tests** 目录下，直接运行：

```cmd
rebuild.bat
```

这个脚本会自动完成以下步骤：
1. 清理旧的 moc 文件
2. 重新运行 qmake
3. 清理构建
4. 重新编译

### 方法 2：手动重建（如果脚本失败）

#### Windows (使用 Qt Creator 集成的命令行环境)

1. **打开 Qt 命令提示符**
   - 开始菜单 → Qt → "Qt 5.14.x (MSVC 2019) 64-bit"
   - 或者使用 Qt Creator 的终端

2. **进入测试目录**
   ```cmd
   cd E:\test\qt\restic-gui\tests
   ```

3. **清理旧文件**
   ```cmd
   del /F /Q build\moc\moc_ResticWrapperTest.cpp 2>nul
   del /F /Q build\moc\moc_BackupManagerTest.cpp 2>nul
   del /F /Q build\moc\moc_DatabaseManagerTest.cpp 2>nul
   ```

4. **重新运行 qmake**
   ```cmd
   qmake tests.pro
   ```

5. **清理构建**
   ```cmd
   nmake clean
   ```

6. **重新编译**
   ```cmd
   nmake
   ```

#### Linux/macOS

1. **进入测试目录**
   ```bash
   cd tests
   ```

2. **清理旧文件**
   ```bash
   rm -f build/moc/moc_ResticWrapperTest.cpp
   rm -f build/moc/moc_BackupManagerTest.cpp
   rm -f build/moc/moc_DatabaseManagerTest.cpp
   ```

3. **重新运行 qmake**
   ```bash
   qmake tests.pro
   ```

4. **清理构建**
   ```bash
   make clean
   ```

5. **重新编译**
   ```bash
   make
   ```

### 方法 3：使用 Qt Creator（最简单）

1. 在 Qt Creator 中打开 `tests/tests.pro`
2. 右键点击项目 → **Run qmake**
3. 右键点击项目 → **Clean All**
4. 右键点击项目 → **Rebuild All**

## 验证修复

编译成功后，检查以下 moc 文件是否存在：

```cmd
dir build\moc\moc_ResticWrapperTest.cpp
dir build\moc\moc_BackupManagerTest.cpp
dir build\moc\moc_DatabaseManagerTest.cpp
```

所有三个文件都应该存在。

## 运行测试

编译成功后，运行测试：

```cmd
..\bin\restic-gui-tests.exe
```

或在 Linux/macOS 上：

```bash
../bin/restic-gui-tests
```

## 如果问题仍然存在

### 检查 tests.pro 文件

确保以下头文件在 HEADERS 列表中（已验证存在）：

```qmake
HEADERS += \
    unit/core/ResticWrapperTest.h \      # Line 192 ✓
    unit/core/BackupManagerTest.h \      # Line 194 ✓
    unit/data/DatabaseManagerTest.h      # Line 207 ✓
```

### 检查头文件格式

确保这些 .h 文件：
1. 包含 `Q_OBJECT` 宏（已验证）
2. 使用 UTF-8 编码
3. 没有 BOM 标记

### 完全清理重建

如果上述方法都失败，尝试完全清理：

```cmd
# Windows
rd /S /Q build
qmake tests.pro
nmake

# Linux/macOS
rm -rf build
qmake tests.pro
make
```

## 已修复的问题总结

在重建之前，我们已经修复了以下问题：

1. ✅ ResticWrapperTest.cpp - 修复了错误的信号名称
2. ✅ DatabaseManagerTest.cpp - 删除了乱码注释，修复了 BackupTask 成员访问
3. ✅ BackupManagerTest.cpp - 修复了注释和语法错误
4. ✅ BackupFlowTest.cpp - 修复了 BackupTask 成员访问
5. ✅ ModelsTest.cpp - 修复了 Schedule 枚举值

现在只需要重新生成 moc 文件即可完成编译！
