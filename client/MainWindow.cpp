#include "MainWindow.h"
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenuBar>
#include <QRandomGenerator>
#include "MessageFactory.h"
#include "MessageTypes.h"

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
    dashboardPage = new DashboardWidget();

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

    // ── NetworkClient callback — fires on Boost thread ────────
    // MUST use QMetaObject::invokeMethod to cross to Qt main thread
    network->onMessageReceived = [this](nlohmann::json j) {
        QMetaObject::invokeMethod(this, [this, j]() {
            onNetworkMessage(j);
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

    // Chat → send CHAT message to server
    connect(dashboardPage, &DashboardWidget::chatMessageEntered,
            [this](const QString& text) {
                if (network && !currentPartyId.isEmpty()) {
                    auto msg = MessageFactory::buildChat(
                        currentUser.toStdString(),
                        currentPartyId.toStdString(),
                        text.toStdString());
                    network->sendMessage(MessageFactory::toJson(msg));
                }
            });

    // Code sync → send CODE_SYNC message to server
    connect(dashboardPage, &DashboardWidget::codeSyncTriggered,
            [this](const QString& code) {
                if (network && !currentPartyId.isEmpty()) {
                    auto msg = MessageFactory::buildCodeSync(
                        currentUser.toStdString(),
                        currentPartyId.toStdString(),
                        code.toStdString());
                    network->sendMessage(MessageFactory::toJson(msg));
                }
            });

    // ── Menu bar logout ───────────────────────────────────────
    auto* logoutAction = menuBar()->addAction("Log Out");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);

    setupDebugToolbar();
}

// ── generates a random 6-char party code ─────────────────────
QString MainWindow::generatePartyCode()
{
    const QString pool = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString code;
    for (int i = 0; i < 6; ++i)
        code += pool[QRandomGenerator::global()->bounded(pool.size())];
    return code;
}

// ── login attempt ─────────────────────────────────────────────
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

// ── register attempt ──────────────────────────────────────────
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

// ── go to dashboard: connect + JOIN ──────────────────────────
void MainWindow::goToDashboard(const QString& username)
{
    currentUser    = username;
    currentPartyId = generatePartyCode();   // each login gets a fresh party

    try {
        network->connect("127.0.0.1", 12345);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Connection Failed",
            "Could not reach the server.\n"
            "Make sure the server is running.\n\n"
            + QString(e.what()));
        loginPage->setLoading(false);
        return;
    }

    // Send JOIN so the server adds us to the room
    auto joinMsg = MessageFactory::buildJoin(
        currentUser.toStdString(),
        currentPartyId.toStdString());
    network->sendMessage(MessageFactory::toJson(joinMsg));

    dashboardPage->setRoomCode(currentPartyId);
    stack->setCurrentIndex(2);
}

// ── auto-login from saved session (no server connect yet) ─────
void MainWindow::autoLogin(const QString& username)
{
    currentUser    = username;
    currentPartyId = generatePartyCode();
    dashboardPage->setRoomCode(currentPartyId);
    stack->setCurrentIndex(2);
}

// ── incoming message dispatcher ───────────────────────────────
// Runs on Qt main thread (via QMetaObject::invokeMethod)
void MainWindow::onNetworkMessage(const nlohmann::json& msg)
{
    std::string type = msg.value("type", "");

    if (type == MessageTypes::MEMBER_LIST) {
        // payload is a JSON array of username strings
        try {
            auto arr = nlohmann::json::parse(
                msg.value("payload", "[]"));
            QStringList members;
            for (auto& m : arr)
                members << QString::fromStdString(m.get<std::string>());
            dashboardPage->updateMemberList(members);
        }
        catch (...) {}
    }
    else if (type == MessageTypes::CHAT) {
        QString sender = QString::fromStdString(msg.value("sender",  ""));
        QString text   = QString::fromStdString(msg.value("payload", ""));
        dashboardPage->appendChatMessage(sender, text);
    }
    else if (type == MessageTypes::CODE_SYNC) {
        QString code = QString::fromStdString(msg.value("payload", ""));
        dashboardPage->applyRemoteCode(code);
    }
    else if (type == MessageTypes::ERROR) {
        QString reason = QString::fromStdString(msg.value("payload", ""));
        QMessageBox::warning(this, "Server Error", reason);
    }
    else if (type == MessageTypes::AUTH_FAIL) {
        loginPage->showError("Authentication failed.");
        loginPage->setLoading(false);
        stack->setCurrentIndex(0);
    }
}

// ── logout ────────────────────────────────────────────────────
void MainWindow::logout()
{
    // Politely tell the server we're leaving
    if (!currentPartyId.isEmpty()) {
        try {
            auto leaveMsg = MessageFactory::buildLeave(
                currentUser.toStdString(),
                currentPartyId.toStdString());
            network->sendMessage(MessageFactory::toJson(leaveMsg));
        }
        catch (...) {}
    }

    network->disconnect();
    sessions->clearSession();
    currentUser.clear();
    currentPartyId.clear();
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