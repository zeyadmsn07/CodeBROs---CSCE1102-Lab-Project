#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <functional>
#include <string>

using boost::asio::ip::tcp;

class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();

    void connect(const std::string& host, int port);
    void disconnect();
    void sendMessage(const nlohmann::json& msg);

    std::function<void(nlohmann::json)> onMessageReceived;

private:
    void read_next();
    boost::asio::io_context io_;
    tcp::socket socket_;
    boost::asio::streambuf buf_;
    std::thread ioThread_;
};

#endif