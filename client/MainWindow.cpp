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
    network = new NetworkClient();
    stack = new QStackedWidget(this);
    setCentralWidget(stack);
    loginPage     = new LoginWidget();
    registerPage  = new RegisterWidget();
    dashboardPage = new QWidget();
    taskPage      = new QWidget();

    QLabel* l3 = new QLabel("Dashboard Page", dashboardPage);
    QLabel* l4 = new QLabel("Tasks Page", taskPage);
    l3->setAlignment(Qt::AlignCenter);
    l4->setAlignment(Qt::AlignCenter);
    new QVBoxLayout(dashboardPage);
    static_cast<QVBoxLayout*>(dashboardPage->layout())->addWidget(l3);
    new QVBoxLayout(taskPage);
    static_cast<QVBoxLayout*>(taskPage->layout())->addWidget(l4);

    stack->addWidget(loginPage);     // 0
    stack->addWidget(registerPage);  // 1
    stack->addWidget(dashboardPage); // 2
    stack->addWidget(taskPage);      // 3

    stack->setCurrentIndex(0);

    network->onMessageReceived = [this](nlohmann::json j) {
    QString str = QString::fromStdString(j.dump());
    QMetaObject::invokeMethod(this, [this, str]() {
        auto msg = nlohmann::json::parse(str.toStdString());
        // handle incoming messages here in later tasks
        qDebug() << "received:" << str;
    }, Qt::QueuedConnection);
};

    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this]{ stack->setCurrentIndex(1); });

    connect(loginPage,    &LoginWidget::loginRequested,
            this, &MainWindow::onLoginAttempt);

    connect(registerPage, &RegisterWidget::goToLoginRequested,
            [this]{ stack->setCurrentIndex(0); });

    connect(registerPage, &RegisterWidget::registerRequested,
            this, &MainWindow::onRegisterAttempt);

    // logout in the menu bar
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

    // registration worked — go to login and let them sign in
    stack->setCurrentIndex(0);
    loginPage->showError("Account created! You can now log in.");
}

void MainWindow::goToDashboard(const QString& username)
{
    currentUser = username;
    network->connect("127.0.0.1", 12345);
    stack->setCurrentIndex(2);
}

void MainWindow::autoLogin(const QString& username)
{
    currentUser = username;
    stack->setCurrentIndex(2);
}

void MainWindow::logout()
{
    network->disconnect();
    sessions->clearSession();
    currentUser.clear();
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