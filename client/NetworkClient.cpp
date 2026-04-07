#include "NetworkClient.h"
#include <iostream>

NetworkClient::NetworkClient(): socket_(io_) {}

NetworkClient::~NetworkClient() {disconnect();}

void NetworkClient::connect(const std::string& host, int port)
{
    tcp::resolver resolver(io_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket_, endpoints);

    read_next();

    ioThread_ = std::thread([this]{ io_.run(); });
}

void NetworkClient::disconnect()
{
    io_.stop();
    if (socket_.is_open())
        socket_.close();
    if (ioThread_.joinable())
        ioThread_.join();
}

void NetworkClient::sendMessage(const nlohmann::json& msg)
{
    std::string line = msg.dump() + "\n";
    auto buf = std::make_shared<std::string>(line);

    boost::asio::async_write(socket_, boost::asio::buffer(*buf),
        [buf](boost::system::error_code, std::size_t){});
}

void NetworkClient::read_next()
{
    boost::asio::async_read_until(socket_, buf_, '\n',
        [this](boost::system::error_code ec, std::size_t)
        {
            if (ec) {
                std::cout << "disconnected from server\n";
                return;
            }

            std::istream stream(&buf_);
            std::string line;
            std::getline(stream, line);

            try {
                auto j = nlohmann::json::parse(line);
                if (onMessageReceived)
                    onMessageReceived(j);
            }
            catch (...) {
                std::cout << "bad json from server: " << line << "\n";
            }

            read_next();
        });
}