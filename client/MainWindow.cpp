#include "MainWindow.h"
#include "MessageFactory.h"

#include <QToolBar>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("CodeBROs");
    setMinimumSize(900, 600);

    sessions = new SessionStore("data/sessions.json");
    network  = new NetworkClient(this);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    loginPage     = new LoginWidget(this);
    registerPage  = new RegisterWidget(this);
    roomPage      = new RoomWidget(network, this);
    dashboardPage = new DashboardWidget(this);

    stack->addWidget(loginPage);      // 0
    stack->addWidget(registerPage);   // 1
    stack->addWidget(roomPage);       // 2
    stack->addWidget(dashboardPage);  // 3

    // ── LoginWidget ──────────────────────────────────────────
    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this]{ stack->setCurrentIndex(1); });
    connect(loginPage, &LoginWidget::loginRequested,
            this, &MainWindow::onLoginAttempt);

    // ── RegisterWidget ───────────────────────────────────────
    connect(registerPage, &RegisterWidget::goToLoginRequested,
            [this]{ stack->setCurrentIndex(0); });
    connect(registerPage, &RegisterWidget::registerRequested,
            this, &MainWindow::onRegisterAttempt);

    // ── Network: auth responses ──────────────────────────────
    connect(network, &NetworkClient::loginSuccess, this,
            [this](QString username){
        currentUser = username;
        sessions->createSession(username.toStdString());
        roomPage->setUsername(username);
        roomPage->refresh();
        stack->setCurrentWidget(roomPage);
        loginPage->setLoading(false);
    });

    connect(network, &NetworkClient::loginFailed, this,
            [this](QString reason){
        loginPage->showError(reason);
        loginPage->setLoading(false);
    });

    connect(network, &NetworkClient::registerSuccess, this, [this]{
        stack->setCurrentIndex(0);
        loginPage->showError("Account created! You can now log in.");
        registerPage->setLoading(false);
    });

    connect(network, &NetworkClient::registerFailed, this,
            [this](QString reason){
        registerPage->showError(reason);
        registerPage->setLoading(false);
    });

    // ── RoomWidget → DashboardWidget ─────────────────────────
    connect(roomPage, &RoomWidget::roomEntered, this,
            [this](const QString& roomId, const QString& initialCode){
        currentRoomId = roomId;
        dashboardPage->setRoomCode(roomId);
        dashboardPage->applyRemoteCode(initialCode);
        stack->setCurrentWidget(dashboardPage);
    });

    // ── DashboardWidget signals ───────────────────────────────
    connect(dashboardPage, &DashboardWidget::logoutRequested,
            this, &MainWindow::logout);

    // FIX: Do NOT append chat locally here.
    // Server broadcasts chat_message back to ALL members including sender.
    // chatReceived signal will fire and appendChatMessage will be called once.
    connect(dashboardPage, &DashboardWidget::chatMessageEntered,
            [this](QString text){
        network->sendMessage(MessageFactory::makeChatMsg(
            currentRoomId.toStdString(),
            currentUser.toStdString(),
            text.toStdString()));
    });

    connect(dashboardPage, &DashboardWidget::codeSyncTriggered,
            [this](QString code){
        network->sendMessage(MessageFactory::makeCodeUpdate(
            currentRoomId.toStdString(),
            code.toStdString()));
    });

    // ── Network: room events → DashboardWidget ───────────────
    connect(network, &NetworkClient::codeUpdated, this,
            [this](QString code){
        dashboardPage->applyRemoteCode(code);
    });

    connect(network, &NetworkClient::chatReceived, this,
            [this](QString sender, QString text){
        dashboardPage->appendChatMessage(sender, text);
    });

    connect(network, &NetworkClient::userJoined, this,
            [this](QString username){
        dashboardPage->appendChatMessage("System", username + " joined the room");
    });

    connect(network, &NetworkClient::userLeft, this,
            [this](QString username){
        dashboardPage->appendChatMessage("System", username + " left the room");
    });

    // ── Menu bar ─────────────────────────────────────────────
    auto* logoutAction = menuBar()->addAction("Log Out");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);

    setupDebugToolbar();

    // ── Connect to server, then check for a saved session ────
    try {
        network->connect("127.0.0.1", 12345);

        QString saved = QString::fromStdString(sessions->checkSession());
        if (!saved.isEmpty()) {
            currentUser = saved;
            // Tell the server our username so disconnect broadcasts show
            // the correct name. Not a login — just registering the username.
            network->sendMessage(MessageFactory::makeSessionRestore(saved.toStdString()));
            roomPage->setUsername(saved);
            roomPage->refresh();
            stack->setCurrentWidget(roomPage);
        } else {
            stack->setCurrentWidget(loginPage);
        }
    } catch (...) {
        stack->setCurrentWidget(loginPage);
    }
}

void MainWindow::onLoginAttempt(const QString& username, const QString& password)
{
    loginPage->setLoading(true);
    network->sendMessage(MessageFactory::makeLogin(
        username.toStdString(), password.toStdString()));
}

void MainWindow::onRegisterAttempt(const QString& username, const QString& password)
{
    registerPage->setLoading(true);
    network->sendMessage(MessageFactory::makeRegister(
        username.toStdString(), password.toStdString()));
}

void MainWindow::logout()
{
    sessions->clearSession();
    currentUser.clear();
    currentRoomId.clear();
    network->sendMessage(MessageFactory::makeLeaveRoom());
    loginPage->setLoading(false);
    loginPage->clearError();
    stack->setCurrentWidget(loginPage);
}

void MainWindow::setupDebugToolbar()
{
    QToolBar* bar = addToolBar("Debug");
    bar->addAction("Login",     [this]{ stack->setCurrentIndex(0); });
    bar->addAction("Register",  [this]{ stack->setCurrentIndex(1); });
    bar->addAction("Rooms",     [this]{ stack->setCurrentIndex(2); });
    bar->addAction("Dashboard", [this]{ stack->setCurrentIndex(3); });
}

MainWindow::~MainWindow()
{
    delete sessions;
}