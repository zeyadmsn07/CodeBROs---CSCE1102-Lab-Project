#include "MainWindow.h"
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("CodeBROs");
    setMinimumSize(900, 600);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    loginPage = new LoginWidget();

    registerPage   = new QWidget();
    dashboardPage  = new QWidget();
    taskPage       = new QWidget();

    QLabel* l2 = new QLabel("Register Page", registerPage);
    QLabel* l3 = new QLabel("Dashboard Page", dashboardPage);
    QLabel* l4 = new QLabel("Tasks Page", taskPage);
    l2->setAlignment(Qt::AlignCenter);
    l3->setAlignment(Qt::AlignCenter);
    l4->setAlignment(Qt::AlignCenter);

    QVBoxLayout* lay2 = new QVBoxLayout(registerPage);
    QVBoxLayout* lay3 = new QVBoxLayout(dashboardPage);
    QVBoxLayout* lay4 = new QVBoxLayout(taskPage);
    lay2->addWidget(l2);
    lay3->addWidget(l3);
    lay4->addWidget(l4);

    stack->addWidget(loginPage);    // index 0
    stack->addWidget(registerPage); // index 1
    stack->addWidget(dashboardPage);// index 2
    stack->addWidget(taskPage);     // index 3

    stack->setCurrentIndex(0);

    connect(loginPage, &LoginWidget::goToRegisterRequested,
            [this](){ stack->setCurrentIndex(1); });

    connect(loginPage, &LoginWidget::loginRequested,
            [this](QString u, QString p){
                stack->setCurrentIndex(2);
            });

    setupDebugToolbar();
}

void MainWindow::setupDebugToolbar()
{
    QToolBar* bar = addToolBar("Debug");
    bar->addAction("Login",
        [this]{ stack->setCurrentIndex(0); });
    bar->addAction("Register",
        [this]{ stack->setCurrentIndex(1); });
    bar->addAction("Dashboard",
        [this]{ stack->setCurrentIndex(2); });
    bar->addAction("Tasks",
        [this]{ stack->setCurrentIndex(3); });
}

MainWindow::~MainWindow() {}
