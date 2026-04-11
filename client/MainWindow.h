#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QString>
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "RoomWidget.h"
#include "DashboardWidget.h"
#include "NetworkClient.h"
#include "SessionStore.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginAttempt(const QString& username, const QString& password);
    void onRegisterAttempt(const QString& username, const QString& password);

private:
    QStackedWidget*  stack;
    LoginWidget*     loginPage;     // index 0
    RegisterWidget*  registerPage;  // index 1
    RoomWidget*      roomPage;      // index 2
    DashboardWidget* dashboardPage; // index 3

    NetworkClient* network;
    SessionStore*  sessions;
    QString        currentUser;
    QString        currentRoomId;

    void logout();
    void setupDebugToolbar();
};

#endif