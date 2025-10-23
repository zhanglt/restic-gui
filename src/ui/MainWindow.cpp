/**
 * @file MainWindow.cpp
 * @brief 主窗口实现
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "pages/HomePage.h"
#include "pages/RepositoryPage.h"
#include "pages/BackupPage.h"
#include "pages/SnapshotPage.h"
#include "pages/RestorePage.h"
#include "pages/StatsPage.h"
#include "dialogs/SettingsDialog.h"
#include "wizards/CreateRepoWizard.h"
#include "../data/ConfigManager.h"
#include "../core/RepositoryManager.h"
#include "../utils/Logger.h"

#include <QLabel>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QIcon>

namespace ResticGUI {
namespace UI {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_homePage(nullptr)
    , m_repositoryPage(nullptr)
    , m_backupPage(nullptr)
    , m_snapshotPage(nullptr)
    , m_restorePage(nullptr)
    , m_statsPage(nullptr)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_currentRepositoryId(-1)
{
    ui->setupUi(this);

    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/app.ico"));

    createPages();
    setupConnections();
    createSystemTray();
    loadSettings();

    // 默认显示首页
    ui->navigationList->setCurrentRow(0);

    Utils::Logger::instance()->log(Utils::Logger::Info, "主窗口已创建");
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::createPages()
{
    // 创建各个页面
    m_homePage = new HomePage(this);
    m_repositoryPage = new RepositoryPage(this);
    m_backupPage = new BackupPage(this);
    m_snapshotPage = new SnapshotPage(this);
    m_restorePage = new RestorePage(this);
    m_statsPage = new StatsPage(this);

    // 添加到堆叠窗口
    ui->pageStack->addWidget(m_homePage);
    ui->pageStack->addWidget(m_repositoryPage);
    ui->pageStack->addWidget(m_backupPage);
    ui->pageStack->addWidget(m_snapshotPage);
    ui->pageStack->addWidget(m_restorePage);
    ui->pageStack->addWidget(m_statsPage);
}

void MainWindow::setupConnections()
{
    // 导航
    connect(ui->navigationList, &QListWidget::currentRowChanged,
            this, &MainWindow::onNavigationChanged);

    // 文件菜单
    connect(ui->actionNewRepository, &QAction::triggered, this, &MainWindow::onNewRepository);
    connect(ui->actionOpenRepository, &QAction::triggered, this, &MainWindow::onOpenRepository);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // 编辑菜单
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);

    // 工具菜单
    connect(ui->actionCheckRepository, &QAction::triggered, this, &MainWindow::onCheckRepository);
    connect(ui->actionUnlockRepository, &QAction::triggered, this, &MainWindow::onUnlockRepository);
    connect(ui->actionPruneRepository, &QAction::triggered, this, &MainWindow::onPruneRepository);

    // 帮助菜单
    connect(ui->actionDocumentation, &QAction::triggered, this, &MainWindow::showDocumentation);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // 工具栏
    connect(ui->actionBackupNow, &QAction::triggered, this, &MainWindow::onBackupNow);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::onRefresh);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::onStop);

    // 监听仓库列表变化
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    connect(repoMgr, &Core::RepositoryManager::repositoryListChanged,
            m_snapshotPage, &SnapshotPage::loadRepositories);
    connect(repoMgr, &Core::RepositoryManager::repositoryListChanged,
            m_restorePage, &RestorePage::loadRepositories);

    // 状态栏初始化
    QLabel* statusLabel = new QLabel(tr("就绪"), this);
    ui->statusBar->addWidget(statusLabel);

    QLabel* repoLabel = new QLabel(tr("当前仓库: 无"), this);
    ui->statusBar->addPermanentWidget(repoLabel);
}

void MainWindow::onNavigationChanged(int index)
{
    ui->pageStack->setCurrentIndex(index);

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("切换到页面: %1").arg(index));
}

void MainWindow::onRepositoryChanged()
{
    // 获取默认仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getDefaultRepository();

    // 更新状态栏
    QList<QLabel*> labels = ui->statusBar->findChildren<QLabel*>();
    if (labels.size() >= 2) {
        QLabel* repoLabel = labels.at(1); // 第二个标签是仓库标签
        if (repo.id > 0) {
            repoLabel->setText(tr("当前仓库: %1").arg(repo.name));
        } else {
            repoLabel->setText(tr("当前仓库: 无"));
        }
    }
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 设置已在 SettingsDialog 中自动保存
        Utils::Logger::instance()->log(Utils::Logger::Info, "设置已更新");
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("关于 Restic GUI"),
        tr("<h3>Restic GUI v1.0.0</h3>"
           "<p>基于Qt的Restic备份工具图形界面</p>"
           "<p>Copyright © 2024</p>"
           "<p>使用 Qt %1</p>").arg(QT_VERSION_STR));
}

void MainWindow::loadSettings()
{
    Data::ConfigManager* config = Data::ConfigManager::instance();

    // 恢复窗口几何信息
    QByteArray geometry = config->getWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    // 恢复窗口状态
    QByteArray state = config->getWindowState();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // 恢复上次选中的仓库
    m_currentRepositoryId = config->getLastSelectedRepository();
}

void MainWindow::saveSettings()
{
    Data::ConfigManager* config = Data::ConfigManager::instance();

    // 保存窗口几何信息
    config->setWindowGeometry(saveGeometry());

    // 保存窗口状态
    config->setWindowState(saveState());

    // 保存当前选中的仓库
    config->setLastSelectedRepository(m_currentRepositoryId);

    config->sync();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 询问是否退出
    int ret = QMessageBox::question(this, tr("确认退出"),
        tr("确定要退出应用程序吗？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        saveSettings();
        event->accept();
        Utils::Logger::instance()->log(Utils::Logger::Info, "应用程序退出");
    } else {
        event->ignore();
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);

    // 处理窗口最小化事件
    if (event->type() == QEvent::WindowStateChange) {
        Data::ConfigManager* config = Data::ConfigManager::instance();

        // 如果启用了"最小化到系统托盘"选项，并且窗口被最小化
        if (config->getMinimizeToTray() && isMinimized()) {
            // 隐藏窗口
            hide();

            // 显示托盘提示
            if (m_trayIcon && m_trayIcon->isVisible()) {
                m_trayIcon->showMessage(
                    tr("Restic GUI"),
                    tr("程序已最小化到系统托盘"),
                    QSystemTrayIcon::Information,
                    2000
                );
            }

            event->ignore();
        }
    }
}

// 文件菜单槽函数
void MainWindow::onNewRepository()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "打开新建仓库向导");

    CreateRepoWizard wizard(this);
    if (wizard.exec() == QDialog::Accepted) {
        // 仓库创建成功，刷新仓库列表
        m_repositoryPage->loadRepositories();

        // 切换到仓库管理页面
        ui->navigationList->setCurrentRow(1);

        QMessageBox::information(this, tr("成功"),
            tr("仓库创建成功！"));
    }
}

void MainWindow::onOpenRepository()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "切换到仓库管理页面");

    // 切换到仓库管理页面
    ui->navigationList->setCurrentRow(1);
}

// 工具菜单槽函数
void MainWindow::onCheckRepository()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "检查仓库");

    // 获取当前选中的仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getDefaultRepository();

    if (repo.id <= 0) {
        QMessageBox::warning(this, tr("警告"),
            tr("请先选择一个仓库！"));
        return;
    }

    QMessageBox::information(this, tr("检查仓库"),
        tr("正在检查仓库 \"%1\"...\n此功能即将实现。").arg(repo.name));
}

void MainWindow::onUnlockRepository()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "解锁仓库");

    // 获取当前选中的仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getDefaultRepository();

    if (repo.id <= 0) {
        QMessageBox::warning(this, tr("警告"),
            tr("请先选择一个仓库！"));
        return;
    }

    int ret = QMessageBox::question(this, tr("确认解锁"),
        tr("确定要解锁仓库 \"%1\" 吗？\n这将删除该仓库的锁文件。").arg(repo.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QMessageBox::information(this, tr("解锁仓库"),
            tr("仓库解锁功能即将实现。"));
    }
}

void MainWindow::onPruneRepository()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "维护仓库");

    // 获取当前选中的仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getDefaultRepository();

    if (repo.id <= 0) {
        QMessageBox::warning(this, tr("警告"),
            tr("请先选择一个仓库！"));
        return;
    }

    int ret = QMessageBox::question(this, tr("确认维护"),
        tr("确定要维护仓库 \"%1\" 吗？\n这将执行 prune 操作，删除不再需要的数据。").arg(repo.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QMessageBox::information(this, tr("维护仓库"),
            tr("仓库维护功能即将实现。"));
    }
}

// 帮助菜单槽函数
void MainWindow::showDocumentation()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "打开文档");

    QMessageBox::information(this, tr("文档"),
        tr("文档功能即将实现。\n\n您可以访问以下链接获取帮助：\n"
           "Restic官方文档: https://restic.readthedocs.io/"));
}

// 工具栏槽函数
void MainWindow::onBackupNow()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "立即备份");

    // 获取当前选中的仓库
    Core::RepositoryManager* repoMgr = Core::RepositoryManager::instance();
    Models::Repository repo = repoMgr->getDefaultRepository();

    if (repo.id <= 0) {
        QMessageBox::warning(this, tr("警告"),
            tr("请先选择一个仓库！"));
        return;
    }

    // 切换到备份任务页面
    ui->navigationList->setCurrentRow(2);

    QMessageBox::information(this, tr("立即备份"),
        tr("请在备份任务页面选择或创建备份任务。"));
}

void MainWindow::onRefresh()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "刷新当前页面");

    // 根据当前页面执行相应的刷新操作
    int currentIndex = ui->pageStack->currentIndex();

    switch (currentIndex) {
        case 0: // 首页
            // 首页不需要刷新
            break;
        case 1: // 仓库管理
            m_repositoryPage->loadRepositories();
            break;
        case 2: // 备份任务
            m_backupPage->loadTasks();
            break;
        case 3: // 快照管理
            m_snapshotPage->loadSnapshots();
            break;
        case 4: // 数据恢复
            m_restorePage->loadRepositories();
            break;
        case 5: // 统计信息
            m_statsPage->loadStats();
            break;
        default:
            break;
    }

    ui->statusBar->showMessage(tr("已刷新"), 2000);
}

void MainWindow::onStop()
{
    Utils::Logger::instance()->log(Utils::Logger::Info, "停止操作");

    int ret = QMessageBox::question(this, tr("确认停止"),
        tr("确定要停止当前操作吗？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QMessageBox::information(this, tr("停止操作"),
            tr("停止操作功能即将实现。"));
    }
}

// 系统托盘相关函数
void MainWindow::createSystemTray()
{
    // 检查系统是否支持托盘
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        Utils::Logger::instance()->log(Utils::Logger::Warning,
            "系统不支持系统托盘功能");
        return;
    }

    // 创建托盘图标
    m_trayIcon = new QSystemTrayIcon(this);

    // 设置托盘图标
    QIcon icon(":/icons/app.ico");
    if (icon.isNull()) {
        Utils::Logger::instance()->log(Utils::Logger::Warning,
            "无法加载托盘图标，尝试使用窗口图标");
        icon = windowIcon();
    }
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip(tr("Restic GUI - 备份管理工具"));

    // 创建托盘菜单
    m_trayMenu = new QMenu(this);

    QAction* showAction = new QAction(tr("显示主窗口"), this);
    connect(showAction, &QAction::triggered, this, &MainWindow::onShowMainWindow);

    QAction* exitAction = new QAction(tr("退出"), this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExitApplication);

    m_trayMenu->addAction(showAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(exitAction);

    m_trayIcon->setContextMenu(m_trayMenu);

    // 连接托盘图标激活信号
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayIconActivated);

    // 显示托盘图标
    m_trayIcon->show();

    Utils::Logger::instance()->log(Utils::Logger::Info, "系统托盘已创建");
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            // 单击或双击托盘图标，显示/隐藏主窗口
            if (isVisible()) {
                hide();
            } else {
                onShowMainWindow();
            }
            break;
        default:
            break;
    }
}

void MainWindow::onShowMainWindow()
{
    // 显示主窗口
    show();
    // 恢复窗口状态（如果是最小化的）
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    // 激活窗口并置于顶层
    raise();
    activateWindow();
}

void MainWindow::onExitApplication()
{
    // 使用 this 作为父窗口，配合 setQuitOnLastWindowClosed(false) 确保对话框关闭不会退出程序
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("确认退出"));
    msgBox.setText(tr("确定要退出应用程序吗？"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    // 设置窗口标志，确保对话框能正常显示（即使主窗口隐藏）
    msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);

    int ret = msgBox.exec();

    Utils::Logger::instance()->log(Utils::Logger::Debug,
        QString("用户选择: %1").arg(ret == QMessageBox::Yes ? "Yes" : "No"));

    if (ret == QMessageBox::Yes) {
        saveSettings();
        Utils::Logger::instance()->log(Utils::Logger::Info, "从系统托盘退出应用程序");

        // 先隐藏托盘图标再退出
        if (m_trayIcon) {
            m_trayIcon->hide();
        }

        qApp->quit();
    } else {
        Utils::Logger::instance()->log(Utils::Logger::Debug, "用户取消退出");

        // 确保托盘图标可见
        if (m_trayIcon && !m_trayIcon->isVisible()) {
            m_trayIcon->show();
            Utils::Logger::instance()->log(Utils::Logger::Warning, "托盘图标被意外隐藏，已重新显示");
        }
    }
}

} // namespace UI
} // namespace ResticGUI
