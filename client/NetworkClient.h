#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QJsonArray>
#include <QObject>
#include <QString>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

class NetworkClient : public QObject {
    Q_OBJECT
   public:
    explicit NetworkClient(QObject* parent = nullptr);
    ~NetworkClient();

    void connect(const std::string& host, int port);
    void disconnect();
    void sendMessage(const nlohmann::json& msg);

   signals:
    void loginSuccess(QString username);
    void loginFailed(QString reason);
    void registerSuccess();
    void registerFailed(QString reason);

    void roomListReceived(QJsonArray rooms);
    void roomCreated(QString roomId, QString roomName);
    void roomJoined(QString roomId, QString initialCode);
    void joinFailed(QString reason);
    void codeUpdated(QString code);
    void chatReceived(QString sender, QString text);
    void userJoined(QString username);
    void userLeft(QString username);

   private:
    void read_next();
    void dispatch(const nlohmann::json& j);

    boost::asio::io_context io_;
    tcp::socket socket_;
    boost::asio::streambuf buf_;
    std::thread ioThread_;
};

#endif