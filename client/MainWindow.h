#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QString>
#include "DashboardWidget.h"
#include "LoginWidget.h"
#include "NetworkClient.h"
#include "RegisterWidget.h"
#include "RoomWidget.h"
#include "SessionStore.h"
#include "TaskWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginAttempt(const QString& username,
                        const QString& password);
    void onRegisterAttempt(const QString& username,
                           const QString& password);

private:
    QStackedWidget*  stack;
    LoginWidget*     loginPage;      // 0
    RegisterWidget*  registerPage;   // 1
    RoomWidget*      roomPage;       // 2
    DashboardWidget* dashboardPage;  // 3
    TaskWidget*      taskPage;       // 4

    NetworkClient* network;
    SessionStore*  sessions;
    QString        currentUser;
    QString        currentRoomId;

    void logout();
    void setupDebugToolbar();
};

#endif