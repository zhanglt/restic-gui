# Restic GUI - 简单易用的备份工具

## 项目简介

Restic GUI 是一个基于 Qt 5.14 开发的跨平台图形化界面程序，为强大的开源备份工具 [restic](https://restic.net/) 提供友好易用的操作界面。

**核心特性：**
- ✅ 向导式操作流程，降低使用门槛
- ✅ 支持多种后端（本地、SFTP、S3、REST等）
- ✅ 完整的备份、恢复、快照管理功能
- ✅ 定时备份任务，自动化保护数据
- ✅ 直观的数据统计和趋势分析
- ✅ 跨平台支持（Windows、Linux、macOS）
- ✅ 完全免费开源

## 项目文档

本项目包含完整的设计文档，为开发提供全方位指导：

### 📋 需求与设计文档

| 文档 | 描述 | 路径 |
|------|------|------|
| **需求规格说明书** | 详细的功能性和非功能性需求 | [docs/需求规格说明书.md](docs/需求规格说明书.md) |
| **概要设计文档** | 系统架构、模块划分、技术选型 | [docs/概要设计文档.md](docs/概要设计文档.md) |
| **详细设计文档** | 所有类的详细接口和实现设计 | [docs/详细设计文档.md](docs/详细设计文档.md) |
| **数据库设计文档** | SQLite数据库schema设计 | [docs/数据库设计文档.md](docs/数据库设计文档.md) |
| **界面原型设计文档** | 所有界面的原型和交互设计 | [docs/界面原型设计文档.md](docs/界面原型设计文档.md) |

### 📚 Restic 官方文档

项目中包含 restic 官方文档（`restic_doc/` 目录），涵盖：
- 安装指南
- 备份、恢复操作
- 仓库管理
- 各种后端配置
- 常见问题解答

## 项目状态

**当前版本：** v1.1
**开发状态：** ✅ 核心功能已全部实现，项目可编译运行
**代码规模：** 75个文件，约11,300行代码
**最后更新：** 2025-10-24

## 技术栈

- **开发框架：** Qt 5.14
- **编程语言：** C++ 17
- **数据库：** SQLite 3
- **构建工具：** qmake（项目使用qmake，不使用CMake）
- **UI设计：** Qt Designer .ui 文件（10个）+ 纯代码（向导和自定义控件）
- **依赖工具：** restic (需单独安装)

## 系统架构

```
Restic GUI 采用三层架构：

┌─────────────────────────────────────────┐
│         表现层（UI Layer）              │
│  - MainWindow                           │
│  - Wizards (向导)                       │
│  - Dialogs (对话框)                     │
└─────────────────────────────────────────┘
                   ↕
┌─────────────────────────────────────────┐
│      业务逻辑层（Business Layer）       │
│  - ResticWrapper (restic封装)           │
│  - RepositoryManager (仓库管理)         │
│  - BackupManager (备份管理)             │
│  - RestoreManager (恢复管理)            │
│  - SnapshotManager (快照管理)           │
│  - SchedulerManager (任务调度)          │
└─────────────────────────────────────────┘
                   ↕
┌─────────────────────────────────────────┐
│      数据访问层（Data Layer）           │
│  - DatabaseManager (数据库)             │
│  - ConfigManager (配置)                 │
│  - PasswordManager (密码管理)           │
└─────────────────────────────────────────┘
```

## 核心功能模块

### 1. 仓库管理
- 创建本地或远程仓库
- 连接已有仓库
- 管理多个仓库
- 仓库完整性检查

### 2. 备份管理
- 创建备份任务（支持多路径、排除规则）
- 手动执行备份
- 定时自动备份
- 备份历史记录

### 3. 快照管理
- 浏览所有快照
- 查看快照详情
- 删除旧快照
- 清理仓库（prune）
- 管理快照标签

### 4. 数据恢复
- 浏览快照文件树
- 搜索文件
- 选择性恢复
- 恢复向导

### 5. 统计与监控
- 仓库统计信息
- 备份趋势分析
- 存储空间分析
- 去重效率统计

## 快速开始

### 前置要求

1. **安装 Qt 5.14**
   - Windows: 下载 Qt 安装程序
   - Linux: `sudo apt install qt5-default qttools5-dev`
   - macOS: `brew install qt@5`

2. **安装 restic**
   - 从 [restic官网](https://restic.net/) 下载对应平台的可执行文件
   - 或使用包管理器：
     - Linux: `sudo apt install restic`
     - macOS: `brew install restic`

### 构建项目

**使用 qmake（推荐）：**

```bash
# Windows (cmd)
qmake restic-gui.pro
nmake          # MSVC编译器
# 或 mingw32-make  # MinGW编译器

# Linux/macOS
qmake restic-gui.pro
make
```

**或使用 Qt Creator（Windows 推荐）：**
1. 打开 `restic-gui.pro` 文件
2. 配置项目（选择 MSVC 或 MinGW kit）
3. 点击构建（Ctrl+B）
4. 运行（Ctrl+R）

### 运行程序

构建成功后，可执行文件位于：
- Windows: `bin/restic-gui.exe`
- Linux/macOS: `bin/restic-gui`

```bash
# Windows
bin\restic-gui.exe

# Linux/macOS
./bin/restic-gui
```

## 已实现功能

### ✅ 第一阶段：需求分析与设计（已完成）
- [x] 需求规格说明书
- [x] 概要设计文档
- [x] 详细设计文档
- [x] 数据库设计文档
- [x] 界面原型设计

### ✅ 第二阶段：项目框架搭建（已完成）
- [x] 创建 Qt 项目结构
- [x] 配置构建系统（qmake）
- [x] 创建基础类框架
- [x] 设置资源文件和翻译
- [x] UI 转换为 Qt Designer .ui 文件

### ✅ 第三阶段：核心功能开发（已完成）
- [x] 仓库管理模块（RepositoryManager）
- [x] 备份功能模块（BackupManager）
- [x] 恢复功能模块（RestoreManager）
- [x] 快照管理模块（SnapshotManager）
- [x] 定时备份模块（SchedulerManager）
- [x] ResticWrapper（restic CLI 完整封装）
- [x] 数据访问层（DatabaseManager、ConfigManager、PasswordManager、CacheManager）
- [x] 工具类（Logger、CryptoUtil、FileSystemUtil、NetworkUtil）
- [x] 数据模型（8个结构体）
- [x] 主窗口和6个页面（全部使用.ui文件）
- [x] 对话框（ProgressDialog、SettingsDialog、PruneOptionsDialog等）
- [x] 向导（CreateRepoWizard，5步向导）
- [x] 自定义控件（FileTreeWidget、SnapshotListWidget）

### 📅 第四阶段：测试与优化（待完善）
- [ ] 单元测试
- [ ] 集成测试
- [ ] 性能优化
- [ ] Bug 修复
- [ ] 用户体验优化

### 📅 第五阶段：发布（计划中）
- [ ] 打包安装程序
- [ ] 编写用户手册
- [ ] 发布 v1.0

## 目录结构

```
restic-gui/
├── docs/                       # 设计文档
│   ├── 需求规格说明书.md (v1.1)
│   ├── 概要设计文档.md (v1.1)
│   ├── 详细设计文档.md (v1.1)
│   ├── 数据库设计文档.md (v1.1)
│   └── 界面原型设计文档.md (v1.1)
├── restic_doc/                 # restic官方文档
├── src/                        # 源代码（75个文件，~11,300行）
│   ├── main.cpp                # 程序入口
│   ├── core/                   # 核心业务逻辑（6个管理器）
│   │   ├── ResticWrapper.h/cpp
│   │   ├── RepositoryManager.h/cpp
│   │   ├── BackupManager.h/cpp
│   │   ├── RestoreManager.h/cpp
│   │   ├── SnapshotManager.h/cpp
│   │   └── SchedulerManager.h/cpp
│   ├── ui/                     # 界面层
│   │   ├── MainWindow.h/cpp/.ui
│   │   ├── pages/              # 6个页面（.ui文件）
│   │   ├── wizards/            # 向导（纯代码）
│   │   ├── dialogs/            # 对话框（.ui文件为主）
│   │   └── widgets/            # 自定义控件（纯代码）
│   ├── models/                 # 数据模型（8个结构体）
│   ├── data/                   # 数据访问层（4个管理器）
│   └── utils/                  # 工具类（4个工具类）
├── resources/                  # 资源文件
│   ├── restic-gui.qrc          # 资源清单
│   ├── icons/                  # 图标
│   ├── sql/                    # 数据库脚本
│   ├── styles/                 # 样式表
│   └── translations/           # 翻译文件
├── build/                      # 构建输出（自动生成）
│   ├── obj/, moc/, ui/, rcc/
├── bin/                        # 可执行文件输出
│   └── restic-gui.exe
├── restic-gui.pro              # qmake配置文件
├── CLAUDE.md                   # Claude项目指导
├── UI_CONVERSION_COMPLETE.md   # UI转换完成报告
└── README.md                   # 本文件
```

**详细文件统计：**
- 头文件 (.h)：36 个
- 实现文件 (.cpp)：29 个
- UI文件 (.ui)：10 个

## 贡献指南

欢迎贡献代码、报告问题或提出建议！

### 如何贡献

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

### 代码规范

- 遵循 Qt 编码规范
- 类名使用 PascalCase
- 成员变量使用 m_ 前缀
- 函数名使用 camelCase
- 注释使用 Doxygen 格式

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

Restic 采用 BSD 2-Clause 许可证

## 致谢

- [restic](https://restic.net/) - 强大的备份工具
- [Qt](https://www.qt.io/) - 优秀的跨平台框架
- 所有贡献者和用户

## 联系方式

- 项目主页：https://github.com/yourname/restic-gui
- 问题反馈：https://github.com/yourname/restic-gui/issues
- 邮箱：yourname@example.com

---

**开发状态：** ✅ 核心功能已全部实现，项目可编译运行

**版本：** v1.1

**最后更新：** 2025-10-24 
