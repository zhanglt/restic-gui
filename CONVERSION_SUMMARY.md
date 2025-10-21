# 转换为.ui文件方式总结

## 已完成转换

### 主窗口和页面
- [x] MainWindow - 主窗口 (完整功能)
- [x] HomePage - 首页 (完整功能)
- [x] RepositoryPage - 仓库管理页 (完整功能)
- [x] BackupPage - 备份任务页 (简单占位)
- [x] SnapshotPage - 快照浏览页 (简单占位)
- [x] RestorePage - 恢复页 (简单占位)
- [x] StatsPage - 统计页 (简单占位)

### 对话框
- [x] ProgressDialog - 进度对话框 (完整功能)
- [x] SettingsDialog - 设置对话框 (完整功能)

### 保持纯代码的组件
- CreateRepoWizard - 向导（包含5个动态页面）
- SnapshotListWidget - 自定义控件
- FileTreeWidget - 自定义控件

## 修改要点

### 1. .h文件修改
```cpp
// 旧方式
#include <QWidget>
class MyPage : public QWidget {
    QLabel* m_label;
};

// 新方式
#include <QWidget>
namespace Ui {
class MyPage;
}
class MyPage : public QWidget {
    Ui::MyPage* ui;
};
```

### 2. .cpp文件修改
```cpp
// 旧方式
MyPage::MyPage() {
    setupUI();
}
void MyPage::setupUI() {
    m_label = new QLabel(...);
}

// 新方式
#include "ui_MyPage.h"
MyPage::MyPage() : ui(new Ui::MyPage) {
    ui->setupUi(this);
}
~MyPage() {
    delete ui;
}
```

### 3. 访问控件
```cpp
// 旧方式
m_label->setText("...");

// 新方式
ui->label->setText("...");
```

## 编译说明

转换后，qmake会自动：
1. 解析.ui文件
2. 生成ui_XXX.h头文件
3. 将生成的文件放在build/ui目录

## .pro文件配置

已在restic-gui.pro中添加：
```qmake
FORMS += \
    src/ui/MainWindow.ui \
    src/ui/pages/HomePage.ui \
    src/ui/pages/RepositoryPage.ui \
    src/ui/pages/BackupPage.ui \
    src/ui/pages/SnapshotPage.ui \
    src/ui/pages/RestorePage.ui \
    src/ui/pages/StatsPage.ui \
    src/ui/dialogs/ProgressDialog.ui \
    src/ui/dialogs/SettingsDialog.ui
```

