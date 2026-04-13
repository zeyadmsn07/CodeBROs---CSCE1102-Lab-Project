#include "NetworkClient.h"

#include <QJsonObject>
#include <QMetaObject>
#include <QString>
#include <iostream>
#include <memory>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent), socket_(io_) {}

NetworkClient::~NetworkClient() { disconnect(); }

void NetworkClient::connect(const std::string& host, int port) {
    boost::asio::ip::tcp::resolver resolver(io_);
    std::string portStr = std::to_string(port);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, portStr);

    boost::asio::connect(socket_, endpoints);

    read_next();

    ioThread_ = std::thread([this]() { io_.run(); });
}

void NetworkClient::disconnect() {
    io_.stop();

    if (socket_.is_open()) {
        socket_.close();
    }

    if (ioThread_.joinable()) {
        ioThread_.join();
    }
}

void NetworkClient::sendMessage(const nlohmann::json& msg) {
    std::string line = msg.dump();
    line = line + "\n";

    std::shared_ptr<std::string> buf = std::make_shared<std::string>(line);

    boost::asio::async_write(socket_, boost::asio::buffer(*buf),
                             [buf](boost::system::error_code ec, std::size_t length) {
                                 // do nothing when write finishes
                             });
}

void NetworkClient::dispatch(const nlohmann::json& j) {
    std::string type = j.value("type", "");

    if (type == "login_success") {
        std::string userStr = j.value("username", "");
        QString u = QString::fromStdString(userStr);

        QMetaObject::invokeMethod(
            this, [this, u]() { emit loginSuccess(u); }, Qt::QueuedConnection);

    } else if (type == "login_failed") {
        std::string reasonStr = j.value("reason", "");
        QString r = QString::fromStdString(reasonStr);

        QMetaObject::invokeMethod(this, [this, r]() { emit loginFailed(r); }, Qt::QueuedConnection);

    } else if (type == "register_success") {
        QMetaObject::invokeMethod(this, [this]() { emit registerSuccess(); }, Qt::QueuedConnection);

    } else if (type == "register_failed") {
        std::string reasonStr = j.value("reason", "");
        QString r = QString::fromStdString(reasonStr);

        QMetaObject::invokeMethod(
            this, [this, r]() { emit registerFailed(r); }, Qt::QueuedConnection);

    } else if (type == "room_list") {
        QJsonArray arr;

        // standard loop instead of range loop
        for (int i = 0; i < j["rooms"].size(); i++) {
            nlohmann::json item = j["rooms"][i];
            QJsonObject o;

            std::string idStr = item.value("id", "");
            std::string nameStr = item.value("name", "");
            int membersCount = item.value("members", 0);

            o["id"] = QString::fromStdString(idStr);
            o["name"] = QString::fromStdString(nameStr);
            o["members"] = membersCount;

            arr.append(o);
        }

        QMetaObject::invokeMethod(
            this, [this, arr]() { emit roomListReceived(arr); }, Qt::QueuedConnection);

    } else if (type == "room_created") {
        std::string idStr = j.value("room_id", "");
        std::string nameStr = j.value("name", "");

        QString id = QString::fromStdString(idStr);
        QString name = QString::fromStdString(nameStr);

        QMetaObject::invokeMethod(
            this, [this, id, name]() { emit roomCreated(id, name); }, Qt::QueuedConnection);

    } else if (type == "room_joined") {
        std::string idStr = j.value("room_id", "");
        std::string codeStr = j.value("code", "");

        QString id = QString::fromStdString(idStr);
        QString code = QString::fromStdString(codeStr);

        QMetaObject::invokeMethod(
            this, [this, id, code]() { emit roomJoined(id, code); }, Qt::QueuedConnection);

    } else if (type == "join_failed") {
        std::string reasonStr = j.value("reason", "");
        QString r = QString::fromStdString(reasonStr);

        QMetaObject::invokeMethod(this, [this, r]() { emit joinFailed(r); }, Qt::QueuedConnection);

    } else if (type == "code_update") {
        std::string codeStr = j.value("code", "");
        QString code = QString::fromStdString(codeStr);

        QMetaObject::invokeMethod(
            this, [this, code]() { emit codeUpdated(code); }, Qt::QueuedConnection);

    } else if (type == "chat_message") {
        std::string senderStr = j.value("sender", "");
        std::string textStr = j.value("text", "");

        QString sender = QString::fromStdString(senderStr);
        QString text = QString::fromStdString(textStr);

        QMetaObject::invokeMethod(
            this, [this, sender, text]() { emit chatReceived(sender, text); },
            Qt::QueuedConnection);

    } else if (type == "user_joined") {
        std::string userStr = j.value("username", "");
        QString u = QString::fromStdString(userStr);

        QMetaObject::invokeMethod(this, [this, u]() { emit userJoined(u); }, Qt::QueuedConnection);

    } else if (type == "user_left") {
        std::string userStr = j.value("username", "");
        QString u = QString::fromStdString(userStr);

        QMetaObject::invokeMethod(this, [this, u]() { emit userLeft(u); }, Qt::QueuedConnection);
    }
}

void NetworkClient::read_next() {
    boost::asio::async_read_until(socket_, buf_, '\n',
                                  [this](boost::system::error_code ec, std::size_t length) {
                                      if (ec) {
                                          std::cout << "disconnected from server\n";
                                          return;
                                      }

                                      std::istream stream(&buf_);
                                      std::string line;
                                      std::getline(stream, line);

                                      try {
                                          nlohmann::json parsedJson = nlohmann::json::parse(line);
                                          dispatch(parsedJson);
                                      } catch (...) {
                                          std::cout << "bad json: " << line << "\n";
                                      }

                                      read_next();
                                  });
}