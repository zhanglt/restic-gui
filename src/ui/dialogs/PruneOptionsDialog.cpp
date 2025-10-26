#include "PruneOptionsDialog.h"
#include "ui_PruneOptionsDialog.h"

namespace ResticGUI {
namespace UI {

PruneOptionsDialog::PruneOptionsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PruneOptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

PruneOptionsDialog::~PruneOptionsDialog()
{
    delete ui;
}

int PruneOptionsDialog::getKeepLast() const
{
    return ui->keepLastSpinBox->value();
}

int PruneOptionsDialog::getKeepDaily() const
{
    return ui->keepDailySpinBox->value();
}

int PruneOptionsDialog::getKeepWeekly() const
{
    return ui->keepWeeklySpinBox->value();
}

int PruneOptionsDialog::getKeepMonthly() const
{
    return ui->keepMonthlySpinBox->value();
}

int PruneOptionsDialog::getKeepYearly() const
{
    return ui->keepYearlySpinBox->value();
}

void PruneOptionsDialog::setKeepPolicy(int keepLast, int keepDaily, int keepWeekly,
                                      int keepMonthly, int keepYearly)
{
    ui->keepLastSpinBox->setValue(keepLast);
    ui->keepDailySpinBox->setValue(keepDaily);
    ui->keepWeeklySpinBox->setValue(keepWeekly);
    ui->keepMonthlySpinBox->setValue(keepMonthly);
    ui->keepYearlySpinBox->setValue(keepYearly);
}

} // namespace UI
} // namespace ResticGUI
