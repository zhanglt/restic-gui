/**
 * @file CreateRepoWizard.cpp
 * @brief 创建仓库向导实现
 */

#include "CreateRepoWizard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

namespace ResticGUI {
namespace UI {

CreateRepoWizard::CreateRepoWizard(QWidget* parent)
    : QWizard(parent)
{
    setWindowTitle(tr("创建仓库向导"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, false);

    // 设置窗口大小
    setMinimumSize(650, 550);
    resize(700, 600);

    addPage(new IntroPage(this));
    addPage(new TypePage(this));
    addPage(new LocationPage(this));
    addPage(new PasswordPage(this));
    addPage(new SummaryPage(this));

    setButtonText(QWizard::FinishButton, tr("创建"));
    setButtonText(QWizard::CancelButton, tr("取消"));
    setButtonText(QWizard::BackButton, tr("上一步"));
    setButtonText(QWizard::NextButton, tr("下一步"));

    // 应用整体样式
    setStyleSheet(
        // 向导页面样式
        "QWizard {"
        "    background-color: #F5F5F5;"
        "}"
        "QWizardPage {"
        "    background-color: white;"
        "}"

        // 标题样式
        "QWizard QLabel[class=\"title\"] {"
        "    font-size: 14pt;"
        "    font-weight: bold;"
        "    color: #333333;"
        "}"

        // 向导按钮样式
        "QWizard QPushButton {"
        "    min-width: 55px;"
        "    min-height: 20px;"
        "    padding: 5px 14px;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "}"

        // 下一步和完成按钮（主要按钮）
        "QWizard QPushButton[text=\"下一步\"], "
        "QWizard QPushButton[text=\"创建\"] {"
        "    background-color: #4A90E2;"
        "    color: white;"
        "    border: 1px solid #4A90E2;"
        "    font-weight: bold;"
        "}"
        "QWizard QPushButton[text=\"下一步\"]:hover, "
        "QWizard QPushButton[text=\"创建\"]:hover {"
        "    background-color: #357ABD;"
        "    border-color: #357ABD;"
        "}"
        "QWizard QPushButton[text=\"下一步\"]:pressed, "
        "QWizard QPushButton[text=\"创建\"]:pressed {"
        "    background-color: #2E6BA8;"
        "}"
        "QWizard QPushButton[text=\"下一步\"]:disabled, "
        "QWizard QPushButton[text=\"创建\"]:disabled {"
        "    background-color: #CCCCCC;"
        "    border-color: #CCCCCC;"
        "    color: #888888;"
        "}"

        // 上一步按钮（次要按钮）
        "QWizard QPushButton[text=\"上一步\"] {"
        "    background-color: white;"
        "    color: #4A90E2;"
        "    border: 1px solid #4A90E2;"
        "}"
        "QWizard QPushButton[text=\"上一步\"]:hover {"
        "    background-color: #F0F8FF;"
        "    border-color: #4A90E2;"
        "}"
        "QWizard QPushButton[text=\"上一步\"]:pressed {"
        "    background-color: #E0F0FF;"
        "}"

        // 取消按钮
        "QWizard QPushButton[text=\"取消\"] {"
        "    background-color: white;"
        "    color: #666666;"
        "    border: 1px solid #CCCCCC;"
        "}"
        "QWizard QPushButton[text=\"取消\"]:hover {"
        "    background-color: #F5F5F5;"
        "    border-color: #999999;"
        "    color: #333333;"
        "}"
        "QWizard QPushButton[text=\"取消\"]:pressed {"
        "    background-color: #E0E0E0;"
        "}"
    );
}

CreateRepoWizard::~CreateRepoWizard()
{
}

void CreateRepoWizard::accept()
{
    // 在对话框关闭前保存字段值
    m_repository.name = field("name").toString();
    m_repository.type = static_cast<Models::RepositoryType>(field("type").toInt());
    m_repository.path = field("path").toString();
    m_repository.createdAt = QDateTime::currentDateTime();

    m_password = field("password").toString();

    // 调用基类的accept()
    QWizard::accept();
}

Models::Repository CreateRepoWizard::getRepository() const
{
    return m_repository;
}

QString CreateRepoWizard::getPassword() const
{
    return m_password;
}

// ========== IntroPage ==========

IntroPage::IntroPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle(tr("欢迎"));
    setSubTitle(tr("此向导将帮助您创建一个新的 Restic 仓库"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    // 欢迎信息区域
    QLabel* introLabel = new QLabel(
        tr("Restic 仓库用于存储备份数据。您可以创建本地仓库或远程仓库。"),
        this
    );
    introLabel->setWordWrap(true);
    introLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 11pt;"
        "    color: #555555;"
        "    line-height: 1.5;"
        "}"
    );
    layout->addWidget(introLabel);

    layout->addSpacing(20);

    // 仓库名称输入
    QLabel* nameLabel = new QLabel(tr("请输入仓库名称："), this);
    nameLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    layout->addWidget(nameLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("例如：我的备份仓库"));
    m_nameEdit->setMinimumHeight(30);
    m_nameEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #A0A0A0;"
        "}"
    );
    layout->addWidget(m_nameEdit);

    layout->addStretch();

    registerField("name*", m_nameEdit);
}

// ========== TypePage ==========

TypePage::TypePage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle(tr("选择仓库类型"));
    setSubTitle(tr("选择仓库的存储位置类型"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* label = new QLabel(tr("仓库类型："), this);
    label->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    layout->addWidget(label);

    m_typeComboBox = new QComboBox(this);
    m_typeComboBox->addItem(tr("本地文件系统"), static_cast<int>(Models::RepositoryType::Local));
    m_typeComboBox->addItem(tr("SFTP"), static_cast<int>(Models::RepositoryType::SFTP));
    m_typeComboBox->addItem(tr("Amazon S3"), static_cast<int>(Models::RepositoryType::S3));
    m_typeComboBox->addItem(tr("REST 服务器"), static_cast<int>(Models::RepositoryType::REST));
    m_typeComboBox->addItem(tr("Azure Blob"), static_cast<int>(Models::RepositoryType::Azure));
    m_typeComboBox->addItem(tr("Google Cloud Storage"), static_cast<int>(Models::RepositoryType::GS));
    m_typeComboBox->addItem(tr("Backblaze B2"), static_cast<int>(Models::RepositoryType::B2));
    m_typeComboBox->addItem(tr("Rclone"), static_cast<int>(Models::RepositoryType::Rclone));

    m_typeComboBox->setMinimumHeight(30);
    m_typeComboBox->setStyleSheet(
        "QComboBox {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QComboBox:hover {"
        "    border-color: #A0A0A0;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 25px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #666666;"
        "    margin-right: 6px;"
        "}"
    );

    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TypePage::onTypeChanged);

    layout->addWidget(m_typeComboBox);

    layout->addSpacing(10);

    m_descriptionText = new QTextEdit(this);
    m_descriptionText->setReadOnly(true);
    m_descriptionText->setMaximumHeight(100);
    m_descriptionText->setStyleSheet(
        "QTextEdit {"
        "    padding: 12px;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    background-color: #F8F9FA;"
        "    font-size: 10pt;"
        "    color: #555555;"
        "}"
    );
    layout->addWidget(m_descriptionText);

    layout->addStretch();

    registerField("type", m_typeComboBox, "currentData");

    onTypeChanged(0);
}

void TypePage::onTypeChanged(int index)
{
    int type = m_typeComboBox->itemData(index).toInt();

    QString description;
    switch (static_cast<Models::RepositoryType>(type)) {
    case Models::RepositoryType::Local:
        description = tr("本地文件系统仓库将数据存储在本地磁盘上。适合个人电脑备份。");
        break;
    case Models::RepositoryType::SFTP:
        description = tr("SFTP 仓库通过 SSH 协议将数据存储在远程服务器上。需要 SSH 访问权限。");
        break;
    case Models::RepositoryType::S3:
        description = tr("Amazon S3 仓库将数据存储在 AWS S3 服务中。需要 AWS 访问密钥。");
        break;
    case Models::RepositoryType::REST:
        description = tr("REST 服务器仓库使用 restic 的 REST 协议。需要运行 rest-server。");
        break;
    default:
        description = tr("其他云存储服务。");
        break;
    }

    m_descriptionText->setText(description);
}

// ========== LocationPage ==========

LocationPage::LocationPage(QWidget* parent)
    : QWizardPage(parent)
    , m_configWidget(nullptr)
{
    setTitle(tr("配置仓库位置"));
    setSubTitle(tr("指定仓库的存储位置"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* label = new QLabel(tr("仓库路径："), this);
    label->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    layout->addWidget(label);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->setSpacing(10);

    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setMinimumHeight(30);
    m_pathEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #A0A0A0;"
        "}"
    );
    pathLayout->addWidget(m_pathEdit, 1);

    QPushButton* browseButton = new QPushButton(tr("浏览..."), this);
    browseButton->setMinimumHeight(30);
    browseButton->setMinimumWidth(75);
    browseButton->setCursor(Qt::PointingHandCursor);
    browseButton->setStyleSheet(
        "QPushButton {"
        "    padding: 5px 14px;"
        "    border: 1px solid #4A90E2;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    color: #4A90E2;"
        "    font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4A90E2;"
        "    color: white;"
        "    border-color: #4A90E2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #357ABD;"
        "    border-color: #357ABD;"
        "}"
    );
    connect(browseButton, &QPushButton::clicked, this, &LocationPage::onBrowseClicked);
    pathLayout->addWidget(browseButton);

    layout->addLayout(pathLayout);

    // 配置选项区域（根据仓库类型动态显示）
    m_configWidget = new QWidget(this);
    layout->addWidget(m_configWidget);

    layout->addStretch();

    registerField("path*", m_pathEdit);
}

void LocationPage::initializePage()
{
    // 根据选择的仓库类型显示相应的配置界面
    int type = field("type").toInt();
    Models::RepositoryType repoType = static_cast<Models::RepositoryType>(type);

    // 清除旧的配置界面
    if (m_configWidget->layout()) {
        QLayout* oldLayout = m_configWidget->layout();
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    // 根据类型创建新的配置界面
    QVBoxLayout* configLayout = new QVBoxLayout(m_configWidget);

    switch (repoType) {
    case Models::RepositoryType::Local:
        // 本地仓库不需要额外配置
        break;

    case Models::RepositoryType::SFTP:
        configLayout->addWidget(new QLabel(tr("SFTP 配置 (可选):")));
        // TODO: 添加 SFTP 相关配置选项（服务器地址、用户名等）
        break;

    case Models::RepositoryType::S3:
        configLayout->addWidget(new QLabel(tr("S3 配置 (可选):")));
        // TODO: 添加 S3 相关配置选项（访问密钥、区域等）
        break;

    case Models::RepositoryType::REST:
        configLayout->addWidget(new QLabel(tr("REST 服务器配置 (可选):")));
        // TODO: 添加 REST 服务器配置选项
        break;

    default:
        break;
    }
}

void LocationPage::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择仓库目录"),
        QDir::homePath()
    );

    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

// ========== PasswordPage ==========

PasswordPage::PasswordPage(QWidget* parent)
    : QWizardPage(parent)
    , m_passwordVisible(false)
{
    setTitle(tr("设置仓库密码"));
    setSubTitle(tr("为仓库设置一个强密码。请妥善保管，丢失密码将无法恢复数据！"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    // 警告区域
    QWidget* warningWidget = new QWidget(this);
    warningWidget->setStyleSheet(
        "QWidget {"
        "    background-color: #FFF3CD;"
        "    border-left: 4px solid #FFC107;"
        "    border-radius: 6px;"
        "}"
    );
    QHBoxLayout* warningLayout = new QHBoxLayout(warningWidget);
    warningLayout->setContentsMargins(12, 12, 12, 12);

    QLabel* warningIcon = new QLabel(tr("⚠️"), this);
    warningIcon->setStyleSheet("QLabel { font-size: 18pt; background: transparent; border: none; }");
    warningLayout->addWidget(warningIcon);

    QLabel* warningLabel = new QLabel(
        tr("<b>警告：密码一旦丢失，将无法恢复仓库中的任何数据！</b>"),
        this
    );
    warningLabel->setStyleSheet("QLabel { color: #856404; font-size: 10pt; background: transparent; border: none; }");
    warningLabel->setWordWrap(true);
    warningLayout->addWidget(warningLabel, 1);

    layout->addWidget(warningWidget);

    layout->addSpacing(10);

    // 密码输入
    QLabel* passwordLabel = new QLabel(tr("密码："), this);
    passwordLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    layout->addWidget(passwordLabel);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    passwordLayout->setSpacing(10);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("输入密码（至少8个字符）"));
    m_passwordEdit->setMinimumHeight(30);
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
    );
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &PasswordPage::onPasswordChanged);
    passwordLayout->addWidget(m_passwordEdit, 1);

    m_togglePasswordButton = new QPushButton(tr("👁"), this);
    m_togglePasswordButton->setMinimumHeight(30);
    m_togglePasswordButton->setFixedWidth(36);
    m_togglePasswordButton->setCursor(Qt::PointingHandCursor);
    m_togglePasswordButton->setStyleSheet(
        "QPushButton {"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    font-size: 13pt;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F0F0F0;"
        "    border-color: #A0A0A0;"
        "}"
    );
    connect(m_togglePasswordButton, &QPushButton::clicked, this, &PasswordPage::onTogglePasswordVisibility);
    passwordLayout->addWidget(m_togglePasswordButton);

    layout->addLayout(passwordLayout);

    // 密码强度指示器
    QHBoxLayout* strengthLayout = new QHBoxLayout();
    strengthLayout->setSpacing(10);

    m_strengthBar = new QProgressBar(this);
    m_strengthBar->setRange(0, 100);
    m_strengthBar->setValue(0);
    m_strengthBar->setTextVisible(false);
    m_strengthBar->setMaximumHeight(8);
    m_strengthBar->setStyleSheet(
        "QProgressBar {"
        "    border: none;"
        "    border-radius: 4px;"
        "    background-color: #E0E0E0;"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 4px;"
        "    background-color: #E74C3C;"
        "}"
    );
    strengthLayout->addWidget(m_strengthBar, 1);

    m_strengthLabel = new QLabel(tr("强度：无"), this);
    m_strengthLabel->setStyleSheet("QLabel { font-size: 9pt; color: #888888; min-width: 60px; }");
    strengthLayout->addWidget(m_strengthLabel);

    layout->addLayout(strengthLayout);

    layout->addSpacing(10);

    // 确认密码输入
    QLabel* confirmLabel = new QLabel(tr("确认密码："), this);
    confirmLabel->setStyleSheet("QLabel { font-size: 10pt; font-weight: bold; color: #333333; }");
    layout->addWidget(confirmLabel);

    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setPlaceholderText(tr("再次输入密码"));
    m_confirmEdit->setMinimumHeight(30);
    m_confirmEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    font-size: 10pt;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #4A90E2;"
        "    padding: 4px 9px;"
        "    background-color: #F8FCFF;"
        "}"
    );
    connect(m_confirmEdit, &QLineEdit::textChanged, this, &PasswordPage::onConfirmPasswordChanged);
    layout->addWidget(m_confirmEdit);

    // 密码匹配状态
    m_matchLabel = new QLabel(this);
    m_matchLabel->setStyleSheet("QLabel { font-size: 9pt; min-height: 20px; }");
    layout->addWidget(m_matchLabel);

    layout->addStretch();

    registerField("password*", m_passwordEdit);
}

bool PasswordPage::validatePage()
{
    if (m_passwordEdit->text() != m_confirmEdit->text()) {
        QMessageBox::warning(this, tr("密码不匹配"),
            tr("两次输入的密码不一致，请重新输入。"));
        m_confirmEdit->clear();
        m_confirmEdit->setFocus();
        return false;
    }

    if (m_passwordEdit->text().length() < 8) {
        QMessageBox::warning(this, tr("密码太短"),
            tr("为了安全，密码长度至少需要 8 个字符。"));
        return false;
    }

    return true;
}

void PasswordPage::onPasswordChanged(const QString& password)
{
    // 计算密码强度
    int strength = calculatePasswordStrength(password);

    m_strengthBar->setValue(strength);

    // 更新强度标签和颜色
    QString strengthText;
    QString barColor;

    if (strength == 0) {
        strengthText = tr("强度：无");
        barColor = "#E0E0E0";
    } else if (strength < 30) {
        strengthText = tr("强度：弱");
        barColor = "#E74C3C";
    } else if (strength < 60) {
        strengthText = tr("强度：中");
        barColor = "#F39C12";
    } else if (strength < 80) {
        strengthText = tr("强度：较强");
        barColor = "#3498DB";
    } else {
        strengthText = tr("强度：强");
        barColor = "#27AE60";
    }

    m_strengthLabel->setText(strengthText);
    m_strengthBar->setStyleSheet(QString(
        "QProgressBar {"
        "    border: none;"
        "    border-radius: 4px;"
        "    background-color: #E0E0E0;"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 4px;"
        "    background-color: %1;"
        "}"
    ).arg(barColor));

    // 检查密码匹配状态
    onConfirmPasswordChanged(m_confirmEdit->text());
}

void PasswordPage::onConfirmPasswordChanged(const QString& password)
{
    QString passwordText = m_passwordEdit->text();

    if (password.isEmpty()) {
        m_matchLabel->clear();
        return;
    }

    if (password == passwordText) {
        m_matchLabel->setText(tr("✓ 密码匹配"));
        m_matchLabel->setStyleSheet("QLabel { color: #27AE60; font-size: 9pt; }");
    } else {
        m_matchLabel->setText(tr("✗ 密码不匹配"));
        m_matchLabel->setStyleSheet("QLabel { color: #E74C3C; font-size: 9pt; }");
    }
}

void PasswordPage::onTogglePasswordVisibility()
{
    m_passwordVisible = !m_passwordVisible;

    if (m_passwordVisible) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_confirmEdit->setEchoMode(QLineEdit::Normal);
        m_togglePasswordButton->setText(tr("🔒"));
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_confirmEdit->setEchoMode(QLineEdit::Password);
        m_togglePasswordButton->setText(tr("👁"));
    }
}

int PasswordPage::calculatePasswordStrength(const QString& password)
{
    if (password.isEmpty()) {
        return 0;
    }

    int strength = 0;
    int length = password.length();

    // 长度评分 (最多40分)
    if (length >= 8) strength += 20;
    if (length >= 12) strength += 10;
    if (length >= 16) strength += 10;

    // 字符类型评分
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (const QChar& ch : password) {
        if (ch.isLower()) hasLower = true;
        else if (ch.isUpper()) hasUpper = true;
        else if (ch.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }

    // 每种字符类型15分
    if (hasLower) strength += 15;
    if (hasUpper) strength += 15;
    if (hasDigit) strength += 15;
    if (hasSpecial) strength += 15;

    // 限制最大值为100
    if (strength > 100) strength = 100;

    return strength;
}

// ========== SummaryPage ==========

SummaryPage::SummaryPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle(tr("确认信息"));
    setSubTitle(tr("请检查以下信息，确认无误后点击\"创建\"按钮"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    m_summaryText = new QTextEdit(this);
    m_summaryText->setReadOnly(true);
    m_summaryText->setStyleSheet(
        "QTextEdit {"
        "    border: 2px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    background-color: #FAFAFA;"
        "    padding: 15px;"
        "    font-size: 10pt;"
        "}"
    );
    layout->addWidget(m_summaryText);
}

void SummaryPage::initializePage()
{
    int type = field("type").toInt();
    QString typeStr;
    switch (static_cast<Models::RepositoryType>(type)) {
    case Models::RepositoryType::Local:
        typeStr = tr("本地文件系统");
        break;
    case Models::RepositoryType::SFTP:
        typeStr = tr("SFTP");
        break;
    case Models::RepositoryType::S3:
        typeStr = tr("Amazon S3");
        break;
    case Models::RepositoryType::REST:
        typeStr = tr("REST 服务器");
        break;
    case Models::RepositoryType::Azure:
        typeStr = tr("Azure Blob");
        break;
    case Models::RepositoryType::GS:
        typeStr = tr("Google Cloud Storage");
        break;
    case Models::RepositoryType::B2:
        typeStr = tr("Backblaze B2");
        break;
    case Models::RepositoryType::Rclone:
        typeStr = tr("Rclone");
        break;
    default:
        typeStr = tr("其他");
        break;
    }

    QString summary = QString(
        "<div style='font-family: sans-serif;'>"
        "<h2 style='color: #333; margin-bottom: 20px; border-bottom: 2px solid #4A90E2; padding-bottom: 8px;'>📋 仓库信息摘要</h2>"

        "<div style='background-color: white; padding: 15px; border-radius: 6px; margin-bottom: 12px;'>"
        "<p style='margin: 8px 0;'><span style='color: #666; font-weight: bold;'>仓库名称：</span>"
        "<span style='color: #333;'>%1</span></p>"
        "</div>"

        "<div style='background-color: white; padding: 15px; border-radius: 6px; margin-bottom: 12px;'>"
        "<p style='margin: 8px 0;'><span style='color: #666; font-weight: bold;'>仓库类型：</span>"
        "<span style='color: #333;'>%2</span></p>"
        "</div>"

        "<div style='background-color: white; padding: 15px; border-radius: 6px; margin-bottom: 12px;'>"
        "<p style='margin: 8px 0;'><span style='color: #666; font-weight: bold;'>存储路径：</span>"
        "<span style='color: #333; word-break: break-all;'>%3</span></p>"
        "</div>"

        "<div style='background-color: white; padding: 15px; border-radius: 6px; margin-bottom: 12px;'>"
        "<p style='margin: 8px 0;'><span style='color: #666; font-weight: bold;'>密码设置：</span>"
        "<span style='color: #27AE60;'>✓ 已设置（%4 个字符）</span></p>"
        "</div>"

        "<div style='background-color: #E8F5E9; padding: 12px; border-radius: 6px; border-left: 4px solid #27AE60; margin-top: 20px;'>"
        "<p style='margin: 5px 0; color: #2E7D32; font-size: 9pt;'>"
        "💡 <b>提示：</b>点击\"创建\"按钮后，系统将初始化仓库。请确保上述信息准确无误。"
        "</p>"
        "</div>"
        "</div>"
    ).arg(field("name").toString())
     .arg(typeStr)
     .arg(field("path").toString())
     .arg(field("password").toString().length());

    m_summaryText->setHtml(summary);
}

} // namespace UI
} // namespace ResticGUI
