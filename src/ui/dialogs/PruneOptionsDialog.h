#ifndef PRUNEOPTIONSDIALOG_H
#define PRUNEOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class PruneOptionsDialog;
}

namespace ResticGUI {
namespace UI {

/**
 * @brief 维护仓库（Prune）选项对话框
 *
 * 让用户配置快照保留策略
 */
class PruneOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PruneOptionsDialog(QWidget* parent = nullptr);
    ~PruneOptionsDialog();

    /**
     * @brief 获取保留最近N个快照
     */
    int getKeepLast() const;

    /**
     * @brief 获取保留每日快照天数
     */
    int getKeepDaily() const;

    /**
     * @brief 获取保留每周快照周数
     */
    int getKeepWeekly() const;

    /**
     * @brief 获取保留每月快照月数
     */
    int getKeepMonthly() const;

    /**
     * @brief 获取保留每年快照年数
     */
    int getKeepYearly() const;

    /**
     * @brief 设置保留策略
     */
    void setKeepPolicy(int keepLast, int keepDaily, int keepWeekly,
                      int keepMonthly, int keepYearly);

private:
    Ui::PruneOptionsDialog* ui;
};

} // namespace UI
} // namespace ResticGUI

#endif // PRUNEOPTIONSDIALOG_H
