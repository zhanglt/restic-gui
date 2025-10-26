#include "PasswordDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>

namespace ResticGUI {
namespace UI {

PasswordDialog::PasswordDialog(const QString& title, const QString& message, QWidget* parent)
    : QDialog(parent),
      m_title(title),
      m_message(message),
      m_passwordEdit(nullptr),
      m_messageLabel(nullptr),
      m_okButton(nullptr),
      m_cancelButton(nullptr)
{
    setupUI();
}

PasswordDialog::~PasswordDialog()
{
}

QString PasswordDialog::getPassword() const
{
    return m_passwordEdit->text();
}

QString PasswordDialog::getPassword(QWidget* parent, const QString& title,
                                   const QString& message, bool* ok)
{
    PasswordDialog dialog(title, message, parent);
    int result = dialog.exec();

    if (ok) {
        *ok = (result == QDialog::Accepted);
    }

    return (result == QDialog::Accepted) ? dialog.getPassword() : QString();
}

void PasswordDialog::setupUI()
{
    setWindowTitle(m_title);
    setMinimumWidth(420);
    setMaximumWidth(480);

    // 设置对话框样式
    setStyleSheet(
        "QDialog {"
        "    background-color: white;"
        "}"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 25, 30, 25);

    // 顶部图标+标题布局
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    // 添加锁图标
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setFixedSize(32, 32);
    iconLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #4A90E2;"
        "    border-radius: 16px;"
        "    padding: 6px;"
        "}"
    );
    // 使用Unicode锁图标
    QLabel* lockIcon = new QLabel("🔒", iconLabel);
    lockIcon->setAlignment(Qt::AlignCenter);
    lockIcon->setStyleSheet(
        "QLabel {"
        "    background: transparent;"
        "    color: white;"
        "    font-size: 18pt;"
        "    border: none;"
        "    padding: 0px;"
        "}"
    );
    lockIcon->setGeometry(0, 0, 32, 32);

    // 标题标签
    QLabel* titleLabel = new QLabel(m_title, this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 14pt;"
        "    font-weight: bold;"
        "    color: #333333;"
        "}"
    );

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // 消息标签
    m_messageLabel = new QLabel(m_message, this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 10pt;"
        "    color: #555555;"
        "    padding: 5px 0px;"
        "}"
    );
    mainLayout->addWidget(m_messageLabel);

    // 密码输入框
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("请输入密码"));
    m_passwordEdit->setMinimumHeight(36);
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 8px 12px;"
        "    border: 2px solid #D0D0D0;"
        "    border-radius: 6px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    background-color: #F8FCFF;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #A0A0A0;"
        "}"
    );

    // 添加密码强度提示（可选）
    QLabel* hintLabel = new QLabel(tr("💡 提示：密码输入后将安全存储在内存中"), this);
    hintLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 8pt;"
        "    color: #888888;"
        "    padding: 2px 0px;"
        "}"
    );

    mainLayout->addWidget(m_passwordEdit);
    mainLayout->addWidget(hintLabel);

    mainLayout->addSpacing(10);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("取消"), this);
    m_cancelButton->setMinimumHeight(36);
    m_cancelButton->setMinimumWidth(90);
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    m_cancelButton->setStyleSheet(
        "QPushButton {"
        "    padding: 8px 20px;"
        "    border-radius: 6px;"
        "    background-color: white;"
        "    color: #666666;"
        "    border: 1px solid #CCCCCC;"
        "    font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F5F5F5;"
        "    border-color: #999999;"
        "    color: #333333;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #E0E0E0;"
        "}"
    );

    m_okButton = new QPushButton(tr("确定"), this);
    m_okButton->setMinimumHeight(36);
    m_okButton->setMinimumWidth(90);
    m_okButton->setCursor(Qt::PointingHandCursor);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet(
        "QPushButton {"
        "    padding: 8px 20px;"
        "    border-radius: 6px;"
        "    background-color: #4A90E2;"
        "    color: white;"
        "    border: 1px solid #4A90E2;"
        "    font-size: 10pt;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #357ABD;"
        "    border-color: #357ABD;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2E6BA8;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #CCCCCC;"
        "    border-color: #CCCCCC;"
        "    color: #999999;"
        "}"
    );

    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, [this]() {
        if (!m_passwordEdit->text().isEmpty()) {
            accept();
        }
    });

    // 密码输入框内容变化时，更新确定按钮状态
    connect(m_passwordEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.isEmpty());
    });

    // 初始状态：如果密码为空，禁用确定按钮
    m_okButton->setEnabled(false);

    // 设置焦点到密码输入框
    m_passwordEdit->setFocus();
}

} // namespace UI
} // namespace ResticGUI
