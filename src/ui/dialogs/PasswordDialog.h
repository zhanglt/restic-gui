#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;

namespace ResticGUI {
namespace UI {

/**
 * @brief 美化的密码输入对话框
 */
class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(const QString& title, const QString& message, QWidget* parent = nullptr);
    ~PasswordDialog();

    /**
     * @brief 获取用户输入的密码
     */
    QString getPassword() const;

    /**
     * @brief 静态便捷方法 - 显示密码输入对话框
     * @param parent 父窗口
     * @param title 对话框标题
     * @param message 提示信息
     * @param ok 输出参数，用户是否点击了确定
     * @return 用户输入的密码（如果取消则返回空字符串）
     */
    static QString getPassword(QWidget* parent, const QString& title,
                               const QString& message, bool* ok);

private:
    void setupUI();

private:
    QString m_title;
    QString m_message;
    QLineEdit* m_passwordEdit;
    QLabel* m_messageLabel;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

} // namespace UI
} // namespace ResticGUI

#endif // PASSWORDDIALOG_H
