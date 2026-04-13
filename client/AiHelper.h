#pragma once
#include <QObject>
#include <QString>

class AiHelper : public QObject {
    Q_OBJECT
   public:
    explicit AiHelper(QObject* parent = nullptr);
    void ask(const QString& code);

   signals:
    void replyReady(QString reply);
};