#include "DashboardWidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTextCursor>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

#include "CodeRunner.h"
#include "Highlighter.h"

DashboardWidget::DashboardWidget(QWidget* parent) : QWidget(parent) {
    buildUi();
    applyStyles();
}

void DashboardWidget::buildUi() {
    // setup code editor
    codeEditor = new QPlainTextEdit();
    codeEditor->setObjectName("codeEditor");

    QFont editorFont("Courier New", 11);
    codeEditor->setFont(editorFont);
    codeEditor->setTabStopDistance(40);
    codeEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    codeEditor->setPlaceholderText("// Start coding here...");

    QPalette p = codeEditor->palette();
    p.setColor(QPalette::Text, QColor("#d4d4d4"));
    p.setColor(QPalette::Base, QColor("#1e1e1e"));
    p.setColor(QPalette::Window, QColor("#1e1e1e"));
    codeEditor->setPalette(p);
    codeEditor->setStyleSheet("QPlainTextEdit { background-color: #1e1e1e; }");

    Highlighter* h = new Highlighter(codeEditor->document());

    chatList = new QListWidget();
    chatList->setObjectName("chatList");
    chatList->setWordWrap(true);
    chatList->setSpacing(2);
    chatList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    chatInput = new QLineEdit();
    chatInput->setObjectName("chatInput");
    chatInput->setPlaceholderText("Type a message...");
    chatInput->setMinimumHeight(36);

    sendButton = new QPushButton("Send");
    sendButton->setObjectName("sendButton");
    sendButton->setMinimumHeight(36);
    sendButton->setFixedWidth(70);

    QHBoxLayout* inputRow = new QHBoxLayout();
    inputRow->setSpacing(6);
    inputRow->addWidget(chatInput);
    inputRow->addWidget(sendButton);

    QWidget* chatArea = new QWidget();
    QVBoxLayout* chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(6);
    chatLayout->addWidget(chatList);
    chatLayout->addLayout(inputRow);

    QSplitter* splitter = new QSplitter(Qt::Vertical);  // split screen between editor and chat
    splitter->addWidget(codeEditor);
    splitter->addWidget(chatArea);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    QLabel* partyLabel = new QLabel("Party");
    partyLabel->setObjectName("partyLabel");

    roomCodeLabel = new QLabel("—");
    roomCodeLabel->setObjectName("roomCodeLabel");
    roomCodeLabel->setWordWrap(true);

    memberList = new QListWidget();
    memberList->setObjectName("memberList");

    typingLabel = new QLabel();
    typingLabel->setObjectName("typingLabel");
    typingLabel->hide();

    tasksButton = new QPushButton("Tasks");
    tasksButton->setObjectName("tasksButton");
    tasksButton->setMinimumHeight(36);

    aiBtn = new QPushButton("Ask AI");
    aiBtn->setObjectName("tasksButton");
    aiBtn->setMinimumHeight(36);

    runBtn = new QPushButton("▶  Run Code");
    runBtn->setObjectName("runBtn");
    runBtn->setMinimumHeight(36);

    outputPane = new QPlainTextEdit();
    outputPane->setObjectName("outputPane");
    outputPane->setReadOnly(true);
    outputPane->setPlaceholderText("Output will appear here...");
    outputPane->setMaximumHeight(120);
    outputPane->setFont(QFont("Courier New", 10));

    logoutButton = new QPushButton("Log Out");
    logoutButton->setObjectName("logoutButton");
    logoutButton->setMinimumHeight(36);

    QVBoxLayout* sidebarLayout = new QVBoxLayout();
    sidebarLayout->setSpacing(8);
    sidebarLayout->setContentsMargins(8, 8, 8, 8);
    sidebarLayout->addWidget(partyLabel);
    sidebarLayout->addWidget(roomCodeLabel);
    sidebarLayout->addWidget(memberList);
    sidebarLayout->addWidget(typingLabel);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(runBtn);       // ← NEW
    sidebarLayout->addWidget(outputPane);   // ← NEW
    sidebarLayout->addWidget(tasksButton);
    sidebarLayout->addWidget(aiBtn);
    sidebarLayout->addWidget(logoutButton);

    QWidget* sidebar = new QWidget();
    sidebar->setObjectName("sidebar");
    sidebar->setLayout(sidebarLayout);

    // root layout
    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(splitter, 3);
    root->addWidget(sidebar, 1);
    syncTimer = new QTimer(this);
    syncTimer->setSingleShot(true);
    syncTimer->setInterval(300);

    auto sendChat = [this]() {
        QString text = chatInput->text();
        text = text.trimmed();
        if (text == "") {
            return;
        }
        chatInput->clear();
        emit chatMessageEntered(text);
    };

    connect(sendButton, &QPushButton::clicked, sendChat);
    connect(chatInput, &QLineEdit::returnPressed, sendChat);

    connect(codeEditor, &QPlainTextEdit::textChanged, [this]() { syncTimer->start(); });

    connect(syncTimer, &QTimer::timeout, [this]() {
        QString currentText = codeEditor->toPlainText();
        emit codeSyncTriggered(currentText);
    });

    connect(tasksButton, &QPushButton::clicked, [this]() { emit openTasksRequested(); });

    connect(logoutButton, &QPushButton::clicked, [this]() { emit logoutRequested(); });

    connect(aiBtn, &QPushButton::clicked, this, [this]() {
        aiBtn->setEnabled(false);
        aiBtn->setText("Thinking...");

        QString code = codeEditor->toPlainText();
        if (code.trimmed() == "") {
            appendChatMessage("AI", "Please write some code first.");
            aiBtn->setEnabled(true);
            aiBtn->setText("Ask AI");
            return;
        }
        QThread* t = new QThread(this);  // Put AI in a separate thread so the GUI doesn't freeze
        AiHelper* ai = new AiHelper();
        ai->moveToThread(t);

        connect(t, &QThread::started, ai, [ai, code]() { ai->ask(code); });

        connect(ai, &AiHelper::replyReady, this, [this, t, ai](QString reply) {
            appendChatMessage("AI", reply);
            aiBtn->setEnabled(true);
            aiBtn->setText("Ask AI");
            t->quit();
            ai->deleteLater();
        });

        connect(t, &QThread::finished, t, &QThread::deleteLater);
        t->start();
    });
    connect(runBtn, &QPushButton::clicked, this, [this]() {
        QString code = codeEditor->toPlainText();
        if (code.trimmed().isEmpty()) {
            outputPane->setPlainText("// Nothing to run.");
            return;
        }
        runBtn->setEnabled(false);
        runBtn->setText("Compiling…");
        outputPane->setPlainText("");

        // Run in a detached thread; post result back to main thread via invokeMethod
        QThread* t = new QThread(this);
        connect(t, &QThread::started, this, [this, code, t]() {
            RunResult r = CodeRunner::run(code.toStdString());
            QString result;
            if (!r.success)
                result = "Compile error:\n" + QString::fromStdString(r.errorMsg);
            else if (r.output.empty())
                result = "(no output)";
            else
                result = QString::fromStdString(r.output);

            QMetaObject::invokeMethod(this, [this, result, t]() {
                outputPane->setPlainText(result);
                runBtn->setEnabled(true);
                runBtn->setText("▶  Run Code");
                t->quit();
            }, Qt::QueuedConnection);
        });
        connect(t, &QThread::finished, t, &QThread::deleteLater);
        t->start();
    });
}

void DashboardWidget::appendChatMessage(const QString& sender, const QString& text) {
    QString time = QTime::currentTime().toString("HH:mm");
    QString fullMessage = "[" + time + "] " + sender + ": " + text;

    QListWidgetItem* item = new QListWidgetItem(fullMessage, chatList);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    QFontMetrics fm(chatList->font());
    int width = chatList->viewport()->width() - 10;
    QRect rect = fm.boundingRect(0, 0, width, 10000, Qt::TextWordWrap, fullMessage);

    QSize itemSize(width, rect.height() + 8);
    item->setSizeHint(itemSize);

    chatList->scrollToBottom();
}

void DashboardWidget::updateMemberList(const QStringList& members) {
    memberList->clear();

    // basic for loop instead of range loop
    for (int i = 0; i < members.size(); i++) {
        memberList->addItem(members[i]);
    }
}

void DashboardWidget::showTypingIndicator(const QString& username) {
    QString msg = username + " is typing...";
    typingLabel->setText(msg);
    typingLabel->show();
    QTimer::singleShot(2000, typingLabel, &QLabel::hide);  // hide after 2 seconds
}

void DashboardWidget::setRoomCode(const QString& code) { roomCodeLabel->setText(code); }

void DashboardWidget::applyRemoteCode(const QString& code) {
    // save position
    int savedPos = codeEditor->textCursor().position();
    codeEditor->blockSignals(true);
    codeEditor->setPlainText(code);
    codeEditor->blockSignals(false);
    int docLen = codeEditor->document()->characterCount();
    QTextCursor cursor = codeEditor->textCursor();

    int newPos = savedPos;
    if (newPos > docLen - 1) {
        newPos = docLen - 1;
    }

    cursor.setPosition(newPos);
    codeEditor->setTextCursor(cursor);
}

void DashboardWidget::applyStyles() {
    setStyleSheet(
        "QListWidget#chatList {"
        "  background-color: #252526;"
        "  color: #e8e8e8;"
        "  border: none;"
        "  font-family: 'Segoe UI', Arial, sans-serif;"
        "  font-size: 15px;"
        "}"
        "QLineEdit#chatInput {"
        "  background-color: #3c3c3c;"
        "  color: #ffffff;"
        "  border: 1px solid #555555;"
        "  border-radius: 6px;"
        "  padding: 0 8px;"
        "  font-size: 13px;"
        "}"
        "QPushButton#sendButton {"
        "  background-color: #7F77DD;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#sendButton:hover  { background-color: #9189E8; }"
        "QPushButton#sendButton:pressed { background-color: #6B63C4; }"
        "QWidget#sidebar {"
        "  background-color: #252526;"
        "  border-left: 1px solid #3c3c3c;"
        "}"
        "QLabel#partyLabel {"
        "  color: #ffffff;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QLabel#roomCodeLabel {"
        "  color: #AFA9EC;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QListWidget#memberList {"
        "  background-color: transparent;"
        "  color: #cccccc;"
        "  border: none;"
        "  font-size: 13px;"
        "}"
        "QLabel#typingLabel {"
        "  color: #888888;"
        "  font-size: 12px;"
        "  font-style: italic;"
        "}"
        "QPushButton#tasksButton {"
        "  background-color: #3c3c3c;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "}"
        "QPushButton#tasksButton:hover { background-color: #505050; }"
        "QPushButton#logoutButton {"
        "  background-color: transparent;"
        "  color: #F09595;"
        "  border: 1px solid #F09595;"
        "  border-radius: 6px;"
        "}"
        "QPushButton#logoutButton:hover {"
        "  background-color: rgba(240,149,149,0.10);"
        "}");
        "QPushButton#runBtn {"
        "  background-color: #3d8b3d;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#runBtn:hover  { background-color: #4caa4c; }"
        "QPushButton#runBtn:pressed { background-color: #2e6b2e; }"
        "QPushButton#runBtn:disabled { background-color: #555555; color: #999999; }"
        "QPlainTextEdit#outputPane {"
        "  background-color: #1a1a1a;"
        "  color: #98e498;"
        "  border: 1px solid #3d8b3d;"
        "  border-radius: 4px;"
        "  font-family: 'Courier New';"
        "  font-size: 11px;"
        "}";
}