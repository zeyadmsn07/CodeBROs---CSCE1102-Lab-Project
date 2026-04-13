#include "LoginWidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QVBoxLayout>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    applyStyles();
}

void LoginWidget::setupUi() {
    QWidget* card = new QWidget(this);
    card->setObjectName("card");
    card->setMinimumWidth(580);
    card->setMaximumWidth(740);
    QWidget* leftPanel = new QWidget(card);
    leftPanel->setObjectName("leftPanel");

    QLabel* title = new QLabel(">_CodeBROs", leftPanel);
    title->setObjectName("titleLabel");
    QFont monoFont("Courier New", 28);
    monoFont.setBold(true);
    title->setFont(monoFont);

    QLabel* subtitle = new QLabel("// collaborative coding\n// for beginners", leftPanel);
    subtitle->setObjectName("subtitleLabel");

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(32, 36, 24, 36);
    leftLayout->addStretch();
    leftLayout->addWidget(title);
    leftLayout->addSpacing(8);
    leftLayout->addWidget(subtitle);
    leftLayout->addStretch();

    // right panel — form
    QWidget* rightPanel = new QWidget(card);

    QLabel* formTitle = new QLabel("// login", rightPanel);
    formTitle->setObjectName("formTitle");

    QLabel* userLbl = new QLabel("USERNAME", rightPanel);
    userLbl->setObjectName("fieldLabel");

    usernameInput = new QLineEdit(rightPanel);
    usernameInput->setObjectName("inputField");
    usernameInput->setPlaceholderText("enter username");
    usernameInput->setMinimumHeight(38);

    QLabel* passLbl = new QLabel("PASSWORD", rightPanel);
    passLbl->setObjectName("fieldLabel");

    passwordInput = new QLineEdit(rightPanel);
    passwordInput->setObjectName("inputField");
    passwordInput->setPlaceholderText("enter password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(38);

    errorLabel = new QLabel("", rightPanel);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->hide();

    loginButton = new QPushButton("[ LOG IN ]", rightPanel);
    loginButton->setObjectName("loginButton");
    loginButton->setMinimumHeight(40);
    loginButton->setCursor(Qt::PointingHandCursor);

    registerLink = new QPushButton("no account? create one", rightPanel);
    registerLink->setObjectName("registerLink");
    registerLink->setFlat(true);
    registerLink->setCursor(Qt::PointingHandCursor);

    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(24, 36, 32, 36);
    rightLayout->addWidget(formTitle);
    rightLayout->addSpacing(14);
    rightLayout->addWidget(userLbl);
    rightLayout->addWidget(usernameInput);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(passLbl);
    rightLayout->addWidget(passwordInput);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(errorLabel);
    rightLayout->addSpacing(12);
    rightLayout->addWidget(loginButton);
    rightLayout->addSpacing(6);
    rightLayout->addWidget(registerLink);
    rightLayout->addStretch();

    QHBoxLayout* cardLayout = new QHBoxLayout(card);  // card: two panels side by side
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);
    cardLayout->addWidget(leftPanel, 1);
    cardLayout->addWidget(rightPanel, 2);

    QVBoxLayout* outer = new QVBoxLayout(this);  // center card in window
    outer->setContentsMargins(20, 20, 20, 20);
    outer->addStretch();
    QHBoxLayout* row = new QHBoxLayout();
    row->addStretch();
    row->addWidget(card);
    row->addStretch();
    outer->addLayout(row);
    outer->addStretch();

    connect(loginButton, &QPushButton::clicked, [this]() {
        QString u = usernameInput->text().trimmed();
        QString p = passwordInput->text();
        if (u.isEmpty() || p.isEmpty()) {
            showError("please fill in all fields.");
            return;
        }
        clearError();
        setLoading(true);
        emit loginRequested(u, p);
    });

    connect(passwordInput, &QLineEdit::returnPressed, loginButton, &QPushButton::click);

    connect(registerLink, &QPushButton::clicked, [this]() { emit goToRegisterRequested(); });
}

void LoginWidget::applyStyles() {
    setStyleSheet(
        "QWidget#card {"
        "  background-color: rgba(0,0,0,0.55);"
        "  border: 1px solid rgba(57,255,20,0.25);"
        "  border-radius: 6px;"
        "}"
        "QWidget#leftPanel {"
        "  border-right: 1px solid rgba(57,255,20,0.15);"
        "}"
        "QLabel#titleLabel {"
        "  color: #39ff14;"
        "  font-family: 'Courier New';"
        "}"
        "QLabel#subtitleLabel {"
        "  color: rgba(57,255,20,0.45);"
        "  font-family: 'Courier New';"
        "  font-size: 12px;"
        "}"
        "QLabel#formTitle {"
        "  color: rgba(57,255,20,0.50);"
        "  font-family: 'Courier New';"
        "  font-size: 12px;"
        "}"
        "QLabel#fieldLabel {"
        "  color: rgba(57,255,20,0.45);"
        "  font-family: 'Courier New';"
        "  font-size: 10px;"
        "  letter-spacing: 1px;"
        "}"
        "QLineEdit#inputField {"
        "  background-color: rgba(57,255,20,0.04);"
        "  border: 1px solid rgba(57,255,20,0.20);"
        "  border-radius: 3px;"
        "  color: #ffffff;"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "  padding: 0 10px;"
        "}"
        "QLineEdit#inputField:focus {"
        "  border: 1px solid rgba(57,255,20,0.60);"
        "}"
        "QPushButton#loginButton {"
        "  background-color: transparent;"
        "  color: #39ff14;"
        "  border: 1px solid #39ff14;"
        "  border-radius: 3px;"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "  letter-spacing: 2px;"
        "}"
        "QPushButton#loginButton:hover {"
        "  background-color: rgba(57,255,20,0.10);"
        "}"
        "QPushButton#loginButton:pressed {"
        "  background-color: rgba(57,255,20,0.20);"
        "}"
        "QPushButton#loginButton:disabled {"
        "  color: rgba(57,255,20,0.30);"
        "  border-color: rgba(57,255,20,0.20);"
        "}"
        "QPushButton#registerLink {"
        "  color: rgba(57,255,20,0.50);"
        "  font-family: 'Courier New';"
        "  font-size: 11px;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton#registerLink:hover {"
        "  color: #39ff14;"
        "}"
        "QLabel#errorLabel {"
        "  color: #ff5555;"
        "  font-family: 'Courier New';"
        "  font-size: 12px;"
        "}");
}

void LoginWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QPixmap bg(":/images/background.jpg");
    painter.drawPixmap(rect(),
                       bg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    QWidget::paintEvent(event);
}

void LoginWidget::showError(const QString& msg) {
    errorLabel->setText(msg);
    errorLabel->show();
}

void LoginWidget::clearError() {
    errorLabel->clear();
    errorLabel->hide();
}

void LoginWidget::setLoading(bool on) {
    loginButton->setEnabled(!on);
    loginButton->setText(on ? "[ logging in... ]" : "[ LOG IN ]");
    usernameInput->setEnabled(!on);
    passwordInput->setEnabled(!on);
}