#ifndef ROOMWIDGET_H
#define ROOMWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>
#include "NetworkClient.h"

class RoomWidget : public QWidget {
    Q_OBJECT
public:
    explicit RoomWidget(NetworkClient* net, QWidget* parent = nullptr);
    void setUsername(const QString& name);
    void refresh();

signals:
    void roomEntered(const QString& roomId, const QString& initialCode);

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void onCreateClicked();
    void onJoinClicked();
    void onRoomList(QJsonArray rooms);
    void onRoomCreated(QString roomId, QString name);
    void onRoomJoined(QString roomId, QString code);
    void onJoinFailed(QString reason);

private:
    void applyStyles();

    NetworkClient* net;
    QLabel*        welcomeLabel;
    QListWidget*   roomList;
    QPushButton*   createBtn;
    QPushButton*   joinBtn;
    QPushButton*   refreshBtn;
};

#endif