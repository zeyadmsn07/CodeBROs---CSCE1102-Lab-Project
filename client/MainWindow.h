#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QString>
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "UserStore.h"
#include "SessionStore.h"
#include "NetworkClient.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void autoLogin(const QString& username);

private slots:
    void onLoginAttempt(const QString& username, const QString& password);
    void onRegisterAttempt(const QString& username, const QString& password);

private:
    QStackedWidget*  stack;
    LoginWidget*     loginPage;
    RegisterWidget*  registerPage;
    QWidget*         dashboardPage;
    QWidget*         taskPage;

    UserStore*    userStore;
    SessionStore* sessions;
    NetworkClient* network;
    QString       currentUser;

    void setupDebugToolbar();
    void goToDashboard(const QString& username);
    void logout();
};

#endif
