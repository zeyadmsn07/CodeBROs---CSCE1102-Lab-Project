#include "MainWindow.h"
#include "MessageFactory.h"
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

    // ── Menu bar (hidden until logged in) ─────────────────────
    menuBar()->setVisible(false);
    backToRoomsAction = menuBar()->addAction("← Rooms");
    connect(backToRoomsAction, &QAction::triggered, this, [this] {
        network->sendMessage(MessageFactory::makeLeaveRoom());
        currentRoomId.clear();
        roomPage->setCurrentRoomId("");
        roomPage->refresh();
        showPage(roomPage);
    });
    menuBar()->addSeparator();
    logoutAction = menuBar()->addAction("Log Out");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);

    // ── LoginWidget ───────────────────────────────────────────
    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this] { showPage(registerPage); });
    connect(loginPage, &LoginWidget::loginRequested,
            this, &MainWindow::onLoginAttempt);

    // ── RegisterWidget ────────────────────────────────────────
    connect(registerPage, &RegisterWidget::goToLoginRequested,
            [this] { showPage(loginPage); });
    connect(registerPage, &RegisterWidget::registerRequested,
            this, &MainWindow::onRegisterAttempt);

    // ── Network: auth responses ───────────────────────────────
    connect(network, &NetworkClient::loginSuccess, this,
            [this](QString username) {
        currentUser = username;
        sessions->createSession(username.toStdString());
        roomPage->setUsername(username);
        roomPage->refresh();
        loginPage->setLoading(false);
        setLoggedIn(true);
        showPage(roomPage);
    });

    connect(network, &NetworkClient::loginFailed, this,
            [this](QString reason) {
        loginPage->showError(reason);
        loginPage->setLoading(false);
    });

    connect(network, &NetworkClient::registerSuccess, this, [this] {
        registerPage->setLoading(false);
        showPage(loginPage);
        loginPage->showError("Account created! You can now log in.");
    });

    connect(network, &NetworkClient::registerFailed, this,
            [this](QString reason) {
        registerPage->showError(reason);
        registerPage->setLoading(false);
    });

    // ── RoomWidget → DashboardWidget ──────────────────────────
    // FIX: signal now carries roomName as second parameter
    connect(roomPage, &RoomWidget::roomEntered, this,
            [this](const QString& roomId, const QString& roomName, const QString& initialCode) {
        currentRoomId = roomId;
        dashboardPage->setRoomName(roomName);      // FIX: show room name, not ROOM_1
        dashboardPage->applyRemoteCode(initialCode);
        showPage(dashboardPage);
    });

    // ── DashboardWidget signals ───────────────────────────────
    connect(dashboardPage, &DashboardWidget::logoutRequested,
            this, &MainWindow::logout);

    connect(dashboardPage, &DashboardWidget::chatMessageEntered,
            [this](QString text) {
        network->sendMessage(MessageFactory::makeChatMsg(
            currentRoomId.toStdString(),
            currentUser.toStdString(),
            text.toStdString()));
        // Do NOT appendChatMessage here — server echoes it back to all clients
        // including the sender, so chatReceived below will display it once.
    });

    connect(dashboardPage, &DashboardWidget::codeSyncTriggered,
            [this](QString code) {
        network->sendMessage(MessageFactory::makeCodeUpdate(
            currentRoomId.toStdString(),
            code.toStdString()));
    });

    // ── Network: room events → DashboardWidget ────────────────
    connect(network, &NetworkClient::codeUpdated, this,
            [this](QString code) {
        dashboardPage->applyRemoteCode(code);
    });

    connect(network, &NetworkClient::chatReceived, this,
            [this](QString sender, QString text) {
        dashboardPage->appendChatMessage(sender, text);
    });

    connect(network, &NetworkClient::userJoined, this,
            [this](QString username) {
        dashboardPage->appendChatMessage("System", username + " joined the room");
    });

    connect(network, &NetworkClient::userLeft, this,
            [this](QString username) {
        dashboardPage->appendChatMessage("System", username + " left the room");
        network->sendMessage(MessageFactory::makeGetRooms()); // refresh count
    });

    // FIX: wire up member list — was missing in the latest version
    connect(network, &NetworkClient::memberListReceived, this,
            [this](QStringList members) {
        dashboardPage->updateMemberList(members);
    });

    // ── Startup: FIX — always show login, never auto-restore session ──
    try {
        network->connect("127.0.0.1", 12345);
    } catch (...) {}
    showPage(loginPage);
}

// ── Helpers ───────────────────────────────────────────────────

void MainWindow::showPage(QWidget* page)
{
    stack->setCurrentWidget(page);
    backToRoomsAction->setVisible(page == dashboardPage);
}

void MainWindow::setLoggedIn(bool loggedIn)
{
    menuBar()->setVisible(loggedIn);
    if (loggedIn) {
        logoutAction->setText("Log Out  (" + currentUser + ")");
    }
}

// ── Auth actions ──────────────────────────────────────────────

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
    setLoggedIn(false);
    loginPage->setLoading(false);
    loginPage->clearError();
    showPage(loginPage);
}

MainWindow::~MainWindow()
{
    delete sessions;
}