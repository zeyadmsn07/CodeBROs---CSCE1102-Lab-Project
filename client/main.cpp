#include <QApplication>
#include <QFile>
#include <QString>
#include "MainWindow.h"
#include <QDebug>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFile file(":/style.qss");
    
    qDebug() << "Exists?" << file.exists();

    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Failed to open QSS!";
    }
    int fontId = QFontDatabase::addApplicationFont(":/PressStart2P-Regular.ttf");
    QString fontFamily;
    if (fontId == -1) {
        fprintf(stderr, "Font failed to load\n");
    } else {
        fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont appFont(fontFamily);
        app.setFont(appFont);
    }
    QFont appFont(fontFamily);
    QApplication::setFont(appFont);
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);
    MainWindow w;
    w.show();
    return app.exec();
}
