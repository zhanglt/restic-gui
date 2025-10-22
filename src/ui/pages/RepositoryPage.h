#ifndef REPOSITORYPAGE_H
#define REPOSITORYPAGE_H

#include <QWidget>
#include <QFutureWatcher>
#include <QTimer>
#include "../../models/Repository.h"

namespace Ui {
class RepositoryPage;
}

namespace ResticGUI {
namespace UI {

class ProgressDialog;

class RepositoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit RepositoryPage(QWidget* parent = nullptr);
    ~RepositoryPage();

public slots:
    void loadRepositories();

private slots:
    void onCreateRepository();
    void onEditRepository();
    void onDeleteRepository();
    void onRefresh();
    void onCheckRepository();
    void onUnlockRepository();
    void onPruneRepository();
    void onCreateRepositoryFinished();
    void onCheckRepositoryFinished();
    void onUnlockRepositoryFinished();
    void onPruneRepositoryFinished();
    void onUpdateProgress();
    void onProgressCancelled();

private:

    Ui::RepositoryPage* ui;
    QFutureWatcher<int>* m_createRepoWatcher;
    QFutureWatcher<bool>* m_checkRepoWatcher;
    QFutureWatcher<bool>* m_unlockRepoWatcher;
    QFutureWatcher<bool>* m_pruneRepoWatcher;
    ProgressDialog* m_progressDialog;
    QTimer* m_progressTimer;
    QTimer* m_timeoutTimer;
    int m_progressValue;
    Models::Repository m_pendingRepo;
    QString m_pendingPassword;
    int m_currentOperationRepoId;
    QString m_currentOperationPassword;
};

} // namespace UI
} // namespace ResticGUI

#endif // REPOSITORYPAGE_H
