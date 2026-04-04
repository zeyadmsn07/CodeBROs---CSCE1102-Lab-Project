#include "LoginWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>

LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    applyStyles();
}

void LoginWidget::setupUi()
{
    QWidget* card = new QWidget(this);
    card->setObjectName("card");
    card->setFixedSize(400, 600);

    QLabel* title = new QLabel("CodeBROs", card);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);

    QLabel* subtitle = new QLabel(
        "Collaborative coding for beginners", card);
    subtitle->setObjectName("subtitleLabel");
    subtitle->setAlignment(Qt::AlignCenter);

    QLabel* userLbl = new QLabel("USERNAME", card);
    userLbl->setObjectName("fieldLabel");

    usernameInput = new QLineEdit(card);
    usernameInput->setObjectName("inputField");
    usernameInput->setPlaceholderText("Enter your username");
    usernameInput->setMinimumHeight(42);

    QLabel* passLbl = new QLabel("PASSWORD", card);
    passLbl->setObjectName("fieldLabel");

    passwordInput = new QLineEdit(card);
    passwordInput->setObjectName("inputField");
    passwordInput->setPlaceholderText("Enter your password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(42);

    errorLabel = new QLabel("", card);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->hide();

    loginButton = new QPushButton("Log In", card);
    loginButton->setObjectName("loginButton");
    loginButton->setMinimumHeight(44);
    loginButton->setCursor(Qt::PointingHandCursor);

    registerLink = new QPushButton(
        "Don't have an account? Create one", card);
    registerLink->setObjectName("registerLink");
    registerLink->setFlat(true);
    registerLink->setCursor(Qt::PointingHandCursor);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(48, 48, 48, 48); 
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(16);
    cardLayout->addWidget(userLbl);
    cardLayout->addWidget(usernameInput);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(passLbl);
    cardLayout->addWidget(passwordInput);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(errorLabel);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(loginButton);
    cardLayout->addWidget(registerLink);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->addStretch();

    QHBoxLayout* row = new QHBoxLayout();
    row->addStretch(5); 
    row->addWidget(card);
    row->addStretch(1); 

    outer->addLayout(row);
    outer->addStretch();

    connect(loginButton, &QPushButton::clicked, [this]() {
        QString u = usernameInput->text().trimmed();
        QString p = passwordInput->text();
        if (u.isEmpty() || p.isEmpty()) {
            showError("Please fill in all fields.");
            return;
        }
        clearError();
        setLoading(true);
        emit loginRequested(u, p);
    });

    connect(passwordInput, &QLineEdit::returnPressed,
            loginButton, &QPushButton::click);

    connect(registerLink, &QPushButton::clicked,
            [this](){ emit goToRegisterRequested(); });
}

void LoginWidget::applyStyles()
{
    this->setStyleSheet(
        "QWidget#card {"
        "  background-color: rgba(0,0,0,0.82);"
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 18px;"
        "}"
        "QLabel#titleLabel {"
        "  color: #ffffff;"
        "  font-size: 26px;"
        "  font-weight: bold;"
        "}"
        "QLabel#subtitleLabel {"
        "  color: rgba(255,255,255,0.45);"
        "  font-size: 13px;"
        "}"
        "QLabel#fieldLabel {"
        "  color: rgba(255,255,255,0.40);"
        "  font-size: 11px;"
        "}"
        "QLineEdit#inputField {"
        "  background-color: rgba(255,255,255,0.07);"
        "  border: 1px solid rgba(255,255,255,0.12);"
        "  border-radius: 8px;"
        "  color: #ffffff;"
        "  font-size: 14px;"
        "  padding: 0 12px;"
        "}"
        "QLineEdit#inputField:focus {"
        "  border: 1px solid rgba(127,119,221,0.80);"
        "}"
        "QPushButton#loginButton {"
        "  background-color: #7F77DD;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#loginButton:hover {"
        "  background-color: #9189E8;"
        "}"
        "QPushButton#loginButton:pressed {"
        "  background-color: #6B63C4;"
        "}"
        "QPushButton#loginButton:disabled {"
        "  background-color: rgba(127,119,221,0.35);"
        "}"
        "QPushButton#registerLink {"
        "  color: #AFA9EC;"
        "  font-size: 13px;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton#registerLink:hover {"
        "  color: #ffffff;"
        "}"
        "QLabel#errorLabel {"
        "  color: #F09595;"
        "  font-size: 13px;"
        "}"
    );
}

void LoginWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QPixmap bg(":/images/background.png.png");
    painter.drawPixmap(
        this->rect(),
        bg.scaled(this->size(),
                  Qt::KeepAspectRatioByExpanding,
                  Qt::SmoothTransformation));
    QWidget::paintEvent(event);
}

void LoginWidget::showError(const QString& message)
{
    errorLabel->setText(message);
    errorLabel->show();
}

void LoginWidget::clearError()
{
    errorLabel->clear();
    errorLabel->hide();
}

void LoginWidget::setLoading(bool loading)
{
    loginButton->setEnabled(!loading);
    loginButton->setText(loading ? "Logging in..." : "Log In");
    usernameInput->setEnabled(!loading);
    passwordInput->setEnabled(!loading);
}
