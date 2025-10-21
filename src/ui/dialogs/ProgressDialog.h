#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

namespace ResticGUI {
namespace UI {

/**
 * @brief 进度对话框
 *
 * 显示备份、恢复等长时间操作的进度
 */
class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget* parent = nullptr);
    ~ProgressDialog();

    void setTitle(const QString& title);
    void setProgress(int percent);
    void setMessage(const QString& message);
    void appendLog(const QString& log);
    void setCompleted(bool success);

signals:
    void cancelled();

private slots:
    void onCancelClicked();
    void onDetailsToggled(bool checked);

private:
    Ui::ProgressDialog* ui;
    bool m_completed;
};

} // namespace UI
} // namespace ResticGUI

#endif // PROGRESSDIALOG_H
