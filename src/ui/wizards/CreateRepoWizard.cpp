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
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, false);

    addPage(new IntroPage(this));
    addPage(new TypePage(this));
    addPage(new LocationPage(this));
    addPage(new PasswordPage(this));
    addPage(new SummaryPage(this));

    setButtonText(QWizard::FinishButton, tr("创建"));
    setButtonText(QWizard::CancelButton, tr("取消"));
    setButtonText(QWizard::BackButton, tr("上一步"));
    setButtonText(QWizard::NextButton, tr("下一步"));
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

    QLabel* introLabel = new QLabel(
        tr("Restic 仓库用于存储备份数据。您可以创建本地仓库或远程仓库。\n\n"
           "请输入仓库名称："),
        this
    );
    introLabel->setWordWrap(true);
    layout->addWidget(introLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("例如：我的备份仓库"));
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

    QLabel* label = new QLabel(tr("仓库类型："), this);
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

    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TypePage::onTypeChanged);

    layout->addWidget(m_typeComboBox);

    m_descriptionText = new QTextEdit(this);
    m_descriptionText->setReadOnly(true);
    m_descriptionText->setMaximumHeight(100);
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

    QLabel* label = new QLabel(tr("仓库路径："), this);
    layout->addWidget(label);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    pathLayout->addWidget(m_pathEdit);

    QPushButton* browseButton = new QPushButton(tr("浏览..."), this);
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
{
    setTitle(tr("设置仓库密码"));
    setSubTitle(tr("为仓库设置一个强密码。请妥善保管，丢失密码将无法恢复数据！"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* warningLabel = new QLabel(
        tr("⚠️ 警告：密码一旦丢失，将无法恢复仓库中的任何数据！"),
        this
    );
    warningLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    warningLabel->setWordWrap(true);
    layout->addWidget(warningLabel);

    QLabel* passwordLabel = new QLabel(tr("密码："), this);
    layout->addWidget(passwordLabel);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("输入密码"));
    layout->addWidget(m_passwordEdit);

    QLabel* confirmLabel = new QLabel(tr("确认密码："), this);
    layout->addWidget(confirmLabel);

    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setPlaceholderText(tr("再次输入密码"));
    layout->addWidget(m_confirmEdit);

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

// ========== SummaryPage ==========

SummaryPage::SummaryPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle(tr("确认信息"));
    setSubTitle(tr("请检查以下信息，确认无误后点击\"创建\"按钮"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_summaryText = new QTextEdit(this);
    m_summaryText->setReadOnly(true);
    layout->addWidget(m_summaryText);
}

void SummaryPage::initializePage()
{
    QString summary;
    summary += tr("<h3>仓库信息摘要</h3>");
    summary += tr("<p><b>名称：</b>%1</p>").arg(field("name").toString());

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
    default:
        typeStr = tr("其他");
        break;
    }

    summary += tr("<p><b>类型：</b>%1</p>").arg(typeStr);
    summary += tr("<p><b>路径：</b>%2</p>").arg(field("path").toString());
    summary += tr("<p><b>密码：</b>已设置（%1 个字符）</p>").arg(field("password").toString().length());

    m_summaryText->setHtml(summary);
}

} // namespace UI
} // namespace ResticGUI
