#include "NetworkClient.h"

#include <QJsonObject>
#include <QMetaObject>
#include <iostream>
#include <memory>

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent), socket_(io_) {}

NetworkClient::~NetworkClient() { disconnect(); }

void NetworkClient::connect(const std::string& host, int port) {
    tcp::resolver resolver(io_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket_, endpoints);
    read_next();
    ioThread_ = std::thread([this]() { io_.run(); });
}

void NetworkClient::disconnect() {
    io_.stop();
    if (socket_.is_open()) socket_.close();
    if (ioThread_.joinable()) ioThread_.join();
}

void NetworkClient::sendMessage(const nlohmann::json& msg) {
    std::string line = msg.dump() + "\n";
    auto buf = std::make_shared<std::string>(line);
    boost::asio::async_write(socket_, boost::asio::buffer(*buf),
        [buf](boost::system::error_code, std::size_t) {});
}

void NetworkClient::dispatch(const nlohmann::json& j) {
    std::string type = j.value("type", "");

    if (type == "login_success") {
        QString u = QString::fromStdString(j.value("username", ""));
        QMetaObject::invokeMethod(this, [this, u]() { emit loginSuccess(u); },
                                  Qt::QueuedConnection);

    } else if (type == "login_failed") {
        QString r = QString::fromStdString(j.value("reason", ""));
        QMetaObject::invokeMethod(this, [this, r]() { emit loginFailed(r); },
                                  Qt::QueuedConnection);

    } else if (type == "register_success") {
        QMetaObject::invokeMethod(this, [this]() { emit registerSuccess(); },
                                  Qt::QueuedConnection);

    } else if (type == "register_failed") {
        QString r = QString::fromStdString(j.value("reason", ""));
        QMetaObject::invokeMethod(this, [this, r]() { emit registerFailed(r); },
                                  Qt::QueuedConnection);

    } else if (type == "room_list") {
        QJsonArray arr;
        for (int i = 0; i < (int)j["rooms"].size(); i++) {
            auto item = j["rooms"][i];
            QJsonObject o;
            o["id"]      = QString::fromStdString(item.value("id",   ""));
            o["name"]    = QString::fromStdString(item.value("name", ""));
            o["members"] = item.value("members", 0);
            arr.append(o);
        }
        QMetaObject::invokeMethod(this, [this, arr]() { emit roomListReceived(arr); },
                                  Qt::QueuedConnection);

    } else if (type == "room_created") {
        QString id   = QString::fromStdString(j.value("room_id", ""));
        QString name = QString::fromStdString(j.value("name",    ""));
        QMetaObject::invokeMethod(this, [this, id, name]() { emit roomCreated(id, name); },
                                  Qt::QueuedConnection);

    } else if (type == "room_joined") {
        QString id   = QString::fromStdString(j.value("room_id", ""));
        QString name = QString::fromStdString(j.value("name",    ""));
        QString code = QString::fromStdString(j.value("code",    ""));
        QMetaObject::invokeMethod(this, [this, id, name, code]() {
            emit roomJoined(id, name, code);
        }, Qt::QueuedConnection);

    } else if (type == "join_failed") {
        QString r = QString::fromStdString(j.value("reason", ""));
        QMetaObject::invokeMethod(this, [this, r]() { emit joinFailed(r); },
                                  Qt::QueuedConnection);

    } else if (type == "member_list") {
        QStringList members;
        for (auto& m : j["members"])
            members << QString::fromStdString(m.get<std::string>());
        QMetaObject::invokeMethod(this, [this, members]() {
            emit memberListReceived(members);
        }, Qt::QueuedConnection);

    } else if (type == "code_update") {
        QString code = QString::fromStdString(j.value("code", ""));
        QMetaObject::invokeMethod(this, [this, code]() { emit codeUpdated(code); },
                                  Qt::QueuedConnection);

    } else if (type == "chat_message") {
        QString sender = QString::fromStdString(j.value("sender", ""));
        QString text   = QString::fromStdString(j.value("text",   ""));
        QMetaObject::invokeMethod(this, [this, sender, text]() {
            emit chatReceived(sender, text);
        }, Qt::QueuedConnection);

    } else if (type == "user_joined") {
        QString u = QString::fromStdString(j.value("username", ""));
        QMetaObject::invokeMethod(this, [this, u]() { emit userJoined(u); },
                                  Qt::QueuedConnection);

    } else if (type == "user_left") {
        QString u = QString::fromStdString(j.value("username", ""));
        QMetaObject::invokeMethod(this, [this, u]() { emit userLeft(u); },
                                  Qt::QueuedConnection);
    }
}

void NetworkClient::read_next() {
    boost::asio::async_read_until(socket_, buf_, '\n',
        [this](boost::system::error_code ec, std::size_t) {
            if (ec) {
                std::cout << "disconnected from server\n";
                return;
            }
            std::istream stream(&buf_);
            std::string  line;
            std::getline(stream, line);
            try {
                dispatch(nlohmann::json::parse(line));
            } catch (...) {
                std::cout << "bad json: " << line << "\n";
            }
            read_next();
        });
}