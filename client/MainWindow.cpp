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

    // ── Stack ─────────────────────────────────────────────────
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
    // Start with an empty, invisible menu bar
    menuBar()->setVisible(false);

    // "Back to Rooms" — shown only while in Dashboard
    backToRoomsAction = menuBar()->addAction("← Rooms");
    connect(backToRoomsAction, &QAction::triggered, this, [this] {
        roomPage->refresh();
        showPage(roomPage);
    });

    // Spacer-style separator
    menuBar()->addSeparator();

    // Logged-in user display — we update the text dynamically
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
    connect(roomPage, &RoomWidget::roomEntered, this,
            [this](const QString& roomId, const QString& initialCode) {
        currentRoomId = roomId;
        dashboardPage->setRoomCode(roomId);
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
        // Do NOT call appendChatMessage here.
        // The server echoes every chat message back to all clients
        // (including the sender), so chatReceived below will display it.
        // Appending locally too would show it twice.
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
    });

    // ── Startup: try saved session, else show login ───────────
    try {
        network->connect("127.0.0.1", 12345);
        QString saved = QString::fromStdString(sessions->checkSession());
        if (!saved.isEmpty()) {
            currentUser = saved;
            roomPage->setUsername(saved);
            roomPage->refresh();
            setLoggedIn(true);
            showPage(roomPage);
        } else {
            showPage(loginPage);
        }
    } catch (...) {
        showPage(loginPage);
    }
}

// ── Helpers ───────────────────────────────────────────────────

void MainWindow::showPage(QWidget* page)
{
    stack->setCurrentWidget(page);

    // "← Rooms" only makes sense when inside the dashboard
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