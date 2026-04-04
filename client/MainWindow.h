#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "LoginWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget* stack;
    LoginWidget*    loginPage;
    QWidget*        registerPage;
    QWidget*        dashboardPage;
    QWidget*        taskPage;

    void setupDebugToolbar();
};

#endif
