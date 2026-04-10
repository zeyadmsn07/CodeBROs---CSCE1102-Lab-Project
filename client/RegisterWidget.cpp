#include "RegisterWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QRegularExpression>

RegisterWidget::RegisterWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    applyStyles();
}

void RegisterWidget::buildUi()
{
    QWidget* card = new QWidget(this);
    card->setObjectName("card");
    card->setFixedWidth(380);

    QLabel* title = new QLabel("Create your account", card);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);

    QLabel* subtitle = new QLabel("Join a party and start coding", card);
    subtitle->setObjectName("subtitleLabel");
    subtitle->setAlignment(Qt::AlignCenter);

    QLabel* userLbl = new QLabel("USERNAME", card);
    userLbl->setObjectName("fieldLabel");
    usernameInput = new QLineEdit(card);
    usernameInput->setObjectName("inputField");
    usernameInput->setPlaceholderText("Choose a username");
    usernameInput->setMinimumHeight(42);

    QLabel* passLbl = new QLabel("PASSWORD", card);
    passLbl->setObjectName("fieldLabel");
    passwordInput = new QLineEdit(card);
    passwordInput->setObjectName("inputField");
    passwordInput->setPlaceholderText("Choose a password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(42);

    QLabel* confirmLbl = new QLabel("CONFIRM PASSWORD", card);
    confirmLbl->setObjectName("fieldLabel");
    confirmInput = new QLineEdit(card);
    confirmInput->setObjectName("inputField");
    confirmInput->setPlaceholderText("Repeat your password");
    confirmInput->setEchoMode(QLineEdit::Password);
    confirmInput->setMinimumHeight(42);

    errorLabel = new QLabel("", card);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
    errorLabel->hide();

    registerBtn = new QPushButton("Create Account", card);
    registerBtn->setObjectName("registerBtn");
    registerBtn->setMinimumHeight(44);
    registerBtn->setCursor(Qt::PointingHandCursor);

    backLink = new QPushButton("Already have an account? Log in", card);
    backLink->setObjectName("backLink");
    backLink->setFlat(true);
    backLink->setCursor(Qt::PointingHandCursor);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(36, 36, 36, 36);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(14);
    cardLayout->addWidget(userLbl);
    cardLayout->addWidget(usernameInput);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(passLbl);
    cardLayout->addWidget(passwordInput);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(confirmLbl);
    cardLayout->addWidget(confirmInput);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(errorLabel);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(registerBtn);
    cardLayout->addWidget(backLink);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->addStretch();
    QHBoxLayout* row = new QHBoxLayout();
    row->addStretch(2);
    row->addWidget(card);
    row->addStretch(1);
    outer->addLayout(row);
    outer->addStretch();

    connect(registerBtn, &QPushButton::clicked, [this]() {
        if (!validate()) return;
        setLoading(true);
        emit registerRequested(
            usernameInput->text().trimmed(),
            passwordInput->text());
    });

    connect(confirmInput, &QLineEdit::returnPressed,
            registerBtn, &QPushButton::click);

    connect(backLink, &QPushButton::clicked,
            [this]() { emit goToLoginRequested(); });
}

bool RegisterWidget::validate()
{
    QString u = usernameInput->text().trimmed();
    QString p = passwordInput->text();
    QString c = confirmInput->text();

    if (u.isEmpty() || p.isEmpty() || c.isEmpty()) {
        showError("Please fill in all fields.");
        return false;
    }

    QRegularExpression rx("^[a-zA-Z0-9_]{3,20}$");
    if (!rx.match(u).hasMatch()) {
        showError("Username must be 3-20 characters.\nLetters, numbers and _ only.");
        return false;
    }

    if (p.length() < 6) {
        showError("Password must be at least 6 characters.");
        return false;
    }

    if (p != c) {
        showError("Passwords do not match.");
        return false;
    }

    clearError();
    return true;
}

void RegisterWidget::applyStyles()
{
    setStyleSheet(
        "QWidget#card {"
        "  background-color: rgba(0,0,0,0.82);"
        "  border: 1px solid rgba(255,255,255,0.10);"
        "  border-radius: 18px;"
        "}"
        "QLabel#titleLabel {"
        "  color: #ffffff;"
        "  font-size: 24px;"
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
        "QPushButton#registerBtn {"
        "  background-color: #7F77DD;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#registerBtn:hover {"
        "  background-color: #9189E8;"
        "}"
        "QPushButton#registerBtn:pressed {"
        "  background-color: #6B63C4;"
        "}"
        "QPushButton#registerBtn:disabled {"
        "  background-color: rgba(127,119,221,0.35);"
        "}"
        "QPushButton#backLink {"
        "  color: #AFA9EC;"
        "  font-size: 13px;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton#backLink:hover {"
        "  color: #ffffff;"
        "}"
        "QLabel#errorLabel {"
        "  color: #F09595;"
        "  font-size: 12px;"
        "}"
    );
}

void RegisterWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    QPixmap bg(":/images/background.jpg");
    painter.drawPixmap(
        rect(),
        bg.scaled(size(),
                  Qt::KeepAspectRatioByExpanding,
                  Qt::SmoothTransformation));
    QWidget::paintEvent(e);
}

void RegisterWidget::showError(const QString& msg)
{
    errorLabel->setText(msg);
    errorLabel->show();
}

void RegisterWidget::clearError()
{
    errorLabel->clear();
    errorLabel->hide();
}

void RegisterWidget::setLoading(bool on)
{
    registerBtn->setEnabled(!on);
    registerBtn->setText(on ? "Creating account..." : "Create Account");
    usernameInput->setEnabled(!on);
    passwordInput->setEnabled(!on);
    confirmInput->setEnabled(!on);
}
