# Restic GUI - 测试开发进度报告

## 📊 当前状态（2025-10-26）

### ✅ 已完成

#### 1. 测试基础设施（100% 完成）

**目录结构：**
```
tests/
├── common/               # 通用测试工具
│   ├── TestBase.h       # ✅ 测试基类（完整实现）
│   └── TestBase.cpp     # ✅ 测试基类实现
├── unit/                # 单元测试
│   ├── core/           # 核心管理器测试
│   ├── data/           # 数据访问层测试
│   ├── utils/          # 工具类测试（已完成4个）
│   └── models/         # 数据模型测试
├── integration/        # 集成测试
├── ui/                 # UI测试
├── tests.pro          # ✅ Qt Test 项目配置
├── main.cpp           # ✅ 测试主函数
├── run_tests.bat      # ✅ Windows 测试运行脚本
├── run_tests.sh       # ✅ Linux/macOS 测试运行脚本
└── generate_test_stubs.py  # ✅ 测试骨架生成脚本
```

**已创建的测试基础设施：**
- [x] 测试目录结构（7个子目录）
- [x] TestBase 基类（提供临时目录、数据库、断言等工具）
- [x] tests.pro 配置文件（完整的 qmake 配置）
- [x] main.cpp 测试主函数（运行所有测试）
- [x] 测试运行脚本（Windows + Unix）
- [x] Python 测试骨架生成脚本

#### 2. 单元测试 - 工具类（100% 完成）

| 测试类 | 头文件 | 实现文件 | 测试用例数 | 状态 |
|--------|--------|----------|-----------|------|
| CryptoUtilTest | ✅ | ✅ | 15 | **完整实现** |
| FileSystemUtilTest | ✅ | ✅ | 12 | **完整实现** |
| LoggerTest | ✅ | ✅ | 4 | **完整实现** |
| NetworkUtilTest | ✅ | ✅ | 3 | **完整实现** |

**总计：4个测试类，34个测试用例（已完成）**

---

### 🚧 进行中 / 待完成

#### 3. 单元测试 - 数据模型（0% 完成）

**需要创建：** 1个测试类，16个测试用例

| 测试类 | 测试用例 | 优先级 | 状态 |
|--------|---------|--------|------|
| ModelsTest | testRepositorySerialize | 高 | ❌ 待创建 |
|  | testRepositoryDeserialize | 高 | ❌ 待创建 |
|  | testBackupTaskSerialize | 高 | ❌ 待创建 |
|  | testBackupTaskDeserialize | 高 | ❌ 待创建 |
|  | testScheduleSerialize | 中 | ❌ 待创建 |
|  | testScheduleDeserialize | 中 | ❌ 待创建 |
|  | testSnapshotSerialize | 高 | ❌ 待创建 |
|  | testSnapshotDeserialize | 高 | ❌ 待创建 |
|  | testFileInfoSerialize | 中 | ❌ 待创建 |
|  | testFileInfoDeserialize | 中 | ❌ 待创建 |
|  | testBackupResultSerialize | 中 | ❌ 待创建 |
|  | testBackupResultDeserialize | 中 | ❌ 待创建 |
|  | testRestoreOptionsSerialize | 中 | ❌ 待创建 |
|  | testRestoreOptionsDeserialize | 中 | ❌ 待创建 |
|  | testRepoStatsSerialize | 低 | ❌ 待创建 |
|  | testRepoStatsDeserialize | 低 | ❌ 待创建 |

#### 4. 单元测试 - 核心管理器（0% 完成）

**需要创建：** 6个测试类，约45个测试用例

| 测试类 | 测试用例数 | 优先级 | 状态 |
|--------|-----------|--------|------|
| ResticWrapperTest | 8 | **极高** | ❌ 待创建 |
| RepositoryManagerTest | 7 | 高 | ❌ 待创建 |
| BackupManagerTest | 8 | **极高** | ❌ 待创建 |
| RestoreManagerTest | 6 | 高 | ❌ 待创建 |
| SnapshotManagerTest | 6 | 高 | ❌ 待创建 |
| SchedulerManagerTest | 5 | 中 | ❌ 待创建 |

#### 5. 单元测试 - 数据访问层（0% 完成）

**需要创建：** 4个测试类，约20个测试用例

| 测试类 | 测试用例数 | 优先级 | 状态 |
|--------|-----------|--------|------|
| DatabaseManagerTest | 8 | **极高** | ❌ 待创建 |
| ConfigManagerTest | 4 | 中 | ❌ 待创建 |
| PasswordManagerTest | 4 | 高 | ❌ 待创建 |
| CacheManagerTest | 4 | 中 | ❌ 待创建 |

#### 6. UI 测试（0% 完成）

**需要创建：** 3个测试类，约14个测试用例

| 测试类 | 测试用例数 | 优先级 | 状态 |
|--------|-----------|--------|------|
| MainWindowTest | 5 | 中 | ❌ 待创建 |
| PagesTest | 6 | 低 | ❌ 待创建 |
| DialogsTest | 3 | 低 | ❌ 待创建 |

#### 7. 集成测试（0% 完成）

**需要创建：** 4个测试类，约8个测试用例

| 测试类 | 测试用例数 | 优先级 | 状态 |
|--------|-----------|--------|------|
| BackupFlowTest | 2 | 高 | ❌ 待创建 |
| RestoreFlowTest | 2 | 高 | ❌ 待创建 |
| RepositoryManagementTest | 2 | 中 | ❌ 待创建 |
| SchedulerTest | 2 | 中 | ❌ 待创建 |

---

## 📈 总体统计

| 类别 | 测试类 | 测试用例 | 已完成 | 待完成 | 完成率 |
|------|--------|----------|--------|--------|--------|
| **测试基础设施** | 1 | - | 1 | 0 | 100% |
| **工具类测试** | 4 | 34 | 4 (34用例) | 0 | 100% |
| **数据模型测试** | 1 | 16 | 0 | 1 (16用例) | 0% |
| **核心管理器测试** | 6 | 45 | 0 | 6 (45用例) | 0% |
| **数据访问层测试** | 4 | 20 | 0 | 4 (20用例) | 0% |
| **UI 测试** | 3 | 14 | 0 | 3 (14用例) | 0% |
| **集成测试** | 4 | 8 | 0 | 4 (8用例) | 0% |
| **总计** | **23** | **137** | **4** | **18 (103用例)** | **25%** |

---

## 🎯 后续开发建议

### 优先级 1：核心功能测试（必须完成）

这些测试对于保证应用核心功能的正确性至关重要：

1. **ResticWrapperTest**（极高优先级）
   - 这是与 restic CLI 交互的核心类
   - 测试 JSON 解析、命令执行、错误处理
   - **预计工作量：2-3小时**

2. **BackupManagerTest**（极高优先级）
   - 测试备份任务的创建和执行
   - 测试备份历史记录
   - **预计工作量：2-3小时**

3. **DatabaseManagerTest**（极高优先级）
   - 测试数据库 CRUD 操作
   - 测试事务处理和并发访问
   - **预计工作量：2-3小时**

4. **ModelsTest**（高优先级）
   - 测试所有数据模型的序列化/反序列化
   - 相对简单但很重要
   - **预计工作量：1-2小时**

### 优先级 2：重要功能测试（建议完成）

5. **RepositoryManagerTest**
6. **RestoreManagerTest**
7. **SnapshotManagerTest**
8. **PasswordManagerTest**

### 优先级 3：辅助功能测试（可选）

9. **SchedulerManagerTest**
10. **ConfigManagerTest**
11. **CacheManagerTest**
12. **集成测试**（4个）
13. **UI 测试**（3个）

---

## 🛠️ 使用测试骨架生成脚本

已创建 `generate_test_stubs.py` 脚本来快速生成测试骨架：

### 脚本功能：
- 自动生成所有测试类的 .h 和 .cpp 骨架文件
- 包含所有测试函数声明和占位符实现
- 跳过已存在的文件，不会覆盖已完成的测试

### 使用方法：

**方法 1：使用 Python 运行**
```bash
cd tests
python generate_test_stubs.py
```

**方法 2：手动创建测试文件**

如果 Python 不可用，可以参考已完成的测试类（CryptoUtilTest, FileSystemUtilTest 等）的结构，手动创建新的测试类。

### 测试类模板结构：

**头文件（例如 ModelsTest.h）：**
```cpp
#ifndef MODELSTEST_H
#define MODELSTEST_H

#include "common/TestBase.h"
#include "models/Repository.h"
// ... 其他 includes

namespace ResticGUI {
namespace Test {

class ModelsTest : public TestBase
{
    Q_OBJECT

private slots:
    void testRepositorySerialize();
    void testRepositoryDeserialize();
    // ... 其他测试函数
};

} // namespace Test
} // namespace ResticGUI

#endif
```

**实现文件（例如 ModelsTest.cpp）：**
```cpp
#include "ModelsTest.h"

namespace ResticGUI {
namespace Test {

void ModelsTest::testRepositorySerialize()
{
    // 创建测试对象
    Models::Repository repo;
    repo.id = 1;
    repo.name = "Test Repo";
    repo.path = "/tmp/test";

    // 序列化
    QVariantMap map = repo.toVariantMap();

    // 验证
    QCOMPARE(map["id"].toInt(), 1);
    QCOMPARE(map["name"].toString(), QString("Test Repo"));
    QCOMPARE(map["path"].toString(), QString("/tmp/test"));
}

void ModelsTest::testRepositoryDeserialize()
{
    // 创建测试数据
    QVariantMap map;
    map["id"] = 1;
    map["name"] = "Test Repo";
    map["path"] = "/tmp/test";

    // 反序列化
    Models::Repository repo = Models::Repository::fromVariantMap(map);

    // 验证
    QCOMPARE(repo.id, 1);
    QCOMPARE(repo.name, QString("Test Repo"));
    QCOMPARE(repo.path, QString("/tmp/test"));
}

} // namespace Test
} // namespace ResticGUI
```

---

## 🧪 如何编译和运行测试

### 使用测试运行脚本（推荐）：

**Windows：**
```bash
cd tests
run_tests.bat
```

**Linux/macOS：**
```bash
cd tests
chmod +x run_tests.sh
./run_tests.sh
```

### 手动编译：

```bash
cd tests
qmake tests.pro
nmake         # Windows (MSVC)
# 或
mingw32-make  # Windows (MinGW)
# 或
make          # Linux/macOS

# 运行测试
../bin/restic-gui-tests
```

### 运行特定测试：

```bash
# 列出所有测试函数
../bin/restic-gui-tests -functions

# 运行特定测试
../bin/restic-gui-tests CryptoUtilTest
../bin/restic-gui-tests CryptoUtilTest::testEncryptDecrypt
```

---

## 📚 参考资料

### Qt Test 文档：
- [Qt Test 官方文档](https://doc.qt.io/qt-5/qtest-overview.html)
- [QTest 宏参考](https://doc.qt.io/qt-5/qtest.html)

### 测试最佳实践：
1. **每个测试应该独立**：不依赖其他测试的执行顺序
2. **使用 TestBase 提供的工具**：临时目录、测试数据库等
3. **测试命名清晰**：`test<ClassName><Method><Scenario>`
4. **使用描述性的失败消息**：`QVERIFY2(condition, "描述信息")`
5. **清理测试数据**：在 `cleanup()` 或析构函数中清理

### 断言宏：
- `QVERIFY(condition)` - 验证条件为真
- `QVERIFY2(condition, message)` - 带消息的验证
- `QCOMPARE(actual, expected)` - 比较两个值
- `QTEST(actual, expected)` - 用于数据驱动测试
- `QFAIL(message)` - 强制失败
- `QSKIP(message)` - 跳过测试

---

## 🚀 下一步行动计划

### 第 1 步：完成核心测试（预计 8-10 小时）
1. ✅ 工具类测试（已完成）
2. ❌ 数据模型测试（1-2小时）
3. ❌ ResticWrapperTest（2-3小时）
4. ❌ BackupManagerTest（2-3小时）
5. ❌ DatabaseManagerTest（2-3小时）

### 第 2 步：完成重要测试（预计 6-8 小时）
6. ❌ RepositoryManagerTest
7. ❌ RestoreManagerTest
8. ❌ SnapshotManagerTest
9. ❌ PasswordManagerTest

### 第 3 步：完成其余测试（预计 4-6 小时）
10. ❌ 其他单元测试
11. ❌ 集成测试
12. ❌ UI 测试

---

## ✍️ 贡献指南

如果您想继续完善测试，请遵循以下步骤：

1. **选择一个未完成的测试类**（参考上面的优先级列表）
2. **创建测试文件**（可使用 generate_test_stubs.py 或手动创建）
3. **实现测试函数**（参考已完成的 CryptoUtilTest 等）
4. **编译并运行测试**（使用 run_tests.bat/sh）
5. **修复失败的测试**
6. **更新本文档**（标记已完成的测试）

---

**最后更新：** 2025-10-26
**当前测试覆盖率：** 约 25%（基础设施 + 工具类）
**目标测试覆盖率：** >75%（核心功能）
