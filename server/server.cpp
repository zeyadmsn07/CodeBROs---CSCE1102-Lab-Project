#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

using boost::asio::ip::tcp;
using json = nlohmann::json;

class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Server& server)
        : socket_(std::move(socket)), server_(server)
    {}

    void start() { read_next(); }

    void send(const std::string& msg)
    {
        auto self = shared_from_this();
        auto data = std::make_shared<std::string>(msg + "\n");
        boost::asio::async_write(socket_, boost::asio::buffer(*data),
            [self, data](boost::system::error_code, std::size_t){});
    }

    std::string partyId;
    std::string username;

private:
    void read_next();
    void handle(const json& msg);

    tcp::socket            socket_;
    boost::asio::streambuf buf_;
    Server&                server_;
};

// ── Server ────────────────────────────────────

class Server
{
public:
    Server(boost::asio::io_context& io, unsigned short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "listening on port " << port << "\n";
        accept_next();
    }

    void addToRoom(const std::string& partyId,
                   std::shared_ptr<Session> s)
    {
        rooms_[partyId].push_back(s);
    }

    void removeFromRoom(const std::string& partyId,
                        Session* s)
    {
        auto& vec = rooms_[partyId];
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [s](auto& wp){
                auto sp = wp.lock();
                return !sp || sp.get() == s;
            }), vec.end());
    }

    void broadcast(const std::string& partyId,
                   const json& msg,
                   Session* exclude = nullptr)
    {
        if (rooms_.find(partyId) == rooms_.end()) return;

        std::string line = msg.dump();
        auto& vec = rooms_[partyId];

        for (auto it = vec.begin(); it != vec.end(); ) {
            auto sp = it->lock();
            if (!sp) { it = vec.erase(it); continue; }
            if (sp.get() != exclude)
                sp->send(line);
            ++it;
        }
    }

    std::vector<std::string> getMemberNames(const std::string& partyId)
    {
        std::vector<std::string> names;
        if (rooms_.find(partyId) == rooms_.end()) return names;

        for (auto& wp : rooms_[partyId]) {
            auto sp = wp.lock();
            if (sp && !sp->username.empty())
                names.push_back(sp->username);
        }
        return names;
    }

private:
    void accept_next()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                    std::make_shared<Session>(
                        std::move(socket), *this)->start();
                accept_next();
            });
    }

    tcp::acceptor acceptor_;
    std::map<std::string,
             std::vector<std::weak_ptr<Session>>> rooms_;
};

// ── Session implementation ────────────────────

void Session::read_next()
{
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, buf_, '\n',
        [self](boost::system::error_code ec, std::size_t)
        {
            if (ec) {
                std::cout << "client disconnected: "
                          << self->username << "\n";
                if (!self->partyId.empty()) {
                    self->server_.removeFromRoom(
                        self->partyId, self.get());

                    json memberList;
                    memberList["type"]    = "MEMBER_LIST";
                    memberList["partyId"] = self->partyId;
                    memberList["payload"] = json(
                        self->server_.getMemberNames(
                            self->partyId)).dump();
                    self->server_.broadcast(
                        self->partyId, memberList);
                }
                return;
            }

            std::istream stream(&self->buf_);
            std::string  line;
            std::getline(stream, line);

            try {
                self->handle(json::parse(line));
            }
            catch (...) {
                std::cout << "bad json\n";
            }

            self->read_next();
        });
}

void Session::handle(const json& msg)
{
    std::string type = msg.value("type", "");

    if (type == "JOIN") {
        username = msg.value("sender",  "");
        partyId  = msg.value("partyId", "");

        server_.addToRoom(partyId, shared_from_this());
        std::cout << username << " joined " << partyId << "\n";

        json memberList;
        memberList["type"]    = "MEMBER_LIST";
        memberList["partyId"] = partyId;
        memberList["payload"] = json(
            server_.getMemberNames(partyId)).dump();
        server_.broadcast(partyId, memberList);
        return;
    }

    if (type == "LEAVE") {
        server_.removeFromRoom(partyId, this);

        json memberList;
        memberList["type"]    = "MEMBER_LIST";
        memberList["partyId"] = partyId;
        memberList["payload"] = json(
            server_.getMemberNames(partyId)).dump();
        server_.broadcast(partyId, memberList);

        partyId.clear();
        return;
    }

    if (type == "CHAT" || type == "TYPING") {
        server_.broadcast(partyId, msg, this);
        return;
    }

    if (type == "CODE_SYNC") {
        server_.broadcast(partyId, msg, this);
        return;
    }
}

// ── main ─────────────────────────────────────

int main()
{
    try {
        boost::asio::io_context io;
        Server server(io, 12345);
        io.run();
    }
    catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
    }
}