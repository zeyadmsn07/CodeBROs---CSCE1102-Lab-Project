#include "MainWindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <iostream>

#include "MessageFactory.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
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

    stack->addWidget(loginPage);     // 0
    stack->addWidget(registerPage);  // 1
    stack->addWidget(roomPage);      // 2
    stack->addWidget(dashboardPage); // 3

    // ── navigation ───────────────────────────────────────────

    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this]() { stack->setCurrentIndex(1); });

    connect(loginPage, &LoginWidget::loginRequested,
            this, &MainWindow::onLoginAttempt);

    connect(registerPage, &RegisterWidget::goToLoginRequested,
            [this]() { stack->setCurrentIndex(0); });

    connect(registerPage, &RegisterWidget::registerRequested,
            this, &MainWindow::onRegisterAttempt);

    // ── auth responses ────────────────────────────────────────

    connect(network, &NetworkClient::loginSuccess, this, [this](QString username) {
        currentUser = username;
        sessions->createSession(username.toStdString());
        roomPage->setUsername(username);
        roomPage->setCurrentRoomId("");   // fresh login — not in any room
        roomPage->refresh();
        stack->setCurrentWidget(roomPage);
        loginPage->setLoading(false);
    });

    connect(network, &NetworkClient::loginFailed, this, [this](QString reason) {
        loginPage->showError(reason);
        loginPage->setLoading(false);
    });

    connect(network, &NetworkClient::registerSuccess, this, [this]() {
        stack->setCurrentIndex(0);
        loginPage->showError("Account created! You can now log in.");
        registerPage->setLoading(false);
    });

    connect(network, &NetworkClient::registerFailed, this, [this](QString reason) {
        registerPage->showError(reason);
        registerPage->setLoading(false);
    });

    // ── entering a room → switch to dashboard ────────────────

    connect(roomPage, &RoomWidget::roomEntered, this,
        [this](const QString& roomId,
               const QString& roomName,
               const QString& initialCode) {
            currentRoomId = roomId;
            roomPage->setCurrentRoomId(roomId);   // block further joins

            dashboardPage->setRoomCode(roomName); // show room name in sidebar
            dashboardPage->applyRemoteCode(initialCode);
            stack->setCurrentWidget(dashboardPage);
        });

    // ── member list updates ───────────────────────────────────

    connect(network, &NetworkClient::memberListReceived, this,
        [this](QStringList members) {
            dashboardPage->updateMemberList(members);
        });

    // ── logout ────────────────────────────────────────────────

    connect(dashboardPage, &DashboardWidget::logoutRequested,
            this, &MainWindow::logout);

    // ── chat ──────────────────────────────────────────────────

    connect(dashboardPage, &DashboardWidget::chatMessageEntered,
        [this](QString text) {
            network->sendMessage(MessageFactory::makeChatMsg(
                currentRoomId.toStdString(),
                currentUser.toStdString(),
                text.toStdString()));
        });

    // ── code sync ─────────────────────────────────────────────

    connect(dashboardPage, &DashboardWidget::codeSyncTriggered,
        [this](QString code) {
            network->sendMessage(MessageFactory::makeCodeUpdate(
                currentRoomId.toStdString(),
                code.toStdString()));
        });

    connect(network, &NetworkClient::codeUpdated, this,
        [this](QString code) { dashboardPage->applyRemoteCode(code); });

    // ── incoming chat ─────────────────────────────────────────

    connect(network, &NetworkClient::chatReceived, this,
        [this](QString sender, QString text) {
            dashboardPage->appendChatMessage(sender, text);
        });

    // ── join/leave notifications ──────────────────────────────

    connect(network, &NetworkClient::userJoined, this,
        [this](QString username) {
            dashboardPage->appendChatMessage(
                "System", username + " joined the room");
        });

    connect(network, &NetworkClient::userLeft, this,
        [this](QString username) {
            dashboardPage->appendChatMessage(
                "System", username + " left the room");
        });

    // ── join failed (e.g. already in room) ───────────────────

    connect(network, &NetworkClient::joinFailed, this,
        [this](QString reason) {
            // only show if we're on the room page
            // (the RoomWidget already shows its own QMessageBox,
            //  but the server-side check sends this too)
        });

    // ── menu bar logout ───────────────────────────────────────

    QAction* logoutAction = menuBar()->addAction("Log Out");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);

    setupDebugToolbar();

    // ── startup: connect and check session ───────────────────

    try {
        network->connect("127.0.0.1", 12345);

        std::string saved = sessions->checkSession();
        if (!saved.empty()) {
            currentUser = QString::fromStdString(saved);
            network->sendMessage(MessageFactory::makeSessionRestore(saved));
            roomPage->setUsername(currentUser);
            roomPage->setCurrentRoomId("");
            roomPage->refresh();
            stack->setCurrentWidget(roomPage);
        } else {
            stack->setCurrentWidget(loginPage);
        }
    } catch (...) {
        stack->setCurrentWidget(loginPage);
    }
}

void MainWindow::onLoginAttempt(const QString& username,
                                const QString& password) {
    loginPage->setLoading(true);
    network->sendMessage(MessageFactory::makeLogin(
        username.toStdString(), password.toStdString()));
}

void MainWindow::onRegisterAttempt(const QString& username,
                                   const QString& password) {
    registerPage->setLoading(true);
    network->sendMessage(MessageFactory::makeRegister(
        username.toStdString(), password.toStdString()));
}

void MainWindow::logout() {
    sessions->clearSession();
    currentUser.clear();
    currentRoomId.clear();

    roomPage->setCurrentRoomId("");   // reset the guard

    network->sendMessage(MessageFactory::makeLeaveRoom());

    loginPage->setLoading(false);
    loginPage->clearError();
    stack->setCurrentWidget(loginPage);
}

void MainWindow::setupDebugToolbar() {
    QToolBar* bar = addToolBar("Debug");
    bar->addAction("Login",     [this]() { stack->setCurrentIndex(0); });
    bar->addAction("Register",  [this]() { stack->setCurrentIndex(1); });
    bar->addAction("Rooms",     [this]() { stack->setCurrentIndex(2); });
    bar->addAction("Dashboard", [this]() { stack->setCurrentIndex(3); });
}

MainWindow::~MainWindow() { delete sessions; }