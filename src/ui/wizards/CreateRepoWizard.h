#ifndef CREATEREPOWIZARD_H
#define CREATEREPOWIZARD_H

#include <QWizard>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include "../../models/Repository.h"

namespace ResticGUI {
namespace UI {

/**
 * @brief 创建仓库向导
 */
class CreateRepoWizard : public QWizard
{
    Q_OBJECT

public:
    explicit CreateRepoWizard(QWidget* parent = nullptr);
    ~CreateRepoWizard();

    Models::Repository getRepository() const;
    QString getPassword() const;

protected:
    void accept() override;

private:
    enum { Page_Intro, Page_Type, Page_Location, Page_Password, Page_Summary };

    // 保存的字段值
    mutable Models::Repository m_repository;
    mutable QString m_password;
};

/**
 * @brief 介绍页面
 */
class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit IntroPage(QWidget* parent = nullptr);

private:
    QLineEdit* m_nameEdit;
};

/**
 * @brief 仓库类型选择页面
 */
class TypePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit TypePage(QWidget* parent = nullptr);

private slots:
    void onTypeChanged(int index);

private:
    QComboBox* m_typeComboBox;
    QTextEdit* m_descriptionText;
};

/**
 * @brief 仓库位置配置页面
 */
class LocationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit LocationPage(QWidget* parent = nullptr);
    void initializePage() override;

private slots:
    void onBrowseClicked();

private:
    QLineEdit* m_pathEdit;
    QWidget* m_configWidget;
};

/**
 * @brief 密码设置页面
 */
class PasswordPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PasswordPage(QWidget* parent = nullptr);
    bool validatePage() override;

private:
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmEdit;
};

/**
 * @brief 摘要确认页面
 */
class SummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SummaryPage(QWidget* parent = nullptr);
    void initializePage() override;

private:
    QTextEdit* m_summaryText;
};

} // namespace UI
} // namespace ResticGUI

#endif // CREATEREPOWIZARD_H
