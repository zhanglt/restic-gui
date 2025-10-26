# Restic GUI - 测试系统

本目录包含 Restic GUI 项目的完整测试套件，基于 Qt Test 框架。

## 📁 目录结构

```
tests/
├── common/                 # 测试基础工具
│   ├── TestBase.h         # 测试基类（提供临时目录、数据库、断言工具）
│   └── TestBase.cpp
├── unit/                  # 单元测试
│   ├── core/             # 核心管理器测试（6个）
│   ├── data/             # 数据访问层测试（4个）
│   ├── utils/            # 工具类测试（4个 - 已完成）
│   └── models/           # 数据模型测试（1个）
├── integration/          # 集成测试（4个）
├── ui/                   # UI 测试（3个）
├── tests.pro            # qmake 项目配置
├── main.cpp             # 测试主函数
├── run_tests.bat        # Windows 快速运行脚本
├── run_tests.sh         # Linux/macOS 快速运行脚本
├── generate_test_stubs.py  # Python 测试骨架生成脚本
├── TEST_PROGRESS.md     # 详细的测试开发进度报告
└── README.md            # 本文件
```

## 🚀 快速开始

### 方法 1：使用测试运行脚本（推荐）

**Windows：**
```cmd
cd tests
run_tests.bat
```

**Linux/macOS：**
```bash
cd tests
chmod +x run_tests.sh
./run_tests.sh
```

脚本会自动完成以下操作：
1. 清理旧的构建文件
2. 运行 qmake 生成 Makefile
3. 编译测试项目
4. 运行所有测试
5. 显示测试结果

### 方法 2：手动编译和运行

```bash
# 1. 进入测试目录
cd tests

# 2. 生成 Makefile
qmake tests.pro

# 3. 编译
nmake          # Windows (MSVC)
mingw32-make   # Windows (MinGW)
make           # Linux/macOS

# 4. 运行测试
../bin/restic-gui-tests          # 运行所有测试
../bin/restic-gui-tests -functions  # 列出所有测试函数
```

## 📊 当前测试状态

**测试覆盖率：** 约 25%（基础设施 + 工具类测试）

**已完成：**
- ✅ 测试基础设施（TestBase, tests.pro, main.cpp, 运行脚本）
- ✅ 工具类测试（4个测试类，34个测试用例）
  - CryptoUtilTest（15个用例）
  - FileSystemUtilTest（12个用例）
  - LoggerTest（4个用例）
  - NetworkUtilTest（3个用例）

**待完成：**
- ❌ 数据模型测试（1个测试类，16个用例）
- ❌ 核心管理器测试（6个测试类，约45个用例）
- ❌ 数据访问层测试（4个测试类，约20个用例）
- ❌ UI 测试（3个测试类，约14个用例）
- ❌ 集成测试（4个测试类，约8个用例）

**详细进度和后续计划** 请查看 [TEST_PROGRESS.md](TEST_PROGRESS.md)

## 🧪 运行特定测试

```bash
# 运行特定测试类
../bin/restic-gui-tests CryptoUtilTest

# 运行特定测试函数
../bin/restic-gui-tests CryptoUtilTest::testEncryptDecrypt

# 列出所有测试函数
../bin/restic-gui-tests -functions

# 详细输出模式
../bin/restic-gui-tests -v2

# 生成 XML 测试报告（用于 CI）
../bin/restic-gui-tests -xunitxml -o test-results.xml
```

## 🛠️ 开发新测试

### 1. 使用 Python 脚本生成骨架（推荐）

```bash
cd tests
python generate_test_stubs.py
```

该脚本会自动生成所有未创建的测试类的骨架文件（.h 和 .cpp），包含测试函数声明和占位符实现。

### 2. 手动创建测试类

参考已完成的测试类（如 `CryptoUtilTest`），创建新的测试类：

**头文件模板（MyTest.h）：**
```cpp
#ifndef MYTEST_H
#define MYTEST_H

#include "common/TestBase.h"
#include "path/to/ClassToTest.h"

namespace ResticGUI {
namespace Test {

class MyTest : public TestBase
{
    Q_OBJECT

private slots:
    void testSomething();
    void testAnotherThing();
};

} // namespace Test
} // namespace ResticGUI

#endif
```

**实现文件模板（MyTest.cpp）：**
```cpp
#include "MyTest.h"

namespace ResticGUI {
namespace Test {

void MyTest::testSomething()
{
    // 准备测试数据
    // ...

    // 执行被测试的功能
    // ...

    // 验证结果
    QVERIFY(condition);
    QCOMPARE(actual, expected);
}

void MyTest::testAnotherThing()
{
    // 测试实现
}

} // namespace Test
} // namespace ResticGUI
```

### 3. 添加到 tests.pro

在 `tests.pro` 的 `SOURCES` 和 `HEADERS` 中添加新的测试文件：

```qmake
SOURCES += \
    # ... 其他文件
    unit/path/MyTest.cpp

HEADERS += \
    # ... 其他文件
    unit/path/MyTest.h
```

### 4. 更新 main.cpp

在 `main.cpp` 中添加新测试类的运行代码：

```cpp
#include "unit/path/MyTest.h"

// 在 main() 函数中添加：
totalTests++;
failedTests += runTest(new ResticGUI::Test::MyTest(), "MyTest");
```

## 💡 测试最佳实践

### 1. 使用 TestBase 提供的工具

TestBase 基类提供了许多实用方法：

```cpp
// 临时目录
QString dir = tempDir();  // 获取临时测试目录
QString subDir = createTempSubDir("mysubdir");  // 创建子目录
QString file = createTempFile("test.txt", "content");  // 创建测试文件

// 测试数据库
QSqlDatabase db = createTestDatabase("test_connection");  // 创建内存数据库
closeTestDatabase("test_connection");  // 关闭数据库

// 验证工具
verifyFileExists("/path/to/file");  // 验证文件存在
verifyDirExists("/path/to/dir");  // 验证目录存在
waitForSignal(object, SIGNAL(signalName()), 5000);  // 等待信号（超时5秒）
compareVariantMaps(expected, actual);  // 比较 QVariantMap
```

### 2. 常用断言宏

```cpp
QVERIFY(condition);  // 验证条件为真
QVERIFY2(condition, "Error message");  // 带错误消息的验证
QCOMPARE(actual, expected);  // 比较两个值（失败时显示详细信息）
QVERIFY(!list.isEmpty());  // 验证列表非空
QCOMPARE(list.size(), 5);  // 验证列表大小
```

### 3. 测试命名规范

- 测试类名：`<ClassUnderTest>Test`（例如：`CryptoUtilTest`）
- 测试函数名：`test<Method><Scenario>`（例如：`testEncryptDecrypt`, `testEncryptWithWrongKey`）

### 4. 测试结构

```cpp
void MyTest::testSomething()
{
    // 1. Arrange（准备）- 设置测试数据和环境
    QString input = "test data";
    MyClass obj;

    // 2. Act（执行）- 执行被测试的操作
    QString result = obj.doSomething(input);

    // 3. Assert（断言）- 验证结果
    QCOMPARE(result, QString("expected output"));
}
```

### 5. 清理测试数据

```cpp
void MyTest::cleanup()
{
    // 每个测试函数执行后自动调用
    // 清理临时数据、关闭连接等
}

void MyTest::cleanupTestCase()
{
    // 所有测试函数执行完后调用一次
    // 清理整个测试类的资源
}
```

## 📚 参考资源

- **Qt Test 官方文档：** https://doc.qt.io/qt-5/qtest-overview.html
- **QTest 宏参考：** https://doc.qt.io/qt-5/qtest.html
- **TEST_PROGRESS.md：** 详细的测试开发进度和后续计划
- **已完成的测试示例：**
  - `unit/utils/CryptoUtilTest.{h,cpp}`
  - `unit/utils/FileSystemUtilTest.{h,cpp}`
  - `unit/utils/LoggerTest.{h,cpp}`
  - `unit/utils/NetworkUtilTest.{h,cpp}`

## 🐛 故障排除

### 编译错误

**问题：** 找不到 qmake
**解决：** 确保 Qt 安装路径在系统 PATH 中，或使用 Qt Creator 的命令行工具

**问题：** 链接错误（undefined reference）
**解决：** 检查 tests.pro 中是否包含了所有需要的源文件和头文件

### 运行时错误

**问题：** 测试可执行文件找不到
**解决：** 检查 `../bin/` 目录，或查看 `../bin/debug/` 和 `../bin/release/` 子目录

**问题：** 数据库错误
**解决：** 确保使用 `createTestDatabase()` 创建测试专用的内存数据库，不要使用主应用的数据库

### 测试失败

**问题：** 网络测试不稳定
**解决：** 网络相关测试可能因环境不同而失败，可以适当放宽断言或跳过（使用 `QSKIP`）

**问题：** 文件权限测试失败
**解决：** 文件权限在不同操作系统上行为不同，Windows 和 Unix 系统需要分别处理

## 📞 获取帮助

- 查看 **TEST_PROGRESS.md** 了解详细的测试开发指南
- 参考已完成的测试类代码
- 查阅 Qt Test 官方文档
- 在项目 issue 中提问

---

**最后更新：** 2025-10-26
**当前版本：** v1.1（测试框架初始版本）
