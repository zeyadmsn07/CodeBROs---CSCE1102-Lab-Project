#include "RoomWidget.h"
#include "MessageFactory.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>

RoomWidget::RoomWidget(NetworkClient* net, QWidget* parent)
    : QWidget(parent), net(net)
{
    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(32, 32, 32, 32);
    vbox->setSpacing(16);

    welcomeLabel = new QLabel("Welcome!", this);
    welcomeLabel->setObjectName("welcomeLabel");
    QFont f = welcomeLabel->font();
    f.setPointSize(18);
    f.setBold(true);
    welcomeLabel->setFont(f);
    vbox->addWidget(welcomeLabel);

    QLabel* subtitle = new QLabel("Select a room to join, or create a new one.", this);
    subtitle->setObjectName("subtitleLabel");
    vbox->addWidget(subtitle);

    roomList = new QListWidget(this);
    roomList->setObjectName("roomList");
    vbox->addWidget(roomList);

    auto* hbox = new QHBoxLayout;
    hbox->setSpacing(10);
    createBtn  = new QPushButton("＋  Create Room", this);
    joinBtn    = new QPushButton("→  Join Room",   this);
    refreshBtn = new QPushButton("↻  Refresh",     this);
    createBtn->setObjectName("primaryBtn");
    joinBtn->setObjectName("primaryBtn");
    refreshBtn->setObjectName("secondaryBtn");
    createBtn->setMinimumHeight(40);
    joinBtn->setMinimumHeight(40);
    refreshBtn->setMinimumHeight(40);
    hbox->addWidget(createBtn);
    hbox->addWidget(joinBtn);
    hbox->addWidget(refreshBtn);
    vbox->addLayout(hbox);

    applyStyles();

    connect(createBtn,  &QPushButton::clicked, this, &RoomWidget::onCreateClicked);
    connect(joinBtn,    &QPushButton::clicked, this, &RoomWidget::onJoinClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &RoomWidget::refresh);

    connect(net, &NetworkClient::roomListReceived, this, &RoomWidget::onRoomList);
    connect(net, &NetworkClient::roomCreated,      this, &RoomWidget::onRoomCreated);
    connect(net, &NetworkClient::roomJoined,       this, &RoomWidget::onRoomJoined);
    connect(net, &NetworkClient::joinFailed,       this, &RoomWidget::onJoinFailed);
}

// draws the same background image used on the login/register pages
void RoomWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    QPixmap bg(":/images/background.jpg");
    painter.drawPixmap(
        rect(),
        bg.scaled(size(),
                  Qt::KeepAspectRatioByExpanding,
                  Qt::SmoothTransformation));
    QWidget::paintEvent(e);
}

void RoomWidget::applyStyles()
{
    setStyleSheet(
        // labels — white so they show over the image
        "QLabel#welcomeLabel {"
        "  color: #ffffff;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
        "QLabel#subtitleLabel {"
        "  color: rgba(255,255,255,0.55);"
        "  font-size: 13px;"
        "}"

        // semi-transparent dark card so the list is readable over the image
        "QListWidget#roomList {"
        "  background-color: rgba(0,0,0,0.65);"
        "  color: #ffffff;"
        "  border: 1px solid rgba(255,255,255,0.12);"
        "  border-radius: 10px;"
        "  font-size: 14px;"
        "  padding: 4px;"
        "}"
        "QListWidget#roomList::item {"
        "  padding: 8px 12px;"
        "  border-radius: 6px;"
        "}"
        "QListWidget#roomList::item:selected {"
        "  background-color: rgba(127,119,221,0.55);"
        "  color: #ffffff;"
        "}"
        "QListWidget#roomList::item:hover {"
        "  background-color: rgba(255,255,255,0.08);"
        "}"

        // primary buttons — same purple as login button
        "QPushButton#primaryBtn {"
        "  background-color: #7F77DD;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 0 16px;"
        "}"
        "QPushButton#primaryBtn:hover   { background-color: #9189E8; }"
        "QPushButton#primaryBtn:pressed { background-color: #6B63C4; }"

        // secondary button — dark transparent
        "QPushButton#secondaryBtn {"
        "  background-color: rgba(0,0,0,0.50);"
        "  color: #cccccc;"
        "  border: 1px solid rgba(255,255,255,0.15);"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  padding: 0 16px;"
        "}"
        "QPushButton#secondaryBtn:hover { background-color: rgba(0,0,0,0.70); }"
    );
}

void RoomWidget::setUsername(const QString& name) {
    welcomeLabel->setText("Welcome, " + name + "!");
}

void RoomWidget::refresh() {
    net->sendMessage(MessageFactory::makeGetRooms());
}

void RoomWidget::onCreateClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Room",
                       "Room name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    net->sendMessage(MessageFactory::makeCreateRoom(name.toStdString()));
}

void RoomWidget::onJoinClicked() {
    QListWidgetItem* item = roomList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No selection", "Pick a room first.");
        return;
    }
    QString roomId = item->data(Qt::UserRole).toString();
    net->sendMessage(MessageFactory::makeJoinRoom(roomId.toStdString()));
}

void RoomWidget::onRoomList(QJsonArray rooms) {
    roomList->clear();
    for (auto val : rooms) {
        QJsonObject r = val.toObject();
        QString label = r["name"].toString()
                      + "  [" + QString::number(r["members"].toInt()) + " online]";
        auto* item = new QListWidgetItem(label, roomList);
        item->setData(Qt::UserRole, r["id"].toString());
    }
}

void RoomWidget::onRoomCreated(QString roomId, QString name) {
    emit roomEntered(roomId, "");
}

void RoomWidget::onRoomJoined(QString roomId, QString code) {
    emit roomEntered(roomId, code);
}

void RoomWidget::onJoinFailed(QString reason) {
    QMessageBox::warning(this, "Join failed", reason);
}