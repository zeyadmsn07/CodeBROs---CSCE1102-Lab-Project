#include "DashboardWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QTime>
#include <QTimer>
#include <QTextCursor>

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    applyStyles();
}

void DashboardWidget::buildUi()
{
    // ── code editor ──────────────────────────────────────────
    codeEditor = new QPlainTextEdit();
    codeEditor->setObjectName("codeEditor");
    codeEditor->setFont(QFont("Courier New", 11));
    codeEditor->setTabStopDistance(40);
    codeEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    codeEditor->setPlaceholderText("// Start coding here...");

    // ── chat list ────────────────────────────────────────────
    chatList = new QListWidget();
    chatList->setObjectName("chatList");
    chatList->setWordWrap(true);
    chatList->setSpacing(2);
    chatList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // ── chat input row ───────────────────────────────────────
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

    // ── chat area (list + input row) ─────────────────────────
    QWidget* chatArea = new QWidget();
    QVBoxLayout* chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(6);
    chatLayout->addWidget(chatList);
    chatLayout->addLayout(inputRow);

    // ── splitter (editor top, chat bottom) ───────────────────
    QSplitter* splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(codeEditor);
    splitter->addWidget(chatArea);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    // ── right sidebar ────────────────────────────────────────
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
    sidebarLayout->addWidget(tasksButton);
    sidebarLayout->addWidget(aiBtn);
    sidebarLayout->addWidget(logoutButton);

    QWidget* sidebar = new QWidget();
    sidebar->setObjectName("sidebar");
    sidebar->setLayout(sidebarLayout);

    // ── root layout ──────────────────────────────────────────
    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(splitter, 3);
    root->addWidget(sidebar,  1);

    // ── sync debounce timer ──────────────────────────────────
    syncTimer = new QTimer(this);
    syncTimer->setSingleShot(true);
    syncTimer->setInterval(300);

    // ── connections ──────────────────────────────────────────

    // Send button + Enter key → emit chatMessageEntered
    auto sendChat = [this]() {
        QString text = chatInput->text().trimmed();
        if (text.isEmpty()) return;
        chatInput->clear();
        emit chatMessageEntered(text);
    };
    connect(sendButton, &QPushButton::clicked, sendChat);
    connect(chatInput,  &QLineEdit::returnPressed, sendChat);

    // Code editor changes → debounced sync
    connect(codeEditor, &QPlainTextEdit::textChanged, [this]() {
        syncTimer->start();   // restart the 300ms timer on every keystroke
    });
    connect(syncTimer, &QTimer::timeout, [this]() {
        emit codeSyncTriggered(codeEditor->toPlainText());
    });

    // Sidebar buttons
    connect(tasksButton,  &QPushButton::clicked,
            [this]() { emit openTasksRequested(); });
    connect(logoutButton, &QPushButton::clicked,
            [this]() { emit logoutRequested(); });
            
    // ── Ask AI button ─────────────────────────────────────────
    connect(aiBtn, &QPushButton::clicked, this, [this]() {
        aiBtn->setEnabled(false);
        aiBtn->setText("Thinking...");

        QString code = codeEditor->toPlainText();
        if (code.trimmed().isEmpty()) {
            appendChatMessage("AI", "Please write some code first.");
            aiBtn->setEnabled(true);
            aiBtn->setText("Ask AI");
            return;
        }

        QThread*  t  = new QThread(this);
        AiHelper* ai = new AiHelper;
        ai->moveToThread(t);

        connect(t,  &QThread::started,  ai, [ai, code]() { ai->ask(code); });

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
}

// ── public slots ─────────────────────────────────────────────

void DashboardWidget::appendChatMessage(const QString& sender,
                                        const QString& text)
{
    QString time = QTime::currentTime().toString("HH:mm");
    QString full = QString("[%1] %2: %3").arg(time, sender, text);

    QListWidgetItem* item = new QListWidgetItem(full, chatList);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    // Calculate proper height for wrapped text
    QFontMetrics fm(chatList->font());
    int width = chatList->viewport()->width() - 10;
    QRect rect = fm.boundingRect(0, 0, width, 10000,
                                 Qt::TextWordWrap, full);
    item->setSizeHint(QSize(width, rect.height() + 8));

    chatList->scrollToBottom();
}

void DashboardWidget::updateMemberList(const QStringList& members)
{
    memberList->clear();
    for (const QString& m : members)
        memberList->addItem(m);
}

void DashboardWidget::showTypingIndicator(const QString& username)
{
    typingLabel->setText(username + " is typing...");
    typingLabel->show();
    // Auto-hide after 2 seconds
    QTimer::singleShot(2000, typingLabel, &QLabel::hide);
}

void DashboardWidget::setRoomCode(const QString& code)
{
    roomCodeLabel->setText(code);
}

// CRITICAL: blockSignals prevents the infinite update loop
// (remote change → setPlainText → textChanged → emit sync → remote change → ...)
void DashboardWidget::applyRemoteCode(const QString& code)
{
    // Save cursor position so the user's caret doesn't jump
    int savedPos = codeEditor->textCursor().position();

    codeEditor->blockSignals(true);
    codeEditor->setPlainText(code);
    codeEditor->blockSignals(false);

    // Restore cursor to saved position (clamped to document length)
    int docLen = codeEditor->document()->characterCount();
    QTextCursor cursor = codeEditor->textCursor();
    cursor.setPosition(qMin(savedPos, docLen - 1));
    codeEditor->setTextCursor(cursor);
}

// ── styles ────────────────────────────────────────────────────
void DashboardWidget::applyStyles()
{
    setStyleSheet(
        "QPlainTextEdit#codeEditor {"
        "  background-color: #1e1e1e;"
        "  color: #d4d4d4;"
        "  border: none;"
        "  font-family: 'Courier New';"
        "  font-size: 11pt;"
        "}"
        "QListWidget#chatList {"
        "  background-color: #252526;"
        "  color: #cccccc;"
        "  border: none;"
        "  font-size: 13px;"
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
        "}"
    );
}