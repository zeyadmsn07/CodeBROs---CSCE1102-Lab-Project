#ifndef ROOMWIDGET_H
#define ROOMWIDGET_H

#include <QJsonArray>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QWidget>

#include "NetworkClient.h"

class RoomWidget : public QWidget {
    Q_OBJECT
public:
    explicit RoomWidget(NetworkClient* net, QWidget* parent = nullptr);

    void setUsername(const QString& name);
    void setCurrentRoomId(const QString& roomId);
    void refresh();

signals:
    void roomEntered(const QString& roomId,
                     const QString& roomName,
                     const QString& initialCode);

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void onCreateClicked();
    void onJoinClicked();
    void onRoomList(QJsonArray rooms);
    void onRoomCreated(QString roomId, QString name);
    void onRoomJoined(QString roomId, QString roomName, QString code);
    void onJoinFailed(QString reason);

private:
    void applyStyles();

    NetworkClient* net;
    QString        currentRoomId_;

    QLabel*      welcomeLabel;
    QListWidget* roomList;
    QPushButton* createBtn;
    QPushButton* joinBtn;
    QPushButton* refreshBtn;
};

#endif