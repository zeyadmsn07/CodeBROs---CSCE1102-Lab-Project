#include "RegisterWidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QVBoxLayout>

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent) {
    buildUi();
    applyStyles();
}

void RegisterWidget::buildUi() {
    QWidget* card = new QWidget(this);
    card->setObjectName("card");
    card->setMinimumWidth(580);
    card->setMaximumWidth(740);

    // left panel — logo
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

    QLabel* formTitle = new QLabel("// register", rightPanel);
    formTitle->setObjectName("formTitle");

    QLabel* userLbl = new QLabel("USERNAME", rightPanel);
    userLbl->setObjectName("fieldLabel");
    usernameInput = new QLineEdit(rightPanel);
    usernameInput->setObjectName("inputField");
    usernameInput->setPlaceholderText("choose a username");
    usernameInput->setMinimumHeight(38);

    QLabel* passLbl = new QLabel("PASSWORD", rightPanel);
    passLbl->setObjectName("fieldLabel");
    passwordInput = new QLineEdit(rightPanel);
    passwordInput->setObjectName("inputField");
    passwordInput->setPlaceholderText("choose a password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(38);

    QLabel* confirmLbl = new QLabel("CONFIRM PASSWORD", rightPanel);
    confirmLbl->setObjectName("fieldLabel");
    confirmInput = new QLineEdit(rightPanel);
    confirmInput->setObjectName("inputField");
    confirmInput->setPlaceholderText("repeat your password");
    confirmInput->setEchoMode(QLineEdit::Password);
    confirmInput->setMinimumHeight(38);

    errorLabel = new QLabel("", rightPanel);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->hide();

    registerBtn = new QPushButton("[ CREATE ACCOUNT ]", rightPanel);
    registerBtn->setObjectName("registerBtn");
    registerBtn->setMinimumHeight(40);
    registerBtn->setCursor(Qt::PointingHandCursor);

    backLink = new QPushButton("already have one? log in", rightPanel);
    backLink->setObjectName("backLink");
    backLink->setFlat(true);
    backLink->setCursor(Qt::PointingHandCursor);

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
    rightLayout->addWidget(confirmLbl);
    rightLayout->addWidget(confirmInput);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(errorLabel);
    rightLayout->addSpacing(12);
    rightLayout->addWidget(registerBtn);
    rightLayout->addSpacing(6);
    rightLayout->addWidget(backLink);
    rightLayout->addStretch();

    // card: two panels side by side
    QHBoxLayout* cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);
    cardLayout->addWidget(leftPanel, 1);
    cardLayout->addWidget(rightPanel, 2);

    // center card in window
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(20, 20, 20, 20);
    outer->addStretch();
    QHBoxLayout* row = new QHBoxLayout();
    row->addStretch();
    row->addWidget(card);
    row->addStretch();
    outer->addLayout(row);
    outer->addStretch();

    connect(registerBtn, &QPushButton::clicked, [this]() {
        if (!validate()) return;
        setLoading(true);
        emit registerRequested(usernameInput->text().trimmed(), passwordInput->text());
    });

    connect(confirmInput, &QLineEdit::returnPressed, registerBtn, &QPushButton::click);

    connect(backLink, &QPushButton::clicked, [this]() { emit goToLoginRequested(); });
}

bool RegisterWidget::validate() {
    QString u = usernameInput->text().trimmed();
    QString p = passwordInput->text();
    QString c = confirmInput->text();

    if (u.isEmpty() || p.isEmpty() || c.isEmpty()) {
        showError("please fill in all fields.");
        return false;
    }

    QRegularExpression rx("^[a-zA-Z0-9_]{3,20}$");
    if (!rx.match(u).hasMatch()) {
        showError("username: 3-20 chars, letters/numbers/_ only.");
        return false;
    }

    if (p.length() < 6) {
        showError("password must be at least 6 characters.");
        return false;
    }

    if (p != c) {
        showError("passwords do not match.");
        return false;
    }

    clearError();
    return true;
}

void RegisterWidget::applyStyles() {
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
        "QPushButton#registerBtn {"
        "  background-color: transparent;"
        "  color: #39ff14;"
        "  border: 1px solid #39ff14;"
        "  border-radius: 3px;"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "  letter-spacing: 2px;"
        "}"
        "QPushButton#registerBtn:hover {"
        "  background-color: rgba(57,255,20,0.10);"
        "}"
        "QPushButton#registerBtn:pressed {"
        "  background-color: rgba(57,255,20,0.20);"
        "}"
        "QPushButton#registerBtn:disabled {"
        "  color: rgba(57,255,20,0.30);"
        "  border-color: rgba(57,255,20,0.20);"
        "}"
        "QPushButton#backLink {"
        "  color: rgba(57,255,20,0.50);"
        "  font-family: 'Courier New';"
        "  font-size: 11px;"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QPushButton#backLink:hover {"
        "  color: #39ff14;"
        "}"
        "QLabel#errorLabel {"
        "  color: #ff5555;"
        "  font-family: 'Courier New';"
        "  font-size: 12px;"
        "}");
}

void RegisterWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QPixmap bg(":/images/background.jpg");
    painter.drawPixmap(rect(),
                       bg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    QWidget::paintEvent(event);
}

void RegisterWidget::showError(const QString& msg) {
    errorLabel->setText(msg);
    errorLabel->show();
}

void RegisterWidget::clearError() {
    errorLabel->clear();
    errorLabel->hide();
}

void RegisterWidget::setLoading(bool on) {
    registerBtn->setEnabled(!on);
    registerBtn->setText(on ? "[ creating account... ]" : "[ CREATE ACCOUNT ]");
    usernameInput->setEnabled(!on);
    passwordInput->setEnabled(!on);
    confirmInput->setEnabled(!on);
}