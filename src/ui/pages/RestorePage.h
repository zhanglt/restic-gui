#ifndef RESTOREPAGE_H
#define RESTOREPAGE_H

#include <QWidget>

namespace Ui {
class RestorePage;
}

namespace ResticGUI {
namespace UI {

class RestorePage : public QWidget
{
    Q_OBJECT

public:
    explicit RestorePage(QWidget* parent = nullptr);
    ~RestorePage();

public slots:
    void loadRepositories();

private slots:
    void onRepositoryChanged(int index);
    void loadSnapshots();
    void onBrowse();
    void onRestore();
    void onSnapshotsUpdated(int repoId);

private:
    Ui::RestorePage* ui;
    int m_currentRepositoryId;
};

} // namespace UI
} // namespace ResticGUI

#endif // RESTOREPAGE_H
