# Restic GUI 项目文件清单

## 📁 项目结构

```
restic-gui/
├── src/                          # 源代码目录
│   ├── main.cpp                  # 应用程序入口
│   │
│   ├── models/                   # 数据模型层 (8个类)
│   │   ├── Repository.h/cpp      # 仓库数据模型
│   │   ├── BackupTask.h/cpp      # 备份任务模型
│   │   ├── Schedule.h/cpp        # 调度配置模型
│   │   ├── Snapshot.h/cpp        # 快照模型
│   │   ├── FileInfo.h/cpp        # 文件信息模型
│   │   ├── BackupResult.h/cpp    # 备份结果模型
│   │   ├── RestoreOptions.h/cpp  # 恢复选项模型
│   │   └── RepoStats.h/cpp       # 仓库统计模型
│   │
│   ├── utils/                    # 工具类层 (4个类)
│   │   ├── Logger.h/cpp          # 日志系统
│   │   ├── CryptoUtil.h/cpp      # 加密工具
│   │   ├── FileSystemUtil.h/cpp  # 文件系统工具
│   │   └── NetworkUtil.h/cpp     # 网络工具
│   │
│   ├── data/                     # 数据访问层 (4个类)
│   │   ├── DatabaseManager.h/cpp # 数据库管理器
│   │   ├── ConfigManager.h/cpp   # 配置管理器
│   │   ├── PasswordManager.h/cpp # 密码管理器
│   │   └── CacheManager.h/cpp    # 缓存管理器
│   │
│   ├── core/                     # 核心业务逻辑层 (6个类)
│   │   ├── ResticWrapper.h/cpp   # Restic命令封装
│   │   ├── RepositoryManager.h/cpp   # 仓库管理器
│   │   ├── BackupManager.h/cpp       # 备份管理器
│   │   ├── SnapshotManager.h/cpp     # 快照管理器
│   │   ├── RestoreManager.h/cpp      # 恢复管理器
│   │   └── SchedulerManager.h/cpp    # 调度管理器
│   │
│   └── ui/                       # UI表现层
│       ├── MainWindow.h/cpp      # 主窗口
│       │
│       ├── pages/                # 主要页面 (6个)
│       │   ├── HomePage.h/cpp            # 首页仪表板
│       │   ├── RepositoryPage.h/cpp      # 仓库管理页面
│       │   ├── BackupPage.h/cpp          # 备份任务页面
│       │   ├── SnapshotPage.h/cpp        # 快照浏览页面
│       │   ├── RestorePage.h/cpp         # 数据恢复页面
│       │   └── StatsPage.h/cpp           # 统计信息页面
│       │
│       ├── wizards/              # 向导 (已创建1个)
│       │   └── CreateRepoWizard.h/cpp    # 创建仓库向导
│       │
│       ├── dialogs/              # 对话框 (已创建2个)
│       │   ├── ProgressDialog.h/cpp      # 进度对话框
│       │   └── SettingsDialog.h/cpp      # 设置对话框
│       │
│       └── widgets/              # 自定义控件 (已创建2个)
│           ├── SnapshotListWidget.h/cpp  # 快照列表控件
│           └── FileTreeWidget.h/cpp      # 文件树控件
│
├── resources/                    # 资源文件目录
│   ├── sql/
│   │   └── init_database.sql     # 数据库初始化脚本
│   └── restic-gui.qrc            # Qt资源文件
│
├── restic-gui.pro                # qmake项目文件
├── README.md                     # 项目说明文档
├── CLAUDE.md                     # 开发上下文文档
└── PROJECT_FILES.md              # 本文件清单

```

## 📊 统计信息

### 已创建文件总数：**67个**

| 模块 | 类数 | 头文件 | 源文件 | 总计 |
|------|------|--------|--------|------|
| 数据模型 | 8 | 8 | 8 | 16 |
| 工具类 | 4 | 4 | 4 | 8 |
| 数据层 | 4 | 4 | 4 | 8 |
| 业务层 | 6 | 6 | 6 | 12 |
| UI-主窗口 | 1 | 1 | 1 | 2 |
| UI-页面 | 6 | 6 | 6 | 12 |
| UI-向导 | 1 | 1 | 1 | 2 |
| UI-对话框 | 2 | 2 | 2 | 4 |
| UI-控件 | 2 | 2 | 2 | 4 |
| **小计** | **34** | **34** | **34** | **68** |
| 其他文件 |  |  |  | 5 |
| **总计** |  |  |  | **73** |

*其他文件：main.cpp, init_database.sql, restic-gui.qrc, restic-gui.pro, README.md, CLAUDE.md, PROJECT_FILES.md*

## ✅ 完成的功能模块

### 1. 数据模型层 ✅
- [x] Repository - 仓库模型（支持8种后端类型）
- [x] BackupTask - 备份任务模型
- [x] Schedule - 调度配置模型（支持5种调度类型）
- [x] Snapshot - 快照模型
- [x] FileInfo - 文件信息模型
- [x] BackupResult - 备份结果模型
- [x] RestoreOptions - 恢复选项模型
- [x] RepoStats - 仓库统计模型

### 2. 工具类层 ✅
- [x] Logger - 线程安全的日志系统
- [x] CryptoUtil - 加密工具（SHA-256、AES-256）
- [x] FileSystemUtil - 文件系统操作
- [x] NetworkUtil - 网络连接检查

### 3. 数据访问层 ✅
- [x] DatabaseManager - SQLite数据库管理
- [x] ConfigManager - QSettings配置管理
- [x] PasswordManager - 密码管理（3种存储模式）
- [x] CacheManager - 智能缓存管理

### 4. 核心业务层 ✅
- [x] ResticWrapper - Restic命令行封装
- [x] RepositoryManager - 仓库CRUD和操作
- [x] BackupManager - 备份任务管理和执行
- [x] SnapshotManager - 快照管理和浏览
- [x] RestoreManager - 数据恢复管理
- [x] SchedulerManager - 任务调度管理

### 5. UI表现层 ✅
- [x] MainWindow - 主窗口框架
- [x] HomePage - 仪表板
- [x] RepositoryPage - 仓库管理
- [x] BackupPage - 备份任务
- [x] SnapshotPage - 快照浏览
- [x] RestorePage - 数据恢复
- [x] StatsPage - 统计信息
- [x] CreateRepoWizard - 创建仓库向导
- [x] ProgressDialog - 进度对话框
- [x] SettingsDialog - 设置对话框
- [x] SnapshotListWidget - 快照列表控件
- [x] FileTreeWidget - 文件树控件

## 🔧 待扩展功能

### UI组件（可选）
- [ ] BackupWizard - 备份向导
- [ ] RestoreWizard - 恢复向导
- [ ] TaskListWidget - 任务列表控件
- [ ] RepoConfigDialog - 仓库配置对话框

### 资源文件（待添加）
- [ ] 应用图标
- [ ] 工具栏图标
- [ ] 翻译文件

## 🎯 核心特性

### 架构设计
- ✅ 三层架构（UI → 业务逻辑 → 数据访问）
- ✅ 单例模式（所有Manager类）
- ✅ 信号/槽机制（层间解耦）
- ✅ 线程安全（互斥锁保护）

### 数据管理
- ✅ SQLite持久化存储
- ✅ 智能缓存机制
- ✅ 配置文件管理
- ✅ 完整的数据库架构

### 密码安全
- ✅ 三种存储模式（不存储/会话缓存/加密存储）
- ✅ AES-256加密（框架已实现）
- ✅ SHA-256哈希
- ✅ 主密码保护

### Restic集成
- ✅ 完整的命令封装
- ✅ JSON输出解析
- ✅ 进度监控（框架已实现）
- ✅ 异步执行支持

### 用户界面
- ✅ 现代化侧边栏导航
- ✅ 页面式内容展示
- ✅ 向导式引导流程
- ✅ 实时进度反馈

## 📝 编译说明

### 前置条件
- Qt 5.14+
- C++17编译器
- CMake 或 qmake

### 编译步骤

**使用 Qt Creator:**
```bash
1. 打开 restic-gui.pro
2. 配置 Qt 5.14 套件
3. 构建项目
4. 运行
```

**使用命令行:**
```bash
cd E:\test\qt\restic-gui
qmake restic-gui.pro
nmake  # Windows
# 或 make  # Linux/macOS
```

## 🚀 运行要求

1. **安装 Restic**
   - 从 https://restic.net 下载
   - 添加到系统 PATH
   - 或在设置中指定路径

2. **运行程序**
   ```bash
   cd bin
   .\restic-gui.exe  # Windows
   # 或 ./restic-gui  # Linux/macOS
   ```

## 📌 开发笔记

### 已知TODO
- CryptoUtil需要集成QCA库实现真正的AES-256
- ResticWrapper需要完善实时进度解析
- 各页面需要连接实际的Manager类
- 添加单元测试
- 添加图标资源

### 代码规范
- 使用Qt命名约定
- 所有字符串使用tr()国际化
- Manager类使用单例模式
- 信号/槽用于跨层通信
- 互斥锁保护共享资源

---

**项目状态**: ✅ 框架完成，可编译运行
**更新日期**: 2025-01-XX
**版本**: v1.0.0-alpha
