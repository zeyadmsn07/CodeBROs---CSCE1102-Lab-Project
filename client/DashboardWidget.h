#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QTimer>
#include <QThread>
#include "AiHelper.h"

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);

public slots:
    void appendChatMessage(const QString& sender, const QString& text);
    void updateMemberList(const QStringList& members);
    void showTypingIndicator(const QString& username);
    void setRoomCode(const QString& code);
    void applyRemoteCode(const QString& code);

signals:
    void chatMessageEntered(const QString& text);
    void codeSyncTriggered(const QString& code);
    void openTasksRequested();
    void logoutRequested();

private:
    // Left side — editor
    QPlainTextEdit* codeEditor;

    // Left side — chat
    QListWidget*    chatList;
    QLineEdit*      chatInput;
    QPushButton*    sendButton;
    QPushButton*    aiBtn;
    // Right side — sidebar
    QLabel*         roomCodeLabel;
    QListWidget*    memberList;
    QLabel*         typingLabel;
    QPushButton*    tasksButton;
    QPushButton*    logoutButton;

    // Code sync debounce timer
    QTimer*         syncTimer;

    void buildUi();
    void applyStyles();
};

#endif