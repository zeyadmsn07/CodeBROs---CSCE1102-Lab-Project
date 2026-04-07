#include "MainWindow.h"
#include "SessionStore.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SessionStore sessions("data/sessions.json");
    std::string savedUser = sessions.checkSession();

    MainWindow w;
    if (!savedUser.empty())
        w.autoLogin(QString::fromStdString(savedUser));

    w.show();
    return app.exec();
}
