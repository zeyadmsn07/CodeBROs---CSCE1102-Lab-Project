#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QString>
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "DashboardWidget.h"
#include "UserStore.h"
#include "SessionStore.h"
#include "NetworkClient.h"
#include <nlohmann/json.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void autoLogin(const QString& username);

private slots:
    void onLoginAttempt   (const QString& username, const QString& password);
    void onRegisterAttempt(const QString& username, const QString& password);
    void onNetworkMessage (const nlohmann::json& msg);   // ← NEW

private:
    QStackedWidget*  stack;
    LoginWidget*     loginPage;
    RegisterWidget*  registerPage;
    DashboardWidget* dashboardPage;
    QWidget*         taskPage;

    UserStore*     userStore;
    SessionStore*  sessions;
    NetworkClient* network;
    QString        currentUser;
    QString        currentPartyId;   // ← NEW

    void setupDebugToolbar();
    void goToDashboard(const QString& username);
    void logout();

    static QString generatePartyCode();   // ← NEW
};

#endif