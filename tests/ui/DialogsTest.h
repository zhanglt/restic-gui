#ifndef DIALOGSTEST_H
#define DIALOGSTEST_H

#include "common/TestBase.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief Dialogs 测试类
 *
 * 测试各种对话框的功能：
 * - ProgressDialog - 进度对话框
 * - SettingsDialog - 设置对话框
 * - PasswordDialog - 密码输入对话框
 * - CreateTaskDialog - 创建任务对话框
 */
class DialogsTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;
    void init() override;
    void cleanup() override;

    // 进度对话框测试
    void testProgressDialogCreation();

    // 设置对话框测试
    void testSettingsDialogCreation();

    // 密码对话框测试
    void testPasswordDialogCreation();

    // 创建任务对话框测试
    void testCreateTaskDialogCreation();
};

} // namespace Test
} // namespace ResticGUI

#endif // DIALOGSTEST_H
