#ifndef MAINWINDOWTEST_H
#define MAINWINDOWTEST_H

#include "common/TestBase.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief MainWindow 测试类
 *
 * 测试主窗口的功能：
 * - 窗口创建和显示
 * - 菜单栏和工具栏
 * - 页面切换
 * - 系统托盘功能
 * - 状态栏更新
 */
class MainWindowTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 窗口基础测试
    void testWindowCreation();
    void testWindowTitle();
    void testWindowResize();

    // 页面切换测试
    void testPageSwitching();

    // 状态栏测试
    void testStatusBarUpdate();
};

} // namespace Test
} // namespace ResticGUI

#endif // MAINWINDOWTEST_H
