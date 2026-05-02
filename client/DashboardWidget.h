#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QThread>
#include <QTimer>
#include <QWidget>

#include "AiHelper.h"

class DashboardWidget : public QWidget {
    Q_OBJECT

   public:
    explicit DashboardWidget(QWidget* parent = nullptr);

   public slots:
    void appendChatMessage(const QString& sender, const QString& text);
    void updateMemberList(const QStringList& members);
    void showTypingIndicator(const QString& username);
    void setRoomCode(const QString& code);   // kept — now sets room name display
    void setRoomName(const QString& name);   // FIX: explicit setter for room name
    void applyRemoteCode(const QString& code);

   signals:
    void chatMessageEntered(const QString& text);
    void codeSyncTriggered(const QString& code);
    void openTasksRequested();
    void logoutRequested();

   private:
    QPlainTextEdit* codeEditor;
    QListWidget*    chatList;
    QLineEdit*      chatInput;
    QPushButton*    sendButton;
    QPushButton*    aiBtn;
    QPushButton*    runBtn;
    QPlainTextEdit* outputPane;
    QLabel*         roomNameLabel;    // FIX: was roomCodeLabel — now shows room name
    QLabel*         userCountLabel;   // FIX: new — shows "N users online"
    QListWidget*    memberList;
    QLabel*         typingLabel;
    QPushButton*    tasksButton;
    QPushButton*    logoutButton;
    QTimer*         syncTimer;

    void buildUi();
    void applyStyles();
};

#endif