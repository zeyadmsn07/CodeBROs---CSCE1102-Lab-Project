#include "MainWindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <iostream>  // often left in by beginners for debugging

#include "MessageFactory.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("CodeBROs");
    setMinimumSize(900, 600);

    sessions = new SessionStore("data/sessions.json");
    network = new NetworkClient(this);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    loginPage = new LoginWidget(this);
    registerPage = new RegisterWidget(this);
    roomPage = new RoomWidget(network, this);
    dashboardPage = new DashboardWidget(this);

    stack->addWidget(loginPage);      // 0
    stack->addWidget(registerPage);   // 1
    stack->addWidget(roomPage);       // 2
    stack->addWidget(dashboardPage);  // 3

    // navigation signals
    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this]() { stack->setCurrentIndex(1); });

    connect(loginPage, &LoginWidget::loginRequested, this, &MainWindow::onLoginAttempt);

    connect(registerPage, &RegisterWidget::goToLoginRequested,
            [this]() { stack->setCurrentIndex(0); });

    connect(registerPage, &RegisterWidget::registerRequested, this, &MainWindow::onRegisterAttempt);

    // auth responses from server
    connect(network, &NetworkClient::loginSuccess, this, [this](QString username) {
        currentUser = username;
        std::string userStr = username.toStdString();
        sessions->createSession(userStr);
        roomPage->setUsername(username);
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

    // switch to dashboard when entering a room
    connect(roomPage, &RoomWidget::roomEntered, this,
            [this](const QString& roomId, const QString& roomName, const QString& initialCode) {
                currentRoomId = roomId;
                dashboardPage->setRoomCode(roomName);
                dashboardPage->applyRemoteCode(initialCode);
                stack->setCurrentWidget(dashboardPage);
            });

    connect(dashboardPage, &DashboardWidget::logoutRequested, this, &MainWindow::logout);

    // handle chat messages
    connect(dashboardPage, &DashboardWidget::chatMessageEntered, [this](QString text) {
        std::string roomStr = currentRoomId.toStdString();
        std::string userStr = currentUser.toStdString();
        std::string textStr = text.toStdString();

        network->sendMessage(MessageFactory::makeChatMsg(roomStr, userStr, textStr));
    });

    connect(dashboardPage, &DashboardWidget::codeSyncTriggered,
            [this](QString code) {  // handle code updates
                std::string roomStr = currentRoomId.toStdString();
                std::string codeStr = code.toStdString();

                network->sendMessage(MessageFactory::makeCodeUpdate(roomStr, codeStr));
            });

    connect(network, &NetworkClient::codeUpdated, this,
            [this](QString code) { dashboardPage->applyRemoteCode(code); });

    connect(network, &NetworkClient::chatReceived, this, [this](QString sender, QString text) {
        dashboardPage->appendChatMessage(sender, text);
    });

    connect(network, &NetworkClient::userJoined, this, [this](QString username) {
        QString msg = username + " joined the room";
        dashboardPage->appendChatMessage("System", msg);
    });

    connect(network, &NetworkClient::userLeft, this, [this](QString username) {
        QString msg = username + " left the room";
        dashboardPage->appendChatMessage("System", msg);
    });

    QAction* logoutAction = menuBar()->addAction("Log Out");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);

    setupDebugToolbar();
    try {
        network->connect("127.0.0.1", 12345);

        std::string sessionStr = sessions->checkSession();
        QString saved = QString::fromStdString(sessionStr);

        if (saved != "") {
            currentUser = saved;
            std::string savedStd = saved.toStdString();
            network->sendMessage(MessageFactory::makeSessionRestore(savedStd));
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

void MainWindow::onLoginAttempt(const QString& username, const QString& password) {
    loginPage->setLoading(true);

    std::string userStr = username.toStdString();
    std::string passStr = password.toStdString();

    network->sendMessage(MessageFactory::makeLogin(userStr, passStr));
}

void MainWindow::onRegisterAttempt(const QString& username, const QString& password) {
    registerPage->setLoading(true);

    std::string userStr = username.toStdString();
    std::string passStr = password.toStdString();

    network->sendMessage(MessageFactory::makeRegister(userStr, passStr));
}

void MainWindow::logout() {
    sessions->clearSession();
    currentUser.clear();
    currentRoomId.clear();

    network->sendMessage(MessageFactory::makeLeaveRoom());

    loginPage->setLoading(false);
    loginPage->clearError();
    stack->setCurrentWidget(loginPage);
}

void MainWindow::setupDebugToolbar() {
    QToolBar* bar = addToolBar("Debug");

    bar->addAction("Login", [this]() { stack->setCurrentIndex(0); });
    bar->addAction("Register", [this]() { stack->setCurrentIndex(1); });
    bar->addAction("Rooms", [this]() { stack->setCurrentIndex(2); });
    bar->addAction("Dashboard", [this]() { stack->setCurrentIndex(3); });
}

MainWindow::~MainWindow() { delete sessions; }