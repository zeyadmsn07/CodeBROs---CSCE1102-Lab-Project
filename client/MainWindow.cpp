#include "MainWindow.h"
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("CodeBROs");
    setMinimumSize(900, 600);

    userStore = new UserStore("data/users.json");
    sessions  = new SessionStore("data/sessions.json");
    network   = new NetworkClient();

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    loginPage     = new LoginWidget();
    registerPage  = new RegisterWidget();
    dashboardPage = new DashboardWidget();   // ← real widget now

    taskPage = new QWidget();
    QLabel* taskLabel = new QLabel("Tasks Page — coming soon", taskPage);
    taskLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout* tl = new QVBoxLayout(taskPage);
    tl->addWidget(taskLabel);

    stack->addWidget(loginPage);      // 0
    stack->addWidget(registerPage);   // 1
    stack->addWidget(dashboardPage);  // 2
    stack->addWidget(taskPage);       // 3

    stack->setCurrentIndex(0);

    // ── NetworkClient callback ────────────────────────────────
    network->onMessageReceived = [this](nlohmann::json j) {
        QString str = QString::fromStdString(j.dump());
        QMetaObject::invokeMethod(this, [this, str]() {
            // Will be fully dispatched in Task 15/16
            qDebug() << "received:" << str;
        }, Qt::QueuedConnection);
    };

    // ── LoginWidget signals ───────────────────────────────────
    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this]{ stack->setCurrentIndex(1); });
    connect(loginPage, &LoginWidget::loginRequested,
            this, &MainWindow::onLoginAttempt);

    // ── RegisterWidget signals ────────────────────────────────
    connect(registerPage, &RegisterWidget::goToLoginRequested,
            [this]{ stack->setCurrentIndex(0); });
    connect(registerPage, &RegisterWidget::registerRequested,
            this, &MainWindow::onRegisterAttempt);

    // ── DashboardWidget signals ───────────────────────────────
    connect(dashboardPage, &DashboardWidget::openTasksRequested,
            [this]{ stack->setCurrentIndex(3); });
    connect(dashboardPage, &DashboardWidget::logoutRequested,
            this, &MainWindow::logout);

    // ── Menu bar logout ───────────────────────────────────────
    auto* logoutAction = menuBar()->addAction("Log Out");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);

    setupDebugToolbar();
}

void MainWindow::onLoginAttempt(const QString& username,
                                const QString& password)
{
    bool ok = userStore->authenticate(
        username.toStdString(), password.toStdString());

    if (!ok) {
        loginPage->showError("Incorrect username or password.");
        loginPage->setLoading(false);
        return;
    }

    userStore->updateLastLogin(username.toStdString());
    sessions->createSession(username.toStdString());
    goToDashboard(username);
}

void MainWindow::onRegisterAttempt(const QString& username,
                                   const QString& password)
{
    bool ok = userStore->registerUser(
        username.toStdString(), password.toStdString());

    if (!ok) {
        registerPage->showError("Username already taken.");
        registerPage->setLoading(false);
        return;
    }

    stack->setCurrentIndex(0);
    loginPage->showError("Account created! You can now log in.");
}

void MainWindow::goToDashboard(const QString& username)
{
    currentUser = username;
    dashboardPage->setRoomCode("Not in a room");
    network->connect("127.0.0.1", 12345);
    stack->setCurrentIndex(2);
}

void MainWindow::autoLogin(const QString& username)
{
    currentUser = username;
    dashboardPage->setRoomCode("Not in a room");
    stack->setCurrentIndex(2);
}

void MainWindow::logout()
{
    network->disconnect();
    sessions->clearSession();
    currentUser.clear();
    loginPage->setLoading(false);   
    loginPage->clearError();        
    stack->setCurrentIndex(0);
}

void MainWindow::setupDebugToolbar()
{
    QToolBar* bar = addToolBar("Debug");
    bar->addAction("Login",     [this]{ stack->setCurrentIndex(0); });
    bar->addAction("Register",  [this]{ stack->setCurrentIndex(1); });
    bar->addAction("Dashboard", [this]{ stack->setCurrentIndex(2); });
    bar->addAction("Tasks",     [this]{ stack->setCurrentIndex(3); });
}

MainWindow::~MainWindow()
{
    delete network;
    delete userStore;
    delete sessions;
}